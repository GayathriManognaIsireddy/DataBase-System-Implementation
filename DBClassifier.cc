#include <iostream>
#include <stdlib.h>
#include "Record.h"
#include "Schema.h"

#include "File.h"
#include "DBFile.h"

//Use for invalid fType
string errorMsg = "DBClassifier Called";

DBClassifier::DBClassifier() {

}

int DBClassifier::Create(const char *fpath, fType file_type, void *startup) {
	cerr << errorMsg;
	return 1;
}

void DBClassifier::Load(Schema &myschema, char *loadpath) {
	cerr << errorMsg;
}

int DBClassifier::Open(const char *fpath) {
	cerr << errorMsg;
	return 1;
}

void DBClassifier::MoveFirst() {
	cerr << errorMsg;
}

int DBClassifier::Close() {
	cerr << errorMsg;
	return 1;
}

void DBClassifier::Add(Record &addme) {
	cerr << errorMsg;
}

int DBClassifier::GetNext(Record &fetchme) {
	cerr << errorMsg;
	return 1;
}

int DBClassifier::GetNext(Record &fetchme, CNF &cnf, Record &literal) {
	cerr << errorMsg;
	return 1;
}