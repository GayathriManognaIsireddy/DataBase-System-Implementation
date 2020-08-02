#include "BigQ.h"
#include <unistd.h>


BigQ :: BigQ (Pipe &in, Pipe &out, OrderMaker &sortorder, int runlen) {
	/* read data from in pipe sort them into runlen pages
      construct priority queue over sorted runs and dump sorted data 
 	  into the out pipe
      finally shut down the out pipe
	*/
	inPipe = &in;
	outPipe = &out;
	sortOrder = &sortorder;
	run_length = runlen;

	pthread_t worker;

	file = new File();
	timespec time;
  	clock_gettime(CLOCK_REALTIME, &time);
  	int timestamp = time.tv_sec*1000000000 + time.tv_nsec;
  	if (timestamp < 0) timestamp *= -1;

    // Initialize the file that will contain all run information
  	sprintf(filename, "bigq_sorted_%d.bin", timestamp);
  	file->Open(0, filename);


	//Call to worker
	debug("Debug is working");
	int threadNumber = pthread_create(&worker, NULL, &thread_worker, (void *) this);
}

void* BigQ::thread_worker(void *arg)
{

	BigQ *bigQ = (BigQ *)arg;
	bigQ->sortAndWriteRuns();
	bigQ->MergeRecords();
	bigQ->outPipe->ShutDown();

	pthread_exit(NULL);

}

void BigQ::sortAndWriteRuns()
{
	Page *currentPage = new Page;
	vector<Record*> vecOfRecords;
	Record *rec = new Record;
	int pageCount = 0;
	int recs = 0;
	while (inPipe->Remove(rec))
	{
		recs++;
		Record* temp = new Record();
		temp->Copy(rec);
		//If the record cannot be added to the page
		if (!currentPage->Append(rec)) {
			if (++pageCount < run_length)
			{
				currentPage->EmptyItOut();
				currentPage->Append(rec);
			}
			else
			{
				//A run is filled, write the run into the file and initialize the next run.
				//Sort the vector
				sort(vecOfRecords.begin(), vecOfRecords.end(), compare(sortOrder));

				//Write the run to the file.
				currentPage->EmptyItOut();
				writeToFile(vecOfRecords, currentPage);
				pageCount = 0;
				currentPage->Append(rec);
			}
		}
		vecOfRecords.push_back(temp);
	}
	//If there are some more records in the vector, add them to a page
	if (!vecOfRecords.empty()) {
		sort(vecOfRecords.begin(), vecOfRecords.end(), compare(sortOrder));
		Page *bufferpage;
		writeToFile(vecOfRecords, bufferpage);
	}

}

void BigQ::writeToFile(vector<Record*> &vecOfRecords, Page *&bufferPage)
{
	Page *page = new Page();
	int IsDirty = false;
	int pageCount = 0;
	int lastRecordIndex = INT_MAX;
	for (int i = 0; i < (int)vecOfRecords.size(); i++)
	{
		Record *rec = new Record();
		IsDirty = true;
		rec->Copy(vecOfRecords[i]);
		if (!page->Append(rec))
		{
			if (++pageCount < run_length)
			{
				file->AddPage(page, GETLENGTH(file->GetLength()));
				page->EmptyItOut();
				page->Append(rec);
				IsDirty = true;
			}
			else
			{
				lastRecordIndex = i;
				break;
			}
		}
	}
	//Add the remaining page to the file and the extra records will remain in the vector and add them to the bufferPage.
	file->AddPage(page, GETLENGTH(file->GetLength()));
	if (lastRecordIndex < vecOfRecords.size()) {
		vecOfRecords.erase(vecOfRecords.begin(), vecOfRecords.begin() + lastRecordIndex);
		for (int i = 0; i < (int)vecOfRecords.size(); i++)
		{
			Record *temp = new Record();
			temp->Copy(vecOfRecords[i]);
			bufferPage->Append(temp);
		}
	}
	else
		vecOfRecords.clear();
}

void BigQ::MergeRecords()
{
	//Create a prioriy queue with variable as RecordStore
	priority_queue<RecordStore,vector<RecordStore>, compare> RecPQ(sortOrder);
	int run = 0;
	int i = 0;
	int numOfPages = GETLENGTH(file->GetLength());
	int numOfRuns = (int) (numOfPages/run_length);
	if (numOfPages%run_length != 0)
		numOfRuns++;
	//Initialize an array of buffer pages for K way merge
	Page BufferPages[numOfRuns];
	//Initialize your priority queue with 1st page of each run
	RecordStore tempRecord;
	while (i < numOfRuns && run < numOfPages)
	{
		Record *rec = new Record();
		file->GetPage(&BufferPages[i], run);
		BufferPages[i].GetFirst(rec);
		tempRecord.arrayIndex = i;
		tempRecord.pageNum = run;
		tempRecord.rec = rec;
		RecPQ.push(tempRecord);
		run += run_length;
		i++;
	}
	//Now get a record from top of each page compare and push in out queue
	while (!RecPQ.empty())
	{
		RecordStore currStore = RecPQ.top();
		int pageNum = currStore.pageNum;
		int index = currStore.arrayIndex;
		outPipe->Insert(currStore.rec);
		//r = new Record;
		//Now pop the record
		RecPQ.pop();

		//If you have exhausted the current page in buffer go to the next page of the run
		Record *newRec = new Record();
		if (BufferPages[index].GetFirst(newRec) == 0)
		{
			pageNum = pageNum + 1;
			//Check if the page is not the last page of the run and check if it is not the last page of the file
			if (pageNum < ((index + 1)*run_length) && pageNum < numOfPages)
			{
				file->GetPage(&BufferPages[index], pageNum);
				//get the record from the newly loaded page, push in the PQ after updating PageNum
				if (BufferPages[index].GetFirst(newRec) != 0)
				{
					tempRecord.arrayIndex = index;
					tempRecord.pageNum = pageNum;
					tempRecord.rec = newRec;
					RecPQ.push(tempRecord);
				}
			}
		}
		//Push the record in the PQ
		else {
			tempRecord.rec = newRec;
			tempRecord.arrayIndex = index;
			tempRecord.pageNum = pageNum;
			RecPQ.push(tempRecord);
		}
	}
}
BigQ::~BigQ () {
	file->Close();

	outPipe->ShutDown();
}
