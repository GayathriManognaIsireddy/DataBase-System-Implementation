#ifndef MAINQUERYPLAN_H
#define MAINQUERYPLAN_H

#include <iostream>
#include <vector>
#include <stdlib.h>
#include "Statistics.h"
#include "ParseTree.h"
#include "Schema.h"
#include "Record.h"
#include "Pipe.h"
#include "Function.h"

using std::endl;
using namespace std;

struct NodePipes
{
	Pipe *queryPipe;
	int pipeId;
};
enum NodeType { SELECTFILE, SELECTPIPE, SUM, DIST, JOIN, GROUPBY, PROJECT, WRITEOUT };

// To create tree nodes for all kinds of operations
class QueryTreeNode
{
public:

	struct TreeNode{
		QueryTreeNode *parentNode;
		QueryTreeNode *leftChild;
		QueryTreeNode *rightChild;
	};

	Schema *nodeSchema;
	OrderMaker *orderMaker;
	NodeType nodeType;
	TreeNode queryNode;
	NodePipes leftPipe;
	NodePipes rightPipe;
	NodePipes outputPipes;
	AndList *cnf;
	Function *function;
	FuncOperator *funcOperator;
	string path;

	QueryTreeNode();
	QueryTreeNode(QueryTreeNode * node, NodeType nodetype, FuncOperator *funcOpe, AndList *andList, int pipeId);
	QueryTreeNode(QueryTreeNode * leftChildNode, QueryTreeNode * rightChildNode, NodeType nodetype, AndList *andList, int pipeId);

	void createJoinSchema();
	void selectPrinter(string operationType);
	void aggrigatePrinter(string operationType);
	void joinPrinter();
	void groupByPrinter();
	void projectPrinter();
	void writeOuttePrinter();
	void schemaPrinter(int numAtts, Attribute *schemaAttributes);
	void nodePrinter();
	void cnfPrinter();
	void setOrderMaker(int numAtts, int *groupAtt, Type *attType);
};

// To optimize and implement the Query plan for given query in main
class mainQueryPlan{
    public:
    FuncOperator *finalFunction;
    TableList *tables;
    AndList *boolean;
    NameList *groupingAtts;
    NameList *attsToSelect;
    int distinctAtts;
    int distinctFunc;
    QueryTreeNode *lastInserted;
    Statistics *statistics;

    mainQueryPlan(FuncOperator *finalFunction, TableList *tables, AndList *boolean, NameList *groupingAtts, NameList *attsToSelect, int distinctAtts, int distinctFunc);
    vector<AndList> getJoinList();
    void getWhereHavingList(vector<AndList> &havingList, vector<AndList> &whereList);
    QueryTreeNode *currentParent(AndList tempWhere, map<string, QueryTreeNode*> leafNodes);
    QueryTreeNode *joinParentNode(string tableName, map<string, QueryTreeNode*> leafNodes);
    vector<AndList> optmizeJoins(vector<AndList> joinsVector);
    void planGenerator();
};

#endif