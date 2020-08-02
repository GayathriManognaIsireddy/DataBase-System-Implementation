#include <iostream>
#include <gtest/gtest.h>
#include <cstring>

#include "Pipe.h"
//#include "test.h"
#include "BigQ.h"

const char *dbfile_dir = "../tpch-dbgen";
const char* fileToTest = "nation.bin";
const char *testTable = "../tpch-dbgen/nation.tbl";

int main(int argc, char **argv) {

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}


//Test for proper sorting of records
TEST(Create_Test, Create_Test1) {

    int buffsz = 100; // pipe cache size
	Pipe inputPipe (buffsz);
	Pipe outputPipe (buffsz);
    int runlen=buffsz;

    ComparisonEngine comp;
    FILE *opeFile = fopen("../tpch-dbgen/region.tbl", "r");
	Schema schema("catalog", "region");
	Record tempRec;
	while (tempRec.SuckNextRecord(&schema, opeFile) == 1) {
		inputPipe.Insert(&tempRec);
	}
	fclose(opeFile);
	inputPipe.ShutDown();

    OrderMaker sortCondition(&schema);

    BigQ bigQObj (inputPipe, outputPipe, sortCondition, runlen);

    bool flag = false;
	Record *curRec = NULL, *preRec = NULL;
    // outputPipe.Remove(curRec);
    // outputPipe.Remove(preRec);
    Record temp;
	while (outputPipe.Remove(&temp)) {
        Record *temp1=preRec;
		preRec = curRec;
		curRec = &temp;
		if (preRec && curRec) {
            int temp=comp.Compare(preRec, curRec, &sortCondition);
            //cout<<"test 1 "<<temp<<endl;
			if (temp ==1) {
				flag = true;
			}

		}
	}

	ASSERT_FALSE(flag);
}

//Test for chekcing proper insertion of records in to the file
TEST(Create_Test, Create_Test2) {
    vector<Record*> recordsList;
    fflush(stdout);
    FILE *dbFile = fopen("../tpch-dbgen/region.tbl", "r");
	Schema test("catalog", "region");
	Record temprec;
    temprec.SuckNextRecord(&test, dbFile);
    //temprec.Print(&test);
	fclose(dbFile);
    recordsList.push_back(&temprec);

    BigQ bq;
    Page *bufferpage;
    File* file = new File();
    fflush(stdout);
	file->Open(0, "TestBigQsorted");
    bq.fileWriter(recordsList, bufferpage, file);
    file->Close();
    int size=0;
    FILE *fp=fopen("TestBigQsorted", "r");
    if(fp!=NULL){
        fseek(fp,0,SEEK_END);
        size=ftell(fp);
    }

    ASSERT_NE(size, 0);
}

//Faile test for inserting records in to the file
TEST(Create_Test, Create_Test3) {
    vector<Record*> recordList;
    fflush(stdout);
    FILE *dbFile = fopen("../tpch-dbgen/region.tbl", "r");
	Schema test("catalog", "region");
	fclose(dbFile);

    BigQ bq;
    Page *bufferpage;
    File* file = new File();
    int size=8;
    fflush(stdout);
	file->Open(0, "Test2BigQsorted");
    bq.fileWriter(recordList, bufferpage, file);
    file->Close();
    FILE *fp=fopen("Test2BigQsorted", "r");
    if(fp!=NULL){
        fseek(fp,0,SEEK_END);
        size-=ftell(fp);
    }
    ASSERT_EQ(size, 0);
}

//Test for remainingRecords funtion exit case, emptying the recordlist
TEST(Create_Test, Create_Test4) {
    vector<Record*> recordsList;
    fflush(stdout);
    FILE *dbFile = fopen("../tpch-dbgen/region.tbl", "r");
	Schema test("catalog", "region");
	Record temprec;
    temprec.SuckNextRecord(&test, dbFile);
    //temprec.Print(&test);
	fclose(dbFile);
    recordsList.push_back(&temprec);

    BigQ bq;
    Page *bufferpage;
    File* file = new File();
    fflush(stdout);
	file->Open(0, "testfile");
    bq.remainingRecords(recordsList, bufferpage, 10);
    file->Close();
    //cout<<"size of record list: "<<recordsList.size()<<endl;
    int size=recordsList.size();
    ASSERT_EQ(size, 0);
}