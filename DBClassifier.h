#ifndef DBCLASSIFIER_H
#define DBCLASSIFIER_H

#include "Record.h"
#include "Schema.h"
#include "File.h"
// #include "DBFile.h"

typedef enum { heap, sorted, tree } fType;

struct LocalStartUp { OrderMaker *localOrder; int runLength; };
class DBClassifier {
public:
	DBClassifier();

	virtual int Create( const char *fpath, fType file_type, void *startup);

	virtual void Load(Schema &myschema,  char *loadpath);

	virtual int Open( const char *fpath);

	virtual void MoveFirst();

	virtual int Close();

	virtual void Add(Record &addme);

	virtual int GetNext(Record &fetchme);
	
	virtual int GetNext(Record &fetchme, CNF &cnf, Record &literal);

};
#endif
