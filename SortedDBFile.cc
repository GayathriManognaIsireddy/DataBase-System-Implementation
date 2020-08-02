#include <iostream>
#include <stdlib.h>
#include "SortedDBFile.h"
#include "string.h"
#include "unistd.h"

// Default constructer for initializing page count and read buffer
SortedDBFile::SortedDBFile()
{
	pageNum = 0;
	readBuffer = new Page();
}


SortedDBFile::~SortedDBFile()
{
	
}

SortedDBFile::SortedDBFile(int numberOfPages)
{
	pageNum=numberOfPages;
}

// Constructer for initializing page count, read buffer and startup object
SortedDBFile::SortedDBFile(void * startup)
{
	pageNum = 0;
	readBuffer = new Page();
	startUpObj = (LocalStartUp*) startup;
}

int SortedDBFile::Create(const char *fpath, fType file_type, void *startup)
{
    readMode = true;
    filePath=(char *)fpath;
    startUpObj = (LocalStartUp*) startup;
	curSortFile = new File();
	curSortFile->Open(0, (char *)fpath);
	return 1;
}

// Loads the records from file by interting trough the given file path
void SortedDBFile::Load(Schema & myschema, char * loadpath)
{
	if (bigQ == NULL || readMode)
	{
        int pipeSize=100;
		inPipe = new Pipe(pipeSize);
		outPipe = new Pipe(pipeSize);
		//cout << "---------------------------Loading the BigQ with the run length " << startUpObj->runLength << endl;
		bigQ = new BigQ(*inPipe, *outPipe, *(startUpObj->localOrder), startUpObj->runLength);
	}

    readMode = false;
	FILE *loadFile = fopen(loadpath, "r");
	Record curRecord;
    int status=curRecord.SuckNextRecord(&myschema, loadFile);
	while (status == 1)
	{
		inPipe->Insert(&curRecord);
		status=curRecord.SuckNextRecord(&myschema, loadFile);
	}

	fclose(loadFile);
	MoveFirst();

}

int SortedDBFile::Open(const char *fpath)
{
    readMode = true;
    filePath=(char *)fpath;
	curSortFile = new File();
	curSortFile->Open(1, (char *)fpath);
	if (curSortFile->GetLength() > 0)
		curSortFile->GetPage(readBuffer, 0);
	return 1;
}

// Moving the file pointer to the first record in the file
void SortedDBFile::MoveFirst()
{
	readBuffer->EmptyItOut();
	bool flag=false;
	if (!readMode) {
		mergeFilePipe();
		flag=true;
	}
	if(flag){
		delete bigQ;
		bigQ = NULL;
		delete inPipe;
		delete outPipe;
	}
	readMode = true;
	//cout << "Reading page zero to the page buffer " << endl;
	pageNum = 0;
	//cout << "The length of the main file is " << curSortFile->GetLength() << endl;
	curSortFile->GetPage(readBuffer, (off_t)pageNum);
	//cout << "Done reading the page into the buffer" << endl;
}

// Closing and deleting the pipe and local buffer objects
int SortedDBFile::Close()
{
	bool flag=false;
	if (!readMode) {
		mergeFilePipe();
		flag=true;

		delete bigQ;
		bigQ = NULL;
		delete inPipe;
		delete outPipe;
	}

	readMode = true;
	curSortFile->Close();
	delete readBuffer;
	delete curSortFile;
	return 0;
}

void SortedDBFile::Add(Record & addme)
{
	if (bigQ == NULL || readMode)
	{
        int pipeSize=100;
		inPipe = new Pipe(pipeSize);
		outPipe = new Pipe(pipeSize);
		//cout << "-------------The runlen is "<< startUpObj->runLength << endl;
		bigQ = new BigQ(*inPipe, *outPipe, *(startUpObj->localOrder), startUpObj->runLength);
	}

    readMode = false;

	inPipe->Insert(&addme);
}

