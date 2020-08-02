#ifndef MAINQUERYPLANER_H
#define MAINQUERYPLANER_H

#include <iostream>
#include <vector>
#include <stdlib.h>
#include "Statistics.h"
#include "ParseTree.h"
#include "Schema.h"
#include "Record.h"
#include "Pipe.h"
#include "Function.h"
#include "DbUtil.h"
#include "RelOp.h"
#include <stdio.h>
#include "DBFile.h"
#include <fstream>
#include <cstdlib>

using std::endl;
using namespace std;

#define PIPE_SIZE 100
enum NodeType { SELECTFILE, SELECTPIPE, SUM, DIST, JOIN, GROUPBY, PROJECT, WRITEOUT };

struct QueryPipes
{
	Pipe *queryPipe;
	int pipeID;
};
class QueryTreeNode
{
public:
	NodeType type;

	struct TreeNode{
		QueryTreeNode *parentNode;
		QueryTreeNode *leftChild;
		QueryTreeNode *rightChild;
	};
	TreeNode queryNode;
	QueryPipes leftPipe;
	QueryPipes rightPipe;
	QueryPipes outputPipes;
	Schema *nodeSchema;
	OrderMaker *order;
	string binPath;
	AndList *cnf;
	string path;
	Function *function;
	FuncOperator *funcOperator;

	SelectFile *sf;
	SelectPipe *sp;
	Join *j;
	GroupBy *gb;
	Project *p;
	Sum *s;
	DuplicateRemoval *d;
	WriteOut *h;
	

	DBFile *dbFile;
	//For project
	int numAttsIn;
	int numAttsOut;
	vector<int> aTK;
	int *attsToKeep;
	int numAttsToKeep;
	
	CNF *operatorCNF;
	Record *literal;

	//void RunInOrder();
	void Run();
	void WaitUntilDone();

	QueryTreeNode();

	QueryTreeNode(QueryTreeNode * lChild, NodeType nodetype, FuncOperator *selFunc, AndList *list, int opPipeID);
	QueryTreeNode(QueryTreeNode * leftChild, QueryTreeNode * rightChild, NodeType nodetype, AndList * list, int opPipeID);
	void setType(NodeType type);

	void GenerateSchema(NameList * selectAttr);
	//New
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
class MainQueryPlaner{
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

    MainQueryPlaner(FuncOperator *finalFunction, TableList *tables, AndList *boolean, NameList *groupingAtts, NameList *attsToSelect, int distinctAtts, int distinctFunc);
    vector<AndList> getJoinList();
    void getWhereHavingList(vector<AndList> &havingList, vector<AndList> &whereList);
    QueryTreeNode *currentParent(AndList tempWhere, map<string, QueryTreeNode*> leafNodes, Statistics *statistics);
    QueryTreeNode *joinParentNode(string tableName, map<string, QueryTreeNode*> leafNodes);
    vector<AndList> optmizeJoins(Statistics *statistics ,vector<AndList> joinsVector);
    void planGenerator(int outPutType, string dbfile_dir);
	//void getJoinsAndWhereClauses(vector<AndList> &joinList, vector<AndList> &havingList, vector<AndList> &whereList);
    int emptyPipe (Pipe &in_pipe, Schema *schema);
    int emptyPipeWithFunction (Pipe &in_pipe, Schema *schema, Function &func);
    void nodeOrderPrinter(QueryTreeNode *lastInserted);
    void nodeOrderExecution(QueryTreeNode *lastInserted);
};

class QueryHandler{
    public:
    char *catalog_path, *dbfile_dir, *tpch_dir = NULL;
    FuncOperator *finalFunction;
    TableList *tables;
    AndList *boolean;
    NameList *groupingAtts;
    NameList *attsToSelect;
    NameList *sortAtts;
    int distinctAtts;
    int distinctFunc;

    InOutPipe *io;

    string createTableName;
    string createTableType;
    vector<string> atts;
    vector<string> attTypes;
    //char *dbfile_dir;

    int queryType;

    QueryHandler();
    QueryHandler(FuncOperator *finalFunction, TableList *tables, AndList *boolean, NameList *groupingAtts, NameList *attsToSelect,
    NameList *sortAtts, int distinctAtts, int distinctFunc, InOutPipe *io, string createTableName, string createTableType,
    vector<string> atts, vector<string> attTypes, int queryType);
    int StartQuery();
    void GetLocations();
    void DropTable(string table);
    int CreateTable();
    int InsertToTable();
    void clear();
};

#endif