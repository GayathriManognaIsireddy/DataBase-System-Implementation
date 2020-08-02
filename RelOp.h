#ifndef REL_OP_H
#define REL_OP_H

#include "Pipe.h"
#include "DBFile.h"
#include "Record.h"
#include "Function.h"

#include <stdio.h>
#include <fstream>
#include <iostream>
#include <sstream>

class RelationalOp {
	public:
	// blocks the caller until the particular relational operator 
	// has run to completion
	virtual void WaitUntilDone () = 0;

	// tell us how much internal memory the operation can use
	virtual void Use_n_Pages (int n) = 0;

	pthread_t workerThread;
};

class SelectFile : public RelationalOp { 

	private:
	static void* runHelper (void *op);
	void selectFileFunction ();

	DBFile *inFile;
	Pipe *outPipe;
	CNF *selOp;
	Record *literal;

	public:
	void Run (DBFile &inFile, Pipe &outPipe, CNF &selOp, Record &literal);
	void WaitUntilDone ();
	void Use_n_Pages (int n);

};

class SelectPipe : public RelationalOp {

	private:
	static void* runHelper(void *op);
	void selectPipeFunction();

	Pipe *inPipe;
	Pipe *outPipe;
	CNF *selOp;
	Record *literal;

	public:
	void Run (Pipe &inPipe, Pipe &outPipe, CNF &selOp, Record &literal);
	void WaitUntilDone ();
	void Use_n_Pages (int n);
};
class Project : public RelationalOp { 

	private:
	static void* runHelper(void *op);
	void projectFunction();

	Pipe *inPipe;
	Pipe *outPipe;
	int *keepMe;
	int numAttsInput;
	int numAttsOutput;

	public:
	void Run (Pipe &inPipe, Pipe &outPipe, int *keepMe, int numAttsInput, int numAttsOutput);
	void WaitUntilDone ();
	void Use_n_Pages (int n);
};
class Join : public RelationalOp { 

	private:
	static void* runHelper(void *op);
    void joinFunction();
    void joinBySort(OrderMaker *leftOM, OrderMaker *rightOM);
	void defaultJoin();
    bool sortedRecordList(Pipe &pipe, Record *rec, vector<Record*> &vec, OrderMaker *sortOrder);
    void insertMergeRecordToPipe(Record *l, Record *r);
	void inPipeToFile(DBFile lFile, char* path, bool left);

	Pipe *inPipeL;
    Pipe *inPipeR;
    Pipe *outPipe;
    CNF * selOp;
    Record *literal;
    int numOfPages;

	public:
	void Run (Pipe &inPipeL, Pipe &inPipeR, Pipe &outPipe, CNF &selOp, Record &literal);
	void WaitUntilDone ();
	void Use_n_Pages (int n);
};
class DuplicateRemoval : public RelationalOp {

	private:
	static void* runHelper(void *op);
	void duplicateFunction();

	Pipe *inPipe;
	Pipe *outPipe;
	Schema *mySchema;
	OrderMaker *myOrder;
	int runLength;

	public:
	void Run (Pipe &inPipe, Pipe &outPipe, Schema &mySchema);
	void WaitUntilDone ();
	void Use_n_Pages (int n);
};
class Sum : public RelationalOp {

	private:
	static void* runHelper(void *op);
	void sumFunction();

	Pipe *inPipe;
    Pipe *outPipe;
    Function *computeMe;

	public:
	void Run (Pipe &inPipe, Pipe &outPipe, Function &computeMe);
	void WaitUntilDone ();
	void Use_n_Pages (int n);
};
class GroupBy : public RelationalOp {

	private:
	static void* runHelper(void *op);
	void groupByFunction();
	void insertRecordToPipe(Record * prevRecord, int numOfAttsOld, int * attListNew, int intSum, double doubleSum, Type type);

	Pipe *inPipe;
	Pipe *outPipe;
	OrderMaker *groupAtts;
	Function *computeMe;
	int numOfPages;

	public:
	void Run (Pipe &inPipe, Pipe &outPipe, OrderMaker &groupAtts, Function &computeMe);
	void WaitUntilDone ();
	void Use_n_Pages (int n);
};
class WriteOut : public RelationalOp {

	private:
	static void* runHelper(void *op);
	void writeOutFunction();

	Pipe *inPipe;
	FILE *outFile;
	Schema *mySchema;

	public:
	void Run (Pipe &inPipe, FILE *outFile, Schema &mySchema);
	void WaitUntilDone ();
	void Use_n_Pages (int n);
};
#endif