// Fetching the next record from the buffer
int SortedDBFile::GetNext(Record & fetchme)
{
	// Mering the records form file and pipe when the function is not with read mode and deleting the remaining pipe objects
	if (!readMode) {
		mergeFilePipe();

		delete bigQ;
		bigQ = NULL;
		delete inPipe;
		delete outPipe;
	}

	readMode = true;
    int pageCount = curSortFile->GetLength()-1;
	pageCount=pageCount!=-1?pageCount:0;

	if (readBuffer->GetFirst(&fetchme) == 0 && ++pageNum < pageCount)
	{
		curSortFile->GetPage(readBuffer, pageNum);
		return readBuffer->GetFirst(&fetchme);
	}

	return pageNum < pageCount;
}

int SortedDBFile::GetNext(Record &fetchme, CNF &cnf, Record &literal)
{
	ComparisonEngine compareObj;
	OrderMaker queryOrder, cnfOrder;
	// Preparing the result query and fetching the total attributes present in it
	int attributesCount = OrderMaker::resultQuery(cnf, *(startUpObj->localOrder), queryOrder, cnfOrder);

	// If the query attribute count is zero, chekcing for the record that matchs with the literal record.
	if(attributesCount == 0)
	{
		while (GetNext(fetchme) != 0)
		{
			if (compareObj.Compare(&fetchme, &literal, &cnf))
			{
				return 1;
			}
		}
	}

	// If there are no records left
	if (GetNext(fetchme) == 0)
		return 0;

	// Comparing the fetched record with literal basing no file sort order and cnforder
	int result = compareObj.Compare(&fetchme, startUpObj->localOrder, &literal, &cnfOrder);
	if(result > 0){
		return 0;
	}
	if(result<0){
		int flag=sortFileBinarySearch(fetchme, cnfOrder, literal, *(startUpObj->localOrder), queryOrder);
		if(flag==0) return 0;
	}

	do {
		if (compareObj.Compare(&fetchme, &queryOrder, &literal, &cnfOrder) > 0)
			return 0;
		if (compareObj.Compare(&fetchme, &literal, &cnf))
			return 1;
	} while (GetNext(fetchme));

	return 0;
}


// Performing binary search on the pages of the curent sort file
int SortedDBFile::sortFileBinarySearch(Record& fetchme, OrderMaker& cnfOrder, Record& literal, OrderMaker& sortOrder, OrderMaker& queryOrder)
{	
	int result;
	ComparisonEngine comp;

	int low = pageNum, high = curSortFile->GetLength()-1;
    high=high!=-1?high:0;
	int mid = low + (high - low)/2;
	while (low < mid)
	{
		
		mid = low + (high - low)/2;
		curSortFile->GetPage(readBuffer, mid);
		GetNext(fetchme);
		int result = comp.Compare(&fetchme, &queryOrder, &literal, &cnfOrder);
		if (result < 0)
			low = mid + 1;
		else
			high = mid - result;

	}

	pageNum = low;
	curSortFile->GetPage(readBuffer, low);

	result = comp.Compare(&fetchme, &queryOrder, &literal, &cnfOrder);
	while(result<0){
		if (!GetNext(fetchme))
			return 0;
		result = comp.Compare(&fetchme, &queryOrder, &literal, &cnfOrder);
	}

	return result == 0;
}

