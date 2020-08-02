#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "DBFile.h"
#include "Defs.h"
#include <string.h>
#include <fstream>

// stub file .. replace it with your own DBFile.cc

DBFile::DBFile () {

}

int DBFile::Create (const char *f_path, fType f_type, void *startup) {
    ofstream fileStream;
    string fileName=(char *) f_path;
	fileName.append(".txt");
	//cout << "given string name: " << fileName << endl;

	fileStream.open(fileName);
	if (f_type == fType::heap) {
		dbClassifier = new HeapDBFile();
		fileStream << "heapFile";
	}

	//Closing the file stream after writing the given f_tyoe in to it
	fileStream.close();

	//Calling the classifier creat funtion with given f_type
	return dbClassifier->Create(f_path, f_type, startup);
}

void DBFile::Load (Schema &f_schema, const char *loadpath) {
    dbClassifier->Load(f_schema,(char *) loadpath);
}

int DBFile::Open (const char *f_path) {
    ifstream fileStream;
    string fileName=(char *) f_path;
	fileName.append(".txt");
	//cout << "given string name: " << fileName << endl;

	fileStream.open(fileName);
    //cout << "done reading file " << fileName << endl;
	string fileMetaContent;
	if (!fileStream.is_open()) {
		return 0;
	}
	else {
		getline(fileStream, fileMetaContent);
        fileStream.close();
	}
    //cout << "It is " << metaContent << endl;
	//Checking the meta content of the file steam
	if (fileMetaContent == "heapFile") {
		dbClassifier = new HeapDBFile();
	}

	return dbClassifier->Open(f_path);
}

void DBFile::MoveFirst () {
    dbClassifier->MoveFirst();
}

int DBFile::Close () {
    return dbClassifier->Close();
}

void DBFile::Add (Record &rec) {
    dbClassifier->Add(rec);
}

int DBFile::GetNext (Record &fetchme) {
    dbClassifier->GetNext(fetchme);
}

int DBFile::GetNext (Record &fetchme, CNF &cnf, Record &literal) {
    dbClassifier->GetNext(fetchme, cnf, literal);
}
