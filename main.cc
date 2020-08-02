
#include <iostream>
#include "Statistics.h"
#include "ParseTree.h"
#include "mainQueryPlan.h"

using namespace std;

extern "C" {
	int yyparse(void);
}

extern struct FuncOperator *finalFunction;
extern struct TableList *tables;
extern struct AndList *boolean;
extern struct NameList *groupingAtts;
extern struct NameList *attsToSelect;
extern int distinctAtts;
extern int distinctFunc;

int main() {
	cout << "Enter the Query followed by Ctrl + D:" << endl;
	yyparse();
	mainQueryPlan *mainQPObject= new mainQueryPlan(finalFunction, tables, boolean, groupingAtts, attsToSelect, distinctAtts, distinctFunc);
	mainQPObject->planGenerator();
}


