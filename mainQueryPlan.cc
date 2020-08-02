#include "mainQueryPlan.h"

// QueryTreeNode default constructor
QueryTreeNode :: QueryTreeNode()
{
	nodeSchema = NULL;
	orderMaker = NULL;
	queryNode.parentNode = NULL;
	queryNode.leftChild = NULL;
	queryNode.rightChild = NULL;
	cnf = NULL;
	leftPipe.pipeId = 0;
	leftPipe.queryPipe = NULL;
	rightPipe.pipeId = 0;
	rightPipe.queryPipe = NULL;
	outputPipes.pipeId = 0;
	outputPipes.queryPipe = NULL;
	function = NULL;
	funcOperator = NULL;
}

// QueryTreeNode parametrised constructor to initialize leftChild, nodetype, function operator, andList and pipeID
QueryTreeNode::QueryTreeNode(QueryTreeNode * node, NodeType nodetype, FuncOperator *funcOpe, AndList *andList, int pipeId)
{
	nodeSchema = node->nodeSchema;
	orderMaker = NULL;
	queryNode.leftChild = node;
	queryNode.rightChild = NULL;
	queryNode.parentNode = NULL;
	leftPipe.pipeId = node->outputPipes.pipeId;
	leftPipe.queryPipe = NULL;
	rightPipe.pipeId = 0;
	rightPipe.queryPipe = NULL;
	outputPipes.pipeId = pipeId;
	outputPipes.queryPipe = NULL;
	nodeType = nodetype;
	cnf = andList;
	funcOperator = funcOpe;

	if(funcOpe!=NULL){
		function = new Function();
		function->GrowFromParseTree(funcOperator, *nodeSchema);
	}
}

// QueryTreeNode parametrised constructor to initialize leftChild and rightChild
QueryTreeNode :: QueryTreeNode(QueryTreeNode * leftChildNode, QueryTreeNode * rightChildNode, NodeType nodetype, AndList *andList,int pipeId)
{
	nodeSchema = NULL;
	orderMaker = NULL;
	queryNode.leftChild = leftChildNode;
	queryNode.rightChild = rightChildNode;
	queryNode.parentNode = NULL;
	leftPipe.pipeId = leftChildNode->outputPipes.pipeId;
	leftPipe.queryPipe = NULL;
	rightPipe.pipeId = rightChildNode->outputPipes.pipeId;
	rightPipe.queryPipe = NULL;
	outputPipes.pipeId = pipeId;
	outputPipes.queryPipe = NULL;
	nodeType = nodetype;
	cnf = andList;
	createJoinSchema();
}

// Creates the schema for join operation by merging the attributes of both the left and right children nodes
void QueryTreeNode::createJoinSchema()
{
	int leftAttCount = queryNode.leftChild->nodeSchema->GetNumAtts();
	int rightAttCount = queryNode.rightChild->nodeSchema->GetNumAtts();
	int numOfAttr = leftAttCount+rightAttCount;
	Attribute *leftAttr = queryNode.leftChild->nodeSchema->GetAtts();
	Attribute *rightAttr = queryNode.rightChild->nodeSchema->GetAtts();
	Attribute mergedAtt[numOfAttr];
	int mergeAttPos = 0;
	for(; mergeAttPos<numOfAttr; mergeAttPos++){
		if(mergeAttPos<leftAttCount){
			mergedAtt[mergeAttPos] = leftAttr[mergeAttPos];
		}
		else{
			mergedAtt[mergeAttPos] = rightAttr[mergeAttPos-leftAttCount];
		}
	}
	nodeSchema = new Schema("Join" , numOfAttr, mergedAtt);
}

// Prints the select(File/Pipe) operation node.
void QueryTreeNode::selectPrinter(string operationType){
	cout << "SELECT "<< operationType <<" OPERATION" << endl;
	cout << "INPUT "<< operationType <<" " << leftPipe.pipeId << endl;
	cout << "OUTPUT "<< operationType <<" " << outputPipes.pipeId << endl;
	cout << "OUTPUT SCHEMA: " << endl;
	schemaPrinter(nodeSchema->GetNumAtts(), nodeSchema->GetAtts());
	cnfPrinter();
}

