#include<iostream>
#include<gtest/gtest.h>
#include <cstring>
#include <string.h>
#include "Record.h"
#include "DBFile.h"


DBFile dbFile;
const char *testFile = "gTest_sort";
const char * tblFilePath = "../tpch-dbgen/nation.tbl";
Schema testSchema("catalog", "nation");

OrderMaker orderObj(&testSchema);
int runLength = 6;
struct { OrderMaker *testOrder; int runLength; } testStartUp = {&orderObj, runLength};

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

//Test for sortDBFile Create funtion
TEST(Create_Test, Create_Test1) {

	int result = dbFile.Create(testFile, sorted, &testStartUp);
	dbFile.Load(testSchema, tblFilePath);
	dbFile.Close();

	ASSERT_EQ(result, 1);
}

//Test for sortDBFile Open funtion
TEST(Open_Test, Open_Test1) {

	int result = dbFile.Open(testFile);
	dbFile.Close();

	ASSERT_EQ(result, 1);
}

//Test for sortDBFile MoveFirst function
TEST(MoveFirst_Test, MoveFirst_Test1) {
	Record firstRecord;
	Record recordCompare;

	ComparisonEngine comp;
	bool flag = false;

	dbFile.Open(testFile);
	dbFile.MoveFirst();	
	dbFile.GetNext(firstRecord);	
	
	dbFile.MoveFirst();
	dbFile.GetNext(recordCompare);
	if (comp.Compare(&firstRecord, &recordCompare, &orderObj)==0) {
		flag = true;
	}
	
	ASSERT_TRUE(flag);
}

//Test for sortDBFile getNext function - for record beeing in sorted order
TEST(GetNext_Test, GetNext_Test1) {
	bool flag = false;
	ComparisonEngine comparObj;
	Record first, second;

	dbFile.GetNext(first);
	dbFile.GetNext(second);

	if(comparObj.Compare(&first, &second, &orderObj) > 0) flag=true;
	
	ASSERT_FALSE(flag);
}

//Test getNext method and check if records are in sorted order - sorted file
TEST(RecordsToCurSortFile_Test, RecordsToCurSortFile_Test1) {
    vector<Record*> vecOfRecords;
    dbFile.Create(testFile, sorted, &testStartUp);
    dbFile.Close();
    Record r1;
    FILE *src=fopen(tblFilePath, "r");

    while (r1.SuckNextRecord (&testSchema, src)) {
        Record *tmp = new Record();
		tmp->Copy(&r1);
		vecOfRecords.push_back(tmp);
	}

    SortedDBFile obj=new SortedDBFile();
    obj.Open(testFile);
    obj.recordsTocurSortFile(vecOfRecords);
    obj.MoveFirst();	
    int i=0;	
	while (obj.GetNext(r1)) {
		i++;
	}

    ASSERT_EQ(i,25);
}