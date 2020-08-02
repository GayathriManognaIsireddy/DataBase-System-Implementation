#include<iostream>
#include<gtest/gtest.h>
#include <cstring>
#include "Record.h"
#include "File.h"
#include "Function.h"
#include "DBFile.h"
#include "DBClassifier.h"
#include "HeapDBFile.h"
#include "SortedDBFile.h"
#include "BigQ.h"
#include "RelOp.h"

extern "C" {
	int yyparse(void); 
	int yyfuncparse(void); 
	void init_lexical_parser(char *); 
	void init_lexical_parser_func(char *);
	void close_lexical_parser();
	void close_lexical_parser_func();
}

extern struct AndList *final;
CNF cnf;
DBFile dbFile;

//SelectFile test
TEST(Test_SelectFile, Test_SelectFile1) {
	Pipe localPipe(100);

	char *pred_ps = "(ps_supplycost < 1.03)";
	char filename[100] = "partsupp.bin";
	dbFile.Open(filename);
	init_lexical_parser(pred_ps);
    yyparse();

    Schema testSchema("catalog", "partsupp");
    Record record;

	cnf.GrowFromParseTree(final, &testSchema, record);
	close_lexical_parser();

    SelectFile SF_ps;
	SF_ps.Use_n_Pages(8);
	SF_ps.Run(dbFile, localPipe, cnf, record);
	SF_ps.WaitUntilDone();
	Record rec;
    int status = localPipe.Remove(&rec);

	dbFile.Close();
    ASSERT_NE(status, 0);
}

//Project test
TEST(Test_Project, Test_Project1) {
	Pipe projectPipe(100);

	char *pred_p = "(p_retailprice > 931.01) AND (p_retailprice < 931.3)";
	char filename[100] = "part.bin";
	Schema testSchema("catalog", "part");
	dbFile.Open(filename);
	init_lexical_parser(pred_p);
	yyparse();
    
    Record record;
	cnf.GrowFromParseTree(final, &testSchema, record);
	close_lexical_parser();

	Pipe outPipe(1000);
	int keepMe[] = { 2,4,6 };

    Project P_p;
	P_p.Use_n_Pages(8);

    SelectFile SF_p;
	SF_p.Use_n_Pages(8);
	SF_p.Run(dbFile, projectPipe, cnf, record);
	P_p.Run(projectPipe, outPipe, keepMe, 5, 3);

	SF_p.WaitUntilDone();
	P_p.WaitUntilDone();

	dbFile.Close();

    Record rec;
    int status = outPipe.Remove(&rec);

    ASSERT_NE(status, 0);
}

//Sum test
TEST(Test_SUM, Test_SUM1) {
	Pipe localPipe(100);
	extern struct FuncOperator *finalfunc;

	char *pred_s = "(s_suppkey = s_suppkey)";
	char filename[100] = "supplier.bin";
	Schema testSchema("catalog", "supplier");
	dbFile.Open(filename);
	init_lexical_parser(pred_s);
	yyparse();

    Record record;
	cnf.GrowFromParseTree(final, &testSchema, record);
	close_lexical_parser();

	Pipe outPipe(1000);
	char *str_sum = "(s_acctbal + (s_acctbal * 1.05))";
	init_lexical_parser_func(str_sum);
    yyfuncparse();

    Function functionObj;
	functionObj.GrowFromParseTree(finalfunc, testSchema);
	close_lexical_parser_func();

    SelectFile SF_p;
    SF_p.Use_n_Pages(8);
	SF_p.Run(dbFile, localPipe, cnf, record);

    Sum T;
	T.Use_n_Pages(1);
	T.Run(localPipe, outPipe, functionObj);

	SF_p.WaitUntilDone();
	T.WaitUntilDone();
	dbFile.Close();

    Record rec;
    int status = outPipe.Remove(&rec);

    ASSERT_NE(status, 0);
}

//Select Pipe test
TEST(Test_SelectPipe, Test_SelectPipe1) {
	Pipe localPipe(100), outPipe(100);

	char *pred_ps = "(ps_supplycost < 1.03)";
	char filename[100] = "partsupp.bin";
	Schema testSchema("catalog", "partsupp");
	dbFile.Open(filename);
	init_lexical_parser(pred_ps);
    yyparse();
    Record record;
	cnf.GrowFromParseTree(final, &testSchema, record);
	close_lexical_parser();

    SelectFile SF_ps;
	SF_ps.Use_n_Pages(4);
	SF_ps.Run(dbFile, localPipe, cnf, record);
	SF_ps.WaitUntilDone();
	dbFile.Close();

    SelectPipe SP;
	SP.Use_n_Pages(4);
	SP.Run(localPipe, outPipe, cnf, record);

	Record rec;
    int status = outPipe.Remove(&rec);

    ASSERT_NE(status, 0);
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}