// Prints the aggrigate function node
void QueryTreeNode::aggrigatePrinter(string operationType){
	cout << operationType << endl;
	cout << "LEFT INPUT PIPE " << leftPipe.pipeId << endl;
	cout << "OUTPUT PIPE " << outputPipes.pipeId << endl;
	cout << "OUTPUT SCHEMA: " << endl;
	schemaPrinter(nodeSchema->GetNumAtts(), nodeSchema->GetAtts());
	cout << endl << "FUNCTION: " << endl;
	function->Print();
}

// Prints the Join operation node
void QueryTreeNode::joinPrinter(){
	cout << "JOIN" << endl;
	cout << "LEFT INPUT PIPE " << leftPipe.pipeId << endl;
	cout << "RIGHT INPUT PIPE " << rightPipe.pipeId << endl;
	cout << "OUTPUT PIPE " << outputPipes.pipeId << endl;
	cout << "OUTPUT SCHEMA: " << endl;
	schemaPrinter(nodeSchema->GetNumAtts(), nodeSchema->GetAtts());
	cnfPrinter();
	cout << endl;
}

// Prints the groupBy operation node
void QueryTreeNode::groupByPrinter(){
	cout << "GROUPBY" << endl;
	cout << "LEFT INPUT PIPE " << leftPipe.pipeId << endl;
	cout << "OUTPUT PIPE " << outputPipes.pipeId << endl;
	cout << "OUTPUT SCHEMA: " << endl;	
	schemaPrinter(nodeSchema->GetNumAtts(), nodeSchema->GetAtts());
	cout << endl << "GROUPING ON " << endl;
	cout << endl << "FUNCTION " << endl;
	function->Print();
}

// Prints the final projection
void QueryTreeNode::projectPrinter(){
	cout << "PROJECT" << endl;
	cout << "INPUT PIPE " << leftPipe.pipeId << endl;
	cout << "OUTPUT PIPE "<< outputPipes.pipeId << endl;
	cout << "OUTPUT SCHEMA: " << endl;
	orderMaker->Print();
	schemaPrinter(nodeSchema->GetNumAtts(), nodeSchema->GetAtts());
}

// Print writeout
void QueryTreeNode::writeOuttePrinter(){
	cout << "WRITE" << endl;
	cout << "LEFT INPUT PIPE " << leftPipe.pipeId << endl;
	cout << "OUTPUT FILE " << path << endl;
}

void QueryTreeNode::schemaPrinter(int numAtts, Attribute *schemaAttributes){

	for(int i=0; i < numAtts; i++)
	{
		cout << "\t" << "Att " << schemaAttributes[i].name<<": ";
		if(schemaAttributes[i].myType == 0)
			cout << "INT" << endl;
		else if(schemaAttributes[i].myType == 1)
			cout << "DOUBLE" << endl;
		else 
			cout << "STRING" << endl;
	}
}

// Prints the current node basing on its nodeType
void QueryTreeNode::nodePrinter(){
  cout << " ****************** " << endl;
  switch (nodeType){
    case SELECTFILE:
		selectPrinter("FILE");
    	break;

    case SELECTPIPE:
		selectPrinter("PIPE");
    	break;

	case SUM:
		aggrigatePrinter("SUM");
    	break;

    case DIST:
		aggrigatePrinter("DISTINCT");
		break;

    case JOIN:
		joinPrinter();
    	break;

    case GROUPBY:
		groupByPrinter();
    	break;

	case PROJECT:
		projectPrinter();
     	break;

    case WRITEOUT:
		writeOuttePrinter();
    	break;
  }
  cout << " ****************** " << endl;
} 

// To set the orderMaker
void QueryTreeNode::setOrderMaker(int numAtts, int *groupAtt, Type *attType){
	orderMaker = new OrderMaker();
	orderMaker->SetAttributes(numAtts, groupAtt, attType);
}

