#include <iostream>
#include <gtest/gtest.h>
#include <cstring>
#include <stdlib.h>
#include "Statistics.h"
#include "ParseTree.h"
#include "mainQueryPlan.h"


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
extern int distinctAtts; // 1 if there is a DISTINCT in a non-aggregate query 
extern int distinctFunc;

TEST(Test_JoinList, Test_JoinList1){
    char* testCNF = "SELECT SUM (ps.ps_supplycost), s.s_suppkey FROM part AS p, supplier AS s, partsupp AS ps WHERE (p.p_partkey = ps.ps_partkey) AND (s.s_suppkey = ps.ps_suppkey) AND (s.s_acctbal > 2500.0) GROUP BY s.s_suppkey";
    yy_scan_string(testCNF);
	yyparse();
    mainQueryPlan *mainQPObject= new mainQueryPlan(finalFunction, tables, boolean, groupingAtts, attsToSelect, distinctAtts, distinctFunc);
	vector<AndList> tempJoinList = mainQPObject->getJoinList();
    ASSERT_TRUE(tempJoinList.size()>0);
    delete mainQPObject;
}

TEST(Test_JoinListNegative, Test_JoinList2){
    char* testCNF = "SELECT n.n_nationkey FROM nation AS n WHERE (n.n_name = 'UNITED STATES')";
    yy_scan_string(testCNF);
	yyparse();
    mainQueryPlan *mainQPObject= new mainQueryPlan(finalFunction, tables, boolean, groupingAtts, attsToSelect, distinctAtts, distinctFunc);
	vector<AndList> tempJoinList = mainQPObject->getJoinList();
    ASSERT_FALSE(tempJoinList.size()>0);
    delete mainQPObject;
}

TEST(Test_optmisedList, Test_optmisedList1){
    char* testCNF = "SELECT SUM (ps.ps_supplycost), s.s_suppkey FROM part AS p, supplier AS s, partsupp AS ps WHERE (p.p_partkey = ps.ps_partkey) AND (s.s_suppkey = ps.ps_suppkey) AND (s.s_acctbal > 2500.0) GROUP BY s.s_suppkey";
    yy_scan_string(testCNF);
	yyparse();
    mainQueryPlan *mainQPObject= new mainQueryPlan(finalFunction, tables, boolean, groupingAtts, attsToSelect, distinctAtts, distinctFunc);
	vector<AndList> tempJoinList = mainQPObject->getJoinList();
    vector<AndList> optmisedList = mainQPObject->optmizeJoins(tempJoinList);
    ASSERT_TRUE(optmisedList.size()>0);
    delete mainQPObject;
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}