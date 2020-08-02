#ifndef BIGQ_H
#define BIGQ_H


#include <pthread.h>
#include <queue> 
#include <iostream>
#include "algorithm"
#include "vector"
#include "Pipe.h"
#include "File.h"
#include "Record.h"
#include "Comparison.h"

using namespace std;

//Class for storing the pages as sets of pages as per the given run length
class pageList{
	public:
		queue <Page*>pageQueue;
};

// Comparator class used in sorting records
class recordCompare {
	OrderMaker* orderMaker;

public:
	recordCompare(OrderMaker *order)
	{
		orderMaker = order;
	}

	bool operator()(Record *left, Record *right) {
		ComparisonEngine comObj;
		if (comObj.Compare(left, right, orderMaker) < 0)
			return true;
		return false;
	}
};

// Used for storing the record and its respective page set
class RecordClass
{
public:
	Record *rec;
	int pageSet;
};

// Comparator class used in sorting records classes
class pairCompare{
	OrderMaker* orderMaker;
	public:
		pairCompare(OrderMaker *order)
		{
			orderMaker = order;
		}
		
		bool operator()(RecordClass left, RecordClass right)
		{
			ComparisonEngine comObj;
			if (comObj.Compare(left.rec, right.rec, orderMaker) < 0)
				return false;
			return true;
		}
};

class BigQ {
	Pipe *inputPipe;
	Pipe *outputPipe;
	OrderMaker *sortOrder;
	int runLength;
	File* file;
	char filename[50];
public:

	BigQ(Pipe &inPipe, Pipe &outPipe, OrderMaker &sortorder, int runlength);
	BigQ();
	BigQ (int runLength);
	~BigQ();

	//Thread function that initiates the BigQ process once the pthread is created
	static void* start_thread(void* arg);

	// Sorts the records read form input pipe as per the given sort order and puts them into pages of given fun_length
	int sortRecords();

	//Function for writing the remaining records present in record list to page buffer
	void remainingRecords(vector<Record*> &recordList, Page *&workingPage, int counter);

	// Writing the sorted record pages in to given file
	void fileWriter(vector<Record*> &recordList, Page *&workingPage);
};

#endif