// To print the CNF of a Node operation if present
void QueryTreeNode::cnfPrinter()
{
	string str;
	if(cnf)
	{
		struct AndList *andList = cnf;

		while(andList!=NULL)
		{
			struct OrList *orList = andList->left;
			if(andList->left)
			{
				str.append("(");
			}
			while(orList)
			{
				struct ComparisonOp *comparisonOp = orList->left;
				if(comparisonOp)
				{
					if(comparisonOp->left)
					{
						str.append(comparisonOp->left->value);
					}

					str.append(comparisonOp->code == LESS_THAN ? "<" : comparisonOp->code == GREATER_THAN ? ">":"=");

					if(comparisonOp->right)
					{
						str.append(comparisonOp->right->value);
					}
				}
				if(orList->rightOr)
				{
					str.append("OR");
				}
				orList = orList->rightOr;
			}
			if(andList->left)
			{
				str.append(")");
			}
			if(andList->rightAnd)
			{
				str.append("AND");
			}
			andList = andList->rightAnd;
		}
		cout << endl << "CNF: " << endl;
		cout<< str << endl;
	}
}

// Constructor to initialize the mainQueryPlan attributes
mainQueryPlan::mainQueryPlan(FuncOperator *finalFunction, TableList *tables, AndList *boolean, NameList *groupingAtts, NameList *attsToSelect, int distinctAtts, int distinctFunc){
    this->finalFunction=finalFunction;
    this->tables=tables;
    this->boolean=boolean;
    this->groupingAtts=groupingAtts;
    this->attsToSelect=attsToSelect;
    this->distinctAtts=distinctAtts;
    this->distinctFunc=distinctFunc;
    lastInserted=NULL;

    statistics = new Statistics();
	statistics->Read("Statistics.txt");
}

// Return the join operations list by iterating trough the boolean
vector<AndList> mainQueryPlan::getJoinList(){
	AndList *currAndList= boolean;
	vector<AndList> joinList;
	while(currAndList!=0){
		OrList *curOrList=currAndList->left;
		AndList tmpAndList = *currAndList;
		if (curOrList && curOrList->left->code == EQUALS && curOrList->left->left->code == NAME
			&& curOrList->left->right->code == NAME)
		{
			tmpAndList.rightAnd = 0;
			joinList.push_back(tmpAndList);
			//break;
		}
		currAndList=currAndList->rightAnd;
	}
	return joinList;
}

// Constructos the where and having list by iterating trough the boolean
void mainQueryPlan::getWhereHavingList(vector<AndList> &havingList, vector<AndList> &whereList){
	AndList *currAndList= boolean;
	while(currAndList!=0){
		OrList *curOrList=currAndList->left;
		AndList tmpAndList = *currAndList;
		if(curOrList&&curOrList->left==0){
			tmpAndList.rightAnd = 0;
			whereList.push_back(tmpAndList);
			//break;
		}
		else if(curOrList==0 || curOrList->left->code != EQUALS || curOrList->left->left->code != NAME
			|| curOrList->left->right->code != NAME){
				vector<string> orTables;
				while (curOrList != 0)
				{
					Operand *operand = curOrList->left->left;
					if (operand->code != NAME)
						operand = curOrList->left->right;
                    string str= operand->value;
					string tableName = str.substr(0, str.find("."));
					if (find(orTables.begin(), orTables.end(), tableName) == orTables.end())
						orTables.push_back(tableName);
					curOrList = curOrList->rightOr;
				}
                AndList newAnd = *currAndList;
				newAnd.rightAnd = 0;
				if (orTables.size() <= 1) {
					whereList.push_back(newAnd);
				}
				else {
					havingList.push_back(newAnd);
					//break;
				}
			}
		currAndList=currAndList->rightAnd;
	}
}

// Return the parent node of where list node
QueryTreeNode *mainQueryPlan::currentParent(AndList tempWhere, map<string, QueryTreeNode*> leafNodes){
	string tablename = tempWhere.left->left->left->code == NAME? tempWhere.left->left->left->value:
																 tempWhere.left->left->right->value;

	QueryTreeNode *tmpNode;
	tmpNode = leafNodes[tablename.substr(0, tablename.find("."))];
	while (tmpNode->queryNode.parentNode != NULL)
	{
		tmpNode = tmpNode->queryNode.parentNode;
	}

	char *tempTableName = strdup(tablename.c_str());
	statistics->Apply(&tempWhere, &tempTableName, 1);

	return tmpNode;
}

