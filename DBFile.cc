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
	else if (f_type == fType::sorted)
	{
		dbClassifier = new SortedDBFile();
		fileStream << "sortedFile\n";
		//cout<<"Creating sort file"<<endl;
		LocalStartUp *localObj;
		localObj = (LocalStartUp*)startup;
		OrderMaker *curOrderMaker = localObj->localOrder;
		fileStream << localObj->runLength << endl;
		int numAttributes = curOrderMaker->GetAttributeCount();
		fileStream << numAttributes << endl;

		int listAttributes[MAX_ANDS];
		Type typeAttributes[MAX_ANDS];
		// Fetching the attributes and respective types
		curOrderMaker->GetAttributes(listAttributes, typeAttributes);
		for (int i = 0; i < numAttributes; i++)
		{
			if (typeAttributes[i] == String)
				fileStream << listAttributes[i] << " " << "String" << endl;
			else if (typeAttributes[i] == Int)
				fileStream << listAttributes[i] << " " << "Int" << endl;
			else
				fileStream << listAttributes[i] << " " << "Double" << endl;
		}
	}
	else if(f_type == tree){
		fileStream << "treeFile" <<endl;
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
	cout << "fileName is: " <<fileName<<endl;
	//cout << "given string name: " << fileName << endl;
	fileStream.open(fileName);
    //cout << "done reading file " << fileName << endl;
	string fileMetaContent;
	if (!fileStream.is_open()) {
		return 0;
	}
	else {
		getline(fileStream, fileMetaContent);
        // fileStream.close();
	}
    //cout << "It is " << metaContent << endl;
	//Checking the meta content of the file steam
	if (fileMetaContent == "heapFile") {
		dbClassifier = new HeapDBFile();
	}
	else if (fileMetaContent == "sortedFile")
	{
		//cout << "The content should be sortedFile " << fileMetaContent << endl;
		int runLength, attributesCount;
		// Fetching the current runlength
		getline(fileStream, fileMetaContent);
		//cout << "the file meta content for the run len is " << fileMetaContent << endl;
		runLength = atoi(fileMetaContent.c_str());
		//cout << "THe run length is " << runLength << endl;
		// Fetching the total attributes count
		getline(fileStream, fileMetaContent);
		//cout << "the file meta content for the run len is " << fileMetaContent << endl;
		attributesCount = atoi(fileMetaContent.c_str());

		int listAttributes[MAX_ANDS];
		Type typeAttributes[MAX_ANDS];
		for (int i = 0; i < attributesCount; i++)
		{
			string curAttribute, curType;
			getline(fileStream, fileMetaContent);
			//cout << "the file meta content for the run len is " << fileMetaContent << endl;
			//cout << metaContent << endl;
			size_t index = fileMetaContent.find(" ");
			curAttribute = fileMetaContent.substr(0,index);
			listAttributes[i] = atoi(curAttribute.c_str());
			curType = fileMetaContent.substr(index + 1);
			if (curType == "String")
			{
				typeAttributes[i] = String;
			}
			else if (curType == "Int")
				typeAttributes[i] = Int ;
			else
				typeAttributes[i] = Double;
		}
		OrderMaker *curOrderMaker = new OrderMaker();
		LocalStartUp *localObj = new LocalStartUp();
		curOrderMaker->SetAttributes(attributesCount, listAttributes, typeAttributes);
		localObj->runLength = runLength;
		localObj->localOrder = curOrderMaker;
		dbClassifier = new SortedDBFile(localObj);
	}
	else if (fileMetaContent == "treeFile"){

	}

	fileStream.close();
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
