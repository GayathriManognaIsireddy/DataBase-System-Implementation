#ifndef SORTEDDBFILE_H
#define SORTEDDBFILE_H

#include "Record.h"
#include "Schema.h"
#include "BigQ.h"
#include "File.h"
#include "DBClassifier.h"

class SortedDBFile : public DBClassifier {
private:
	Pipe *inPipe;
	Pipe *outPipe;
	BigQ *bigQ;

    string filePath;
	Page *readBuffer;
	File *curSortFile;
	LocalStartUp *startUpObj;
	
	bool readMode;
	int pageNum;
public:
	SortedDBFile();

	SortedDBFile(int numberOfPages);

	SortedDBFile(void *startup);

	int Create(const char *fpath, fType file_type, void *startup);

	void Load(Schema &myschema, char *loadMe);

	int Open(const char *fpath);

	void MoveFirst();

	int Close();

	void Add(Record &addme);

	int GetNext(Record &fetchme);

	int GetNext(Record &fetchme, CNF &cnf, Record &literal);

	int sortFileBinarySearch(Record& fetchme, OrderMaker& cnfOrder, Record& literal, OrderMaker& sortOrder, OrderMaker& queryOrder);

	void mergeFilePipe();

	void recordsTocurSortFile(vector<Record*> vecOfRecords);

	~SortedDBFile();
};
#endif