// Returns the parent node of join node corresponding to the table node in leafNodes map
QueryTreeNode *mainQueryPlan::joinParentNode(string tableName, map<string, QueryTreeNode*> leafNodes){
	QueryTreeNode *curPlanNode;
	string curTable = tableName.substr(0, tableName.find("."));

	curPlanNode = leafNodes[curTable];
	while (curPlanNode->queryNode.parentNode != NULL)
		curPlanNode = curPlanNode->queryNode.parentNode;
	return curPlanNode;
}

// Optimizes the join operations vector
vector<AndList> mainQueryPlan::optmizeJoins(vector<AndList> joinsVector)
{
    if(joinsVector.size()==0) return joinsVector;
	vector<AndList> optimizedList;
	
	while (joinsVector.size() > 0)
	{	
        int maxEstimateIndex = 0;
		double maxCost = -1;
		for (int i = 0; i < joinsVector.size(); i++)
		{
			double curCost = 0.0;
            string leftValue = joinsVector[i].left->left->left->value;
			string leftTable = leftValue.substr(0, leftValue.find("."));
            string rightValue = joinsVector[i].left->left->right->value;
			string rightTable = rightValue.substr(0, rightValue.find("."));
			char* tableList[] = { (char*)leftTable.c_str(), (char*)rightTable.c_str() };
			curCost = statistics->Estimate(&joinsVector[i], tableList, 2);

			if (i == 0 || maxCost > curCost)
			{
				maxCost = curCost;
				maxEstimateIndex = i;
			}
		}
		optimizedList.push_back(joinsVector[maxEstimateIndex]);
		joinsVector.erase(joinsVector.begin() + maxEstimateIndex);
	}
	return optimizedList;
}

