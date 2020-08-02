#include <iostream>
#include <stdlib.h>
#include "HeapDBFile.h"
#include "string.h"

HeapDBFile::HeapDBFile() {
	//Initializing the class varibles
	pageNum = 0;
	curHeapFile = new File();
	pageBuffered = new Page();
	scanBuffer = new Page();
}


int HeapDBFile::Create(const char *f_path, fType f_type, void *startup) {
	//Creating the new file
	//pageNum = 0;
	curHeapFile->Open(0, (char *)f_path);
	return 1;
}

void HeapDBFile::Load(Schema &f_schema, char *loadpath) {
	cout << "----------------------------MAGIC" << endl;
	Record curRecord;
	// Opening the file in read mode
	FILE *loadFile = fopen(loadpath, "r");

	//Status to check for avalibulity of data or records
	int status=curRecord.SuckNextRecord(&f_schema, loadFile);
	while (status == 1) {
		Add(curRecord);
		status=curRecord.SuckNextRecord(&f_schema, loadFile);
	}

	//Loading the records from pageBuffer to heap file and emptying the pageBuffer 
	if(pageBuffered->pageLength()){
		int pageCount = curHeapFile->GetLength()-1;
		pageCount=pageCount!=-1?pageCount:0;
		curHeapFile->AddPage(pageBuffered, pageCount);
		pageBuffered->EmptyItOut();
	}

	//cout << "Final page count: " << curHeapFile->GetLength() << endl;
	cout << "Done loding to file. Total no.of Pages: " << curHeapFile->GetLength() << endl;
	fclose(loadFile);
}


int HeapDBFile::Open(const char *f_path) {
	//cout << "In open of heap" << endl;
	//Opening the file by passing the flag as 1
	curHeapFile->Open(1, (char *)f_path);
	//cout << "opened	" << endl;
	curHeapFile->GetPage(scanBuffer, pageNum);
	//cout << "got page" << endl;
	return 1;
}

void HeapDBFile::MoveFirst() {
	//Emptying the scan buffer if it contains any previous records
	if(scanBuffer->pageLength()>0){
		scanBuffer->EmptyItOut();
	}
	
	//Before initializing scanBuffer, load the records from pageBuffer to heap file and emptying the pageBuffer 
	if(pageBuffered->pageLength()>0)
	{
		off_t pageCount = curHeapFile->GetLength()-1;
		pageCount=pageCount!=-1?pageCount:0;
		curHeapFile->AddPage(pageBuffered, pageCount);
		pageBuffered->EmptyItOut();
	}
	

	//Fetching the first page of the heap file into scanBuffer
	pageNum = 0;
	curHeapFile->GetPage(scanBuffer, pageNum);
}

int HeapDBFile::Close() {
	//Close the heap file once the records in the page buffer writen into it
	if(pageBuffered->pageLength()>0){
		int pageCount = curHeapFile->GetLength()-1;
		pageCount=pageCount!=-1?pageCount:0;
		curHeapFile->AddPage(pageBuffered, pageCount);
		pageBuffered->EmptyItOut();
	}
	curHeapFile->Close();
	delete curHeapFile;
	pageNum = 0;

	delete pageBuffered;
	delete scanBuffer;

	return 1;
}

void HeapDBFile::Add(Record &addme) {
	// Status to check for successfully appending the record to the page
	int statusFlag=(pageBuffered)->Append(&addme);
	//Successfully appended
	if(statusFlag==1) return; 
	
	//Failed to append, write the records present in pageBuffer to file
	off_t pageCount = curHeapFile->GetLength()-1;
	pageCount=pageCount!=-1?pageCount:0;
	curHeapFile->AddPage(pageBuffered, pageCount);
	if (pageCount == 0)
	{
		pageNum = 0;
		curHeapFile->GetPage(scanBuffer, pageNum);
	}
	pageBuffered->EmptyItOut();

	//Append the record to pageBuffer
	pageBuffered->Append(&addme);
}

int HeapDBFile::GetNext(Record &fetchme) {
	// Status flag to check for fetching the first record from the scanBuffer
	int statusFlag = scanBuffer->GetFirst(&fetchme);
	if(statusFlag == 1){
		return 1;
	}

	//If no records present in the scan buffer page
	off_t noOfPages =curHeapFile->GetLength();
	//If there are some pages present in the file
	if(noOfPages>0){
		pageNum++;
		//Reading the last page. Before reading put all the records in pageBuffer into file
		if(noOfPages -1 == pageNum){

			if(pageBuffered->pageLength()>0){
				curHeapFile->AddPage(pageBuffered,pageNum);
				pageBuffered->EmptyItOut();
				curHeapFile->GetPage(scanBuffer, pageNum);
				statusFlag = scanBuffer->GetFirst(&fetchme);
			}
			else{
				return 0;
			}
		}
		//Reading pages before the last page
		else if(noOfPages-1 > pageNum){
			curHeapFile->GetPage(scanBuffer,pageNum);
			statusFlag = scanBuffer->GetFirst(&fetchme);
		}
	}
	//If file dosen't contain any page as of now, put the records from scanBuffer to file and read the file.
	else{
		if(pageBuffered->pageLength()>0){
			curHeapFile->AddPage(pageBuffered,0);
			pageBuffered->EmptyItOut();
			curHeapFile->GetPage(scanBuffer,0);
			pageNum = 0;

			statusFlag = scanBuffer->GetFirst(&fetchme)==1?1:0;
		}
		else{
			return 0;
		}
	}

	return statusFlag;
}


int HeapDBFile::GetNext(Record &fetchme, CNF &cnf, Record &literal)
{
	ComparisonEngine compareObj;
	// statusFlag to check for records presenting in the page
	int statusFlag = GetNext(fetchme);
	while (statusFlag == 1)
	{
		//Found a matching record
		if (compareObj.Compare(&fetchme, &literal, &cnf))
		{
			return 1;
		}
		statusFlag = GetNext(fetchme);
	}
	return 0;
}