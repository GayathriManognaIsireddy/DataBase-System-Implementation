#include "BigQ.h"
#include <unistd.h>
#include <cmath>


BigQ :: BigQ (Pipe &inPipe, Pipe &outPipe, OrderMaker &sortorder, int runlength) {
	// read data from in pipe sort them into runlen pages

    // construct priority queue over sorted runs and dump sorted data 
 	// into the out pipe

    // finally shut down the out pipe
	inputPipe = &inPipe;
	outputPipe = &outPipe;
	sortOrder = &sortorder;
	runLength = runlength;

	pthread_t worker;

	pthread_create(&worker, NULL, &start_thread, (void *) this);
}

BigQ::BigQ () {
}

BigQ::BigQ (int runLength) {
	runLength=(int)runLength;
}

BigQ::~BigQ () {
}

void* BigQ::start_thread(void *arg)
{
	BigQ *bigQ = (BigQ *)arg;
	bigQ->sortRecords();
}

int BigQ::sortRecords()
{
	vector<Record*> recordsList;
	Record *record = new Record;
	int curPageCount = 0;

	File* file = new File();
	file->Open(0, "sortedBigQ");

	Page *workingPage = new Page;
	int recCounter=0;
	while (inputPipe->Remove(record))
	{
		recCounter++;
		Record* tempRecord = new Record();
		tempRecord->Copy(record);
		//Appending the record to workingpage untile the page is full
		if (!workingPage->Append(record)) {
			workingPage->EmptyItOut();
			if (++curPageCount >= runLength)
			{
				sort(recordsList.begin(), recordsList.end(), recordCompare(sortOrder));
				fileWriter(recordsList, workingPage, file);
				curPageCount = 0;
			}
			workingPage->Append(record);
		}
		recordsList.push_back(tempRecord);
	}
	if(recCounter==0) return 0;
	Page *bufferpage;
	//Add the rest of the records present in the record list to page
	//!recordsList.empty()&&
	if (recordsList.size()>0) {
		sort(recordsList.begin(), recordsList.end(), recordCompare(sortOrder));
		fileWriter(recordsList, bufferpage, file);
	}

	priority_queue<RecordClass,vector<RecordClass>, pairCompare> pqRecords(sortOrder);
	int len=file->GetLength();
	int totalPages = len==0?0:len-1;
	int totalRuns = ceil((float)totalPages/runLength);
	//cout << "The number of pages is " << totalPages << "  and the run length is " << runLength << endl;
	//cout<< "Num of runs   "<<totalRuns<<endl;

	pageList *pageBufferSet[totalRuns];
	for(int j=0; j<totalRuns; j++){
		pageBufferSet[j] = new pageList();
	}

	//cout<< "After initializing pageBufferSet "<<totalPages<<endl;
	for(int j=0; j<totalPages; j++){
		Page *temp=new Page();
		
		file->GetPage(temp, j);
		int pos=j/runLength;
		//cout << "While pusing pages to pos index ********"<<pos<< endl;
		pageBufferSet[pos]->pageQueue.push(temp);
	}
	//cout<< "After pusing pages to pageBufferSet "<<totalRuns<<endl;
	
	RecordClass recordHold;
	int setCount = 0;
	//Schema mySchema ("catalog", "partsupp");
	for(int j=0; j<totalRuns&&setCount < totalPages; j++){
		//cout<<"1. run value is "<<run<<endl;
		Record *record = new Record();
		//(*pageBufferSet[j]).pageQueue.front().GetFirst(rec);
		pageList *temp = pageBufferSet[j];
		queue<Page *> tempPageQ = (*temp).pageQueue;
		Page *tempPage = tempPageQ.front();
		tempPage->GetFirst(record);
		//rec->Print(&mySchema);
		recordHold.pageSet = j;
		//recordHold.pageNum = run;
		recordHold.rec = record;
		pqRecords.push(recordHold);
		//cout<<"2. run value if "<<run<<endl;
		setCount += runLength;
	}

	//cout << "After pushing record to Priority queue  "<<endl;
	cout << "Priority Queue size  "<<pqRecords.size()<<endl;
	
	while (!pqRecords.empty())
	{
		//cout<<"Inside while loop "<<RecPQ.size()<<endl;
		RecordClass recClassObj = pqRecords.top();
		pqRecords.pop();
		int setIndex = recClassObj.pageSet;
		outputPipe->Insert(recClassObj.rec);
		//Now pop the record
		bool flag=false;
		//If you have exhausted the current page in buffer go to the next page of the run
		Record *curTempRecord = new Record();
		if ((*pageBufferSet[setIndex]).pageQueue.front()->GetFirst(curTempRecord) == 0)
		{
			(*pageBufferSet[setIndex]).pageQueue.pop();
			//Check if the page is not the last page of the run and check if it is not the last page of the file
			if (pageBufferSet[setIndex]->pageQueue.size()>0)
			{
				if ((*pageBufferSet[setIndex]).pageQueue.front()->GetFirst(curTempRecord) != 0)
				{
					flag=true;
					//cout<<"Page number if "<<numPage<<endl;
				}
			}
		}
		//Push the record in the PQ
		else {
			flag=true;
		}
		if(flag){
			recordHold.rec = curTempRecord;
			recordHold.pageSet = setIndex;
			pqRecords.push(recordHold);
		}
	}
	file->Close();
	outputPipe->ShutDown();
	return 1;
}

void BigQ::remainingRecords(vector<Record*> &recordsList, Page *&workingPage, int counter){
	int size=recordsList.size();
	if (counter >= size) {
		recordsList.clear();
	}
	else{
		for (int j = counter+1; j < size; j++)
		{
			Record *temp = new Record();
			temp->Copy(recordsList[j]);
			workingPage->Append(temp);
		}
		recordsList.erase(recordsList.begin(), recordsList.begin() + counter);
	}
}

void BigQ::fileWriter(vector<Record*> &recordsList, Page *&workingPage, File* file)
{
	Page *curPage = new Page();
	int curPageCount = 0;
	int counter = 0;
	while (counter < recordsList.size())
	{
		Record *curTempRecord = new Record();
		curTempRecord->Copy(recordsList[counter]);
		if (!curPage->Append(curTempRecord))
		{
			if (++curPageCount >= runLength)
			{
				break;
			}
			else{
				int len=file->GetLength();
				int temp = len==0?0:len-1;
				file->AddPage(curPage, temp);
				curPage->EmptyItOut();
				curPage->Append(curTempRecord);
			}
		}
		counter++;
	}
	//Add the remaining page to the file and the extra records will remain in the record list and add them to the bufferPage.
	remainingRecords(recordsList, workingPage, counter);
	int len=file->GetLength();
	int temp = len==0?0:len-1;
	if(curPage->pageLength()>0) 
		file->AddPage(curPage, temp);
}