// To optimize and construct the query plan for the given input query
void mainQueryPlan::planGenerator(){

	int pipeCount = 1;
	TableList *queryTables = tables;
	map<string, QueryTreeNode*> leafNodeMap;

    // Fetching all the tbales involved in the query to generate the leaf nodes
	while (queryTables!=NULL)
	{
		string curTableName = queryTables->aliasAs!=0?queryTables->aliasAs:queryTables->tableName;
		leafNodeMap.insert(std::pair<string, QueryTreeNode*>(curTableName, new QueryTreeNode()));

		QueryTreeNode *curNode = leafNodeMap[queryTables->aliasAs];
        curNode->outputPipes.pipeId = pipeCount++;
		curNode->nodeType = SELECTFILE;
		curNode->nodeSchema = new Schema("catalog", queryTables->tableName);
		
		if(curTableName==queryTables->aliasAs){
			statistics->CopyRel(queryTables->tableName, queryTables->aliasAs);
			curNode->nodeSchema->updateName(curTableName);
		}

		lastInserted = curNode;
		queryTables = queryTables->next;
	}

	vector<AndList> tempJoinList = getJoinList();

	vector<AndList> tempHavingList;
	vector<AndList> tempWhereList;
	getWhereHavingList(tempHavingList, tempWhereList);

	// Creating the tree nodes of type select pipe for every where condition
	for (unsigned i = 0; i < tempWhereList.size(); i++)
	{
		QueryTreeNode *curParent = currentParent(tempWhereList[i], leafNodeMap);
		QueryTreeNode *curNode = new QueryTreeNode(curParent, SELECTPIPE, NULL, &tempWhereList[i], pipeCount++);
		curParent->queryNode.parentNode = curNode;
		lastInserted = curNode;
	}
	
	// Creating Join nodes for the tree plan
    tempJoinList = optmizeJoins(tempJoinList);
	AndList curJoin;
	for (int i = 0; i < tempJoinList.size(); i++)
	{
		QueryTreeNode *curLeftParent = joinParentNode(tempJoinList[i].left->left->left->value, leafNodeMap);
		QueryTreeNode *curRightParent = joinParentNode(tempJoinList[i].left->left->right->value, leafNodeMap);
		QueryTreeNode *curNode = new QueryTreeNode(curLeftParent, curRightParent, JOIN, &tempJoinList[i], pipeCount++);
	
		curLeftParent->queryNode.parentNode = curNode;
		curRightParent->queryNode.parentNode = curNode;
		lastInserted = curNode;
	}

	// Tree nodes for having
	for (int i = 0; i < tempHavingList.size(); i++)
	{
		QueryTreeNode *tmpNode = lastInserted;
		QueryTreeNode *curNode = new QueryTreeNode(tmpNode, SELECTPIPE, NULL, &tempHavingList[i], pipeCount++);
		tmpNode->queryNode.parentNode = curNode;
		lastInserted = curNode;
	}

	// Distinct(Duplicate) nodes
	if (finalFunction != NULL&&distinctFunc != NULL)
	{
		QueryTreeNode *tmpNode = lastInserted;
		QueryTreeNode *curNode = new QueryTreeNode(tmpNode, DIST, NULL, NULL, pipeCount++);
		tmpNode->queryNode.parentNode = curNode;
		lastInserted = curNode;
	}

    // Tree node for aggrigate functions
    bool groupingFlag = groupingAtts!=NULL;
	if (finalFunction != NULL)
	{
		NodeType curType = groupingAtts == NULL ? SUM: GROUPBY;
		QueryTreeNode *curparent = lastInserted;
		QueryTreeNode *curNode = new QueryTreeNode(curparent, curType, finalFunction, NULL, pipeCount++);
		curparent->queryNode.parentNode = curNode;
		lastInserted = curNode;

		if(groupingAtts != NULL){
			Schema *nodeSchema = curparent->nodeSchema;
			int groupAttCount = 0;
			vector<int> attsToGroup;
			vector<Type> whichType;
			while (groupingAtts) {
				groupAttCount++;

				attsToGroup.push_back(nodeSchema->Find(groupingAtts->name));
				whichType.push_back(nodeSchema->FindType(groupingAtts->name));
				groupingAtts = groupingAtts->next;
			}
			curNode->setOrderMaker(groupAttCount, &attsToGroup[0], &whichType[0]);
		}
	}

	// Project tree node
	if (attsToSelect != 0) {

		QueryTreeNode *tmpNode = lastInserted;
		QueryTreeNode *newNode = new QueryTreeNode(tmpNode, PROJECT, NULL, NULL, pipeCount++);
		tmpNode->queryNode.parentNode = newNode;

		int counter=0;
		Attribute *newAttr = new Attribute[counter];
		vector<int> attIndexes;
		NameList *tempAttsToSelect = attsToSelect;
		while (tempAttsToSelect != 0) {
			string attribute = tempAttsToSelect->name;
			attIndexes.push_back(newNode->nodeSchema->Find(const_cast<char*>(attribute.c_str())));
			counter++;
			tempAttsToSelect = tempAttsToSelect->next;	
		}
		Attribute *attr = newNode->nodeSchema->GetAtts();
		for (int i = 0; i < counter; i++)
		{
			newAttr[i] = attr[attIndexes[i]];
		}
		newNode->nodeSchema = new Schema("ProjectSchems", counter, newAttr);

        cout<< "Number of selects: "<< counter<< endl;
        cout<< "Number of joins: "<<tempJoinList.size()<< endl;
        if(groupingFlag) cout<< "GROUPING ON ";
		Schema *nodeSchema = tmpNode->nodeSchema;
		int numAttsToGroup = 0;
		vector<int> attsToGroup;
		vector<Type> whichType;
		while (attsToSelect) {

			numAttsToGroup++;
            cout<<attsToSelect->name<<endl;
            // cout<< " join list count:   "<<tempJoinList.size()<< endl; 
			attsToGroup.push_back(nodeSchema->Find(attsToSelect->name));
			whichType.push_back(nodeSchema->FindType(attsToSelect->name));
			attsToSelect = attsToSelect->next;
		}

		newNode->setOrderMaker(numAttsToGroup, &attsToGroup[0], &whichType[0]);

		lastInserted = newNode;
	}

    // Printing the plan tree nodes
	if(lastInserted != NULL)
	{
		stack<QueryTreeNode*> st;
		stack<QueryTreeNode*> postOrderSt;
		st.push(lastInserted);
		while(!st.empty()){
			QueryTreeNode *current = st.top();
			st.pop();
			postOrderSt.push(current);
			if(current->queryNode.leftChild) st.push(current->queryNode.leftChild);
			if(current->queryNode.rightChild) st.push(current->queryNode.rightChild);
		}

		while(!postOrderSt.empty()){
			QueryTreeNode *current = postOrderSt.top();
			postOrderSt.pop();
			current->nodePrinter();
		}
	}
}