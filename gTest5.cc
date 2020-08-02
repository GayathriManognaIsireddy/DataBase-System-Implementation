#include <iostream>
#include <gtest/gtest.h>
#include <cstring>
#include <stdlib.h>
#include "Statistics.h"
#include "ParseTree.h"
#include "MainQueryPlaner.h"

using namespace std;

extern "C" {
	int yyparse(void);
    struct YY_BUFFER_STATE* yy_scan_string(const char*);
}

extern struct FuncOperator *finalFunction;
extern struct TableList *tables;
extern struct AndList *boolean;
extern struct NameList *groupingAtts;
extern struct NameList *attsToSelect;
extern struct NameList *sortAtts;
extern int distinctAtts;
extern int distinctFunc;

extern struct InOutPipe *io;

extern string createTableName;
extern string createTableType;
extern vector<string> atts;
extern vector<string> attTypes;
//extern char *dbfile_dir;

extern int queryType;


TEST(Test_CreatTable, Test_CreatTable1){
    char* testCNF = "CREATE TABLE testSortTable (att1 INTEGER, att2 DOUBLE, att3 STRING) AS SORTED ON att1, att2";
    yy_scan_string(testCNF);
	yyparse();
    QueryHandler *queryHandlerObj = new QueryHandler(finalFunction, tables, boolean, groupingAtts, attsToSelect, sortAtts, distinctAtts, distinctFunc, 
                                                                io, createTableName, createTableType, atts, attTypes, queryType);
    queryHandlerObj->GetLocations();
    int status = queryHandlerObj->CreateTable();
    ASSERT_EQ(status, 1);
    delete queryHandlerObj;
}

TEST(Test_InserToTable, Test_InserToTable1){
    char* testCNF = "INSERT toload.txt INTO testSortTable";
    yy_scan_string(testCNF);
	yyparse();
    QueryHandler *queryHandlerObj = new QueryHandler(finalFunction, tables, boolean, groupingAtts, attsToSelect, sortAtts, distinctAtts, distinctFunc, 
                                                                io, createTableName, createTableType, atts, attTypes, queryType);
    queryHandlerObj->GetLocations();
    int status = queryHandlerObj->InsertToTable();
    ASSERT_EQ(status, 1);
    delete queryHandlerObj;
}

// TEST(Test_TableDrop, Test_TableDrop1){
//     // char* testCNF = "DROP TABLE testSortTable";
//     // yy_scan_string(testCNF);
// 	// yyparse();
//     QueryHandler *queryHandlerObj = new QueryHandler(finalFunction, tables, boolean, groupingAtts, attsToSelect, sortAtts, distinctAtts, distinctFunc, 
//                                                                 io, createTableName, createTableType, atts, attTypes, queryType);
//     //queryHandlerObj->GetLocations();
//     int status = queryHandlerObj->DropTable("testSortTable");
//     ASSERT_EQ(status, 0);
//     delete queryHandlerObj;
// }

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}