// Method to merge records from both file and pipe and place the resultent records back to main file as per sort order
void SortedDBFile::mergeFilePipe()
{
	inPipe->ShutDown();
	string curSortFilePath = filePath;
	Page *localBufferPage = new Page();
	ComparisonEngine comparObject;

	vector<Record*> vecOfRecords;

	int pageIndex = 0;
	int pageCount = curSortFile->GetLength()-1;
	pageCount=pageCount!=-1?pageCount:0;
	// fileRecord to fetch record from file
	Record *fileRecord = new Record();
    bool fileEnd = false;
	if (pageCount > 0)
	{
		curSortFile->GetPage(localBufferPage, pageIndex);
		localBufferPage->GetFirst(fileRecord);
	}
	else fileEnd = true;

	// pipeRecord to fetch record from pipe
	Record *pipeRecord = new Record();
	Record *TempRecordToWrite;
	bool pipeEnd = (outPipe->Remove(pipeRecord) == 0)?true:false;
    bool pipeFlag = true;
	
	while (!fileEnd && !pipeEnd)
	{
		if (comparObject.Compare(fileRecord, pipeRecord, startUpObj->localOrder) >= 0)
		{
			TempRecordToWrite = pipeRecord;
			pipeFlag = true;
		}
		else
		{
			TempRecordToWrite = fileRecord;
			pipeFlag = false;
		}

		Record *tmp = new Record();
		tmp->Copy(TempRecordToWrite);
		vecOfRecords.push_back(tmp);

		if (!pipeFlag)
		{
			if (localBufferPage->GetFirst(fileRecord) == 0)
			{
				if (++pageIndex < pageCount)
				{
					curSortFile->GetPage(localBufferPage, pageIndex);
					localBufferPage->GetFirst(fileRecord);
				}
				else fileEnd = true;
			}
		}
		else
		{
			if (outPipe->Remove(pipeRecord) == 0)
				pipeEnd = true;
		}
	}

    while((!fileEnd) && pipeEnd){
        TempRecordToWrite = fileRecord;
		Record *tmp = new Record();
		tmp->Copy(TempRecordToWrite);
		vecOfRecords.push_back(tmp);

        if (localBufferPage->GetFirst(fileRecord) == 0)
		{
			if (++pageIndex < pageCount)
			{
				curSortFile->GetPage(localBufferPage, pageIndex);
				localBufferPage->GetFirst(fileRecord);
			}
			else fileEnd = true;
		}
    }


    while((!pipeEnd) && fileEnd){
        TempRecordToWrite = pipeRecord;
		Record *tmp = new Record();
		tmp->Copy(TempRecordToWrite);
		vecOfRecords.push_back(tmp);

        if (outPipe->Remove(pipeRecord) == 0)
				pipeEnd = true;
    }
    delete fileRecord;
	delete pipeRecord;
    delete localBufferPage;

	outPipe->ShutDown();
	remove(filePath.c_str());
	curSortFile->Close();
	delete curSortFile;
	curSortFile = new File();
	curSortFile->Open(0, (char*)filePath.c_str());
	// placing the resultent vector of records into curent sort file
	recordsTocurSortFile(vecOfRecords);
}

// Method to place the sorted records present in the vector to curSortFile
void SortedDBFile::recordsTocurSortFile(vector<Record*> vecOfRecords){
	Page *TempWriteBuffer = new Page();
	//bool flag=false;
	// iterating trough the vector and placing the record into tempWriteBuffer, if the tempWriteBuffer is full place the buffer page to 
	// sort file
	//cout<< "vector size is: "<<vecOfRecords.size()<< endl;
	for(int i=0; i<(int)vecOfRecords.size(); i++){
		if (TempWriteBuffer->Append(vecOfRecords[i]) == 0)
		{
        	int pageCount = curSortFile->GetLength()-1;
	    	pageCount=pageCount!=-1?pageCount:0;
			curSortFile->AddPage(TempWriteBuffer, pageCount);
			TempWriteBuffer->EmptyItOut();
			TempWriteBuffer->Append(vecOfRecords[i]);
		}
		//else flag=true;
	}
	// if buffer page still hold additional records after the interation
	//if(flag){
		int pageCount = curSortFile->GetLength()-1;
		pageCount=pageCount!=-1?pageCount:0;
		curSortFile->AddPage(TempWriteBuffer, pageCount);
		TempWriteBuffer->EmptyItOut();
	//}

}