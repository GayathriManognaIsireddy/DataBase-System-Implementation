#ifndef BIGQ_H
#define BIGQ_H

#ifdef DEBUG
#define error(x) (std::cerr << (x) << endl)
#define debug(x) (std::cout << (x) << endl)
#else
#define error(x) 
#define debug(x)
#endif

#define GETLENGTH(totalPages) ((totalPages == 0) ? 0 : totalPages - 1)

#include <pthread.h>
#include <queue> 
#include <iostream>
#include "Pipe.h"
#include "File.h"
#include "Record.h"
#include "Comparison.h"
#include "algorithm"
#include<bits/stdc++.h> 
#include "vector"

//A structure to the know the current location of the record in a run
class RecordStore
{
public:
	Record *rec;
	int pageNum;
	int arrayIndex;
};

//Comparator to sort the vector of Records
class compare {
	OrderMaker* orderMaker;

public:
	compare(OrderMaker *order)
	{
		orderMaker = order;
	}

	bool operator()(Record *left, Record *right) {
		ComparisonEngine comp;
		if (comp.Compare(left, right, orderMaker) < 0)
			return true;
		return false;
	}

	bool operator()(RecordStore left, RecordStore right)
	{
		ComparisonEngine comp;
		if (comp.Compare(left.rec, right.rec, orderMaker) < 0)
			return false;
		return true;
	}

};



class BigQ {

public:
	//The thread worker that handles the core functionality of BigQ
	static void* thread_worker(void* arg);

	BigQ(Pipe &in, Pipe &out, OrderMaker &sortorder, int runlen);
	~BigQ();

	/*
	Write the sorted records from the vector into the file. If the records do not fit
	into the runlen pages, them add the remaining records into the bufferPage.
	*/
	void writeToFile(vector<Record*> &vecOfRecords, Page *&bufferPage);


	/*
	Reads the records from the input pipe, sorts the records into pages for runlen
	and sends each run to the writeToFile to write to File instance.
	*/
	void sortAndWriteRuns();


	/*
	Performs a K-way merge to read the sorted runs that were written by sortAndWriteRuns,
	merge them and send them to the output Pipe.
	*/
	void MergeRecords();
	
private:
	Pipe *inPipe;
	Pipe *outPipe;
	OrderMaker *sortOrder;
	int run_length;
	File* file;

	pthread_t worker;
	char filename[50];

};

#endif
