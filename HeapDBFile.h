#ifndef HEAPDBFILE_H
#define HEAPDBFILE_H

#include "DBClassifier.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
// #include "DBFile.h"


class HeapDBFile : public DBClassifier {
private:
	File *curHeapFile; // Main heap file
	int pageNum; // Index used for reterving required page

	Page *pageBuffered; // Page buffered with records till moment
	Page *scanBuffer; // Buffer used for reading records in scan operation

public:
	HeapDBFile();

	int Create( const char *fpath, fType file_type, void *startup);

	void Load(Schema &myschema, char *loadpath);

	int Open( const char *fpath);

	void MoveFirst();

	int Close();

	void Add(Record &addme);

	int GetNext(Record &fetchme);

	int GetNext(Record &fetchme, CNF &cnf, Record &literal);
};
#endif
