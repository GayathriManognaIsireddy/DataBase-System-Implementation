#include <iostream>
#include<gtest/gtest.h>
#include <cstring>
#include <stdlib.h>
#include "Statistics.h"
#include "ParseTree.h"
#include <math.h>
extern "C" struct YY_BUFFER_STATE *yy_scan_string(const char*);
extern "C" int yyparse(void);
extern struct AndList *final;

char* fileName = "Statistics.txt";

TEST(Test_Estimae, Test_Estimate1){
    bool flag=true;
    Statistics s;
        char *relName[] = {"orders","customer","nation"};

	s.AddRel(relName[0],1500000);
	s.AddAtt(relName[0], "o_custkey",150000);

	s.AddRel(relName[1],150000);
	s.AddAtt(relName[1], "c_custkey",150000);
	s.AddAtt(relName[1], "c_nationkey",25);
	
	s.AddRel(relName[2],25);
	s.AddAtt(relName[2], "n_nationkey",25);

	char *cnf = "(c_custkey = o_custkey)";
	yy_scan_string(cnf);
	yyparse();

	// Join the first two relations in relName
	s.Apply(final, relName, 2);
	
	cnf = " (c_nationkey = n_nationkey)";
	yy_scan_string(cnf);
	yyparse();
	
	double result = s.Estimate(final, relName, 3);
	cout << "Estimated result is " << result << endl;
	if(fabs(result-1500000)>0.1)
		flag=false;
	s.Apply(final, relName, 3);

	s.Write(fileName);
    ASSERT_TRUE(flag);
}

TEST(Test_Read, Test_Read1){
    bool flag=true;
    Statistics s;
        char *relName[] = {"supplier","partsupp"};

	
	s.AddRel(relName[0],10000);
	s.AddAtt(relName[0], "s_suppkey",10000);

	s.AddRel(relName[1],800000);
	s.AddAtt(relName[1], "ps_suppkey", 10000);	

	char *cnf = "(s_suppkey = ps_suppkey)";

	yy_scan_string(cnf);
	yyparse();
	double result = s.Estimate(final, relName, 2);
	if(result!=800000)
		flag=false;
	s.Apply(final, relName, 2);

	// test write and read
	s.Write(fileName);

	//reload the statistics object from file
	Statistics s1;
	s1.Read(fileName);	
	cnf = "(s_suppkey>1000)";	
	yy_scan_string(cnf);
	yyparse();
	double dummy = s1.Estimate(final, relName, 2);
	if(fabs(dummy*3.0-result) >0.1) flag=false;

    ASSERT_TRUE(flag);
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}