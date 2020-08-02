#include "MainQueryPlaner.h"

using namespace std;


QueryTreeNode :: QueryTreeNode()
{
	queryNode.leftChild=NULL;
	queryNode.rightChild=NULL;
	queryNode.parentNode=NULL;
	nodeSchema = NULL;
	operatorCNF = NULL;
	order = NULL;
	cnf = NULL;
	function = NULL;
	literal = new Record();
	funcOperator = NULL;
	leftPipe.pipeID = 0;
	leftPipe.queryPipe = NULL;

	rightPipe.pipeID = 0;
	rightPipe.queryPipe = NULL;

	outputPipes.pipeID = 0;
	outputPipes.queryPipe = NULL;
}

QueryTreeNode::QueryTreeNode(QueryTreeNode * lChild, NodeType nodetype, FuncOperator *opr, AndList *list, int opPipeID)
{
	leftPipe.pipeID = 0;
	leftPipe.queryPipe = NULL;

	rightPipe.pipeID = 0;
	rightPipe.queryPipe = NULL;

	outputPipes.pipeID = 0;
	outputPipes.queryPipe = NULL;
	operatorCNF = NULL;

	nodeSchema = lChild->nodeSchema;
	literal = new Record();
	cnf = list;
	type = nodetype;
	if(nodetype != PROJECT)
		setType(nodetype);
	funcOperator = opr;
	queryNode.leftChild = lChild;
	queryNode.rightChild = NULL;
	leftPipe.pipeID = lChild->outputPipes.pipeID;
	leftPipe.queryPipe = lChild->outputPipes.queryPipe; 

	outputPipes.pipeID = opPipeID;
	queryNode.parentNode = NULL;
	if(opr!=NULL){
		function = new Function();
		function->GrowFromParseTree(funcOperator, *nodeSchema);
	}
}

QueryTreeNode :: QueryTreeNode(QueryTreeNode * lChild, QueryTreeNode * rChild, NodeType nodetype, AndList * list,int opPipeID)
{

	leftPipe.pipeID = 0;
	leftPipe.queryPipe = NULL;

	rightPipe.pipeID = 0;
	rightPipe.queryPipe = NULL;

	outputPipes.pipeID = 0;
	outputPipes.queryPipe = NULL;

	cnf = list;
	type = nodetype;
	literal = new Record();
	leftPipe.pipeID = lChild->outputPipes.pipeID;
	rightPipe.pipeID = rChild->outputPipes.pipeID;
	leftPipe.queryPipe = lChild->outputPipes.queryPipe;
	rightPipe.queryPipe = rChild->outputPipes.queryPipe;

	operatorCNF = NULL;
	outputPipes.pipeID = opPipeID;

	queryNode.leftChild = lChild;
	queryNode.rightChild = rChild;
	queryNode.parentNode = NULL;
	if (nodetype == JOIN){
		createJoinSchema();
		operatorCNF = new CNF();
		operatorCNF->GrowFromParseTree(cnf, queryNode.leftChild->nodeSchema, queryNode.rightChild->nodeSchema, *literal);
	}
	setType(nodetype);
}


void QueryTreeNode::Run(){

	switch (type){

		case SELECTFILE:
			cout << "Running select file" << endl;
			sf->Run(*dbFile, *outputPipes.queryPipe, *operatorCNF, *literal);
		    break;

		case SELECTPIPE:
			cout << "Running select pipe" << endl;
			sp->Run(*leftPipe.queryPipe, *outputPipes.queryPipe, *operatorCNF, *literal);
		    break;

		case PROJECT:
			cout << "Running project" << endl;
			p->Run(*leftPipe.queryPipe, *outputPipes.queryPipe, attsToKeep, numAttsIn, numAttsOut);
		    break;

		case JOIN:
			cout << "Running join" << endl;
			j->Run(*leftPipe.queryPipe, *rightPipe.queryPipe, *outputPipes.queryPipe, *operatorCNF, *literal);
		    break;

		case SUM:
			cout << "Running SUM " << endl;
			s->Run(*leftPipe.queryPipe, *outputPipes.queryPipe, *function);
		  break;

		case GROUPBY:
		cout << "RUnning Groupby" << endl;
			gb->Run(*leftPipe.queryPipe, *outputPipes.queryPipe, *order, *function );
			break;

		case DIST:
			cout << "Running duplicate removal " << endl;
			d->Run(*leftPipe.queryPipe, *outputPipes.queryPipe, *nodeSchema);
		    break;

		case WRITEOUT:
		    break;
	}
}

void QueryTreeNode::WaitUntilDone(){
	switch (type){

		case SELECTFILE:
			sf->WaitUntilDone();
		    break;

		case SELECTPIPE:
			sp->WaitUntilDone();
		    break;

		case PROJECT:
			p->WaitUntilDone();
		    break;

		case JOIN:
			j->WaitUntilDone();
		    break;

		case SUM:
			s->WaitUntilDone();
		    break;

		case GROUPBY:
			gb->WaitUntilDone();
		  break;

		case DIST:
			d->WaitUntilDone();
		  break;

		case WRITEOUT:
		    break;
	}
}


void QueryTreeNode::setType(NodeType setter){

	outputPipes.queryPipe = new Pipe(100);
	switch (type){

	    case SELECTFILE:
				cout << "incase " << SELECTFILE << endl;
				dbFile = new DBFile();
				cout << "******************** Selecting the path " << path.c_str() << endl;
				dbFile->Open((char*)(path.c_str()));
				sf = new SelectFile();
				sf->Use_n_Pages(100);	
				break;

	    case SELECTPIPE:
				sp = new SelectPipe();
				operatorCNF = new CNF();
				operatorCNF->GrowFromParseTree(cnf, nodeSchema, *literal);
				sp->Use_n_Pages(100);
	      break;

	    case PROJECT:
				p = new Project();
				numAttsToKeep = (int) aTK.size();
				attsToKeep = new int[numAttsToKeep];
				for (int i = 0; i < numAttsToKeep; i++){
		  			attsToKeep[i] = aTK[i];
				}	
				p->Use_n_Pages(100);
				break;

	    case JOIN:

	      j = new Join();
				j->Use_n_Pages(100);
	      break;

	    case SUM:

	      s = new Sum();
				s->Use_n_Pages(100);
				break;

	    case GROUPBY:

	     	gb = new GroupBy();
				gb->Use_n_Pages(100);
	     	break;

	    case DIST:

	     	d = new DuplicateRemoval();
				d->Use_n_Pages(100);
	     	break;

	    case WRITEOUT:
	      break;
	  }
}


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
	cout << "INPUT "<< operationType <<" " << leftPipe.pipeID << endl;
	cout << "OUTPUT "<< operationType <<" " << outputPipes.pipeID << endl;
	cout << "OUTPUT SCHEMA: " << endl;
	schemaPrinter(nodeSchema->GetNumAtts(), nodeSchema->GetAtts());
	cnfPrinter();
}

// Prints the aggrigate function node
void QueryTreeNode::aggrigatePrinter(string operationType){
	cout << operationType << endl;
	cout << "LEFT INPUT PIPE " << leftPipe.pipeID << endl;
	cout << "OUTPUT PIPE " << outputPipes.pipeID << endl;
	cout << "OUTPUT SCHEMA: " << endl;
	schemaPrinter(nodeSchema->GetNumAtts(), nodeSchema->GetAtts());
	cout << endl << "FUNCTION: " << endl;
	function->Print();
}

// Prints the Join operation node
void QueryTreeNode::joinPrinter(){
	cout << "JOIN" << endl;
	cout << "LEFT INPUT PIPE " << leftPipe.pipeID << endl;
	cout << "RIGHT INPUT PIPE " << rightPipe.pipeID << endl;
	cout << "OUTPUT PIPE " << outputPipes.pipeID << endl;
	cout << "OUTPUT SCHEMA: " << endl;
	schemaPrinter(nodeSchema->GetNumAtts(), nodeSchema->GetAtts());
	cnfPrinter();
	cout << endl;
}

// Prints the groupBy operation node
void QueryTreeNode::groupByPrinter(){
	cout << "GROUPBY" << endl;
	cout << "LEFT INPUT PIPE " << leftPipe.pipeID << endl;
	cout << "OUTPUT PIPE " << outputPipes.pipeID << endl;
	cout << "OUTPUT SCHEMA: " << endl;	
	schemaPrinter(nodeSchema->GetNumAtts(), nodeSchema->GetAtts());
	cout << endl << "GROUPING ON " << endl;
	order->Print();
	cout << endl << "FUNCTION " << endl;
	function->Print();
}

// Prints the final projection
void QueryTreeNode::projectPrinter(){
	cout << "PROJECT" << endl;
	cout << "INPUT PIPE " << leftPipe.pipeID << endl;
	cout << "OUTPUT PIPE "<< outputPipes.pipeID << endl;
	cout << "OUTPUT SCHEMA: " << endl;
	//order->Print();
	schemaPrinter(nodeSchema->GetNumAtts(), nodeSchema->GetAtts());
}

// Print writeout
void QueryTreeNode::writeOuttePrinter(){
	cout << "WRITE" << endl;
	cout << "LEFT INPUT PIPE " << leftPipe.pipeID << endl;
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
  switch (type){
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

void QueryTreeNode::GenerateSchema(NameList * selectAttr)
{
	vector<int> indexOfAttsToKeep;
	string attribute;
	while (selectAttr != 0) {
		attribute = selectAttr->name;
		//nodeSchema->Print();
		indexOfAttsToKeep.push_back(nodeSchema->Find(const_cast<char*>(attribute.c_str())));
		selectAttr = selectAttr->next;
		
	}
	Attribute *attr = nodeSchema->GetAtts();
	Attribute *newAttr = new Attribute[indexOfAttsToKeep.size()];
	for (int i = 0; i < indexOfAttsToKeep.size(); i++)
	{
		cout << i << endl;
		newAttr[i] = attr[indexOfAttsToKeep[i]];
	}
	nodeSchema = new Schema("Distinct", indexOfAttsToKeep.size(), newAttr);
}

void QueryTreeNode::setOrderMaker(int numAtts, int *groupAtt, Type *attType){
	order = new OrderMaker();
	order->SetAttributes(numAtts, groupAtt, attType);
}


MainQueryPlaner::MainQueryPlaner(FuncOperator *finalFunction, TableList *tables, AndList *boolean, NameList *groupingAtts, NameList *attsToSelect, int distinctAtts, int distinctFunc){
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

int MainQueryPlaner::emptyPipe (Pipe &in_pipe, Schema *schema) {
	Record rec;
	int cnt = 0;
	while (in_pipe.Remove (&rec)) {
		rec.Print (schema);
		cnt++;
	}
	return cnt;
}

int MainQueryPlaner::emptyPipeWithFunction (Pipe &in_pipe, Schema *schema, Function &function) {
	Record rec;
	int cnt = 0;
	double sum = 0;
	cout<< "In sume clear pipe "<<endl;
	while (in_pipe.Remove (&rec)) {
		cout<< "In while clear pipe "<<endl;
		int ival = 0; double dval = 0;
		function.Apply (rec, ival, dval);
		sum += (ival + dval);
		cnt++;
		cout << " Sum: " << sum << endl;
		cout<< "In while clear pipe end "<<endl;
	}
	return cnt;
}

// Return the join operations list by iterating trough the boolean
vector<AndList> MainQueryPlaner::getJoinList(){
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
void MainQueryPlaner::getWhereHavingList(vector<AndList> &havingList, vector<AndList> &whereList){
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

// Optimizes the join operations vector
vector<AndList> MainQueryPlaner::optmizeJoins(Statistics *statistics, vector<AndList> joinsVector)
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


void MainQueryPlaner::nodeOrderPrinter(QueryTreeNode *lastInserted){
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

void MainQueryPlaner::nodeOrderExecution(QueryTreeNode *lastInserted){
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
		current->Run();
	}
}

// Return the parent node of where list node
QueryTreeNode *MainQueryPlaner::currentParent(AndList tempWhere, map<string, QueryTreeNode*> leafNodes, Statistics *statistics){
	string tablename = tempWhere.left->left->left->code == NAME? tempWhere.left->left->left->value:
																 tempWhere.left->left->right->value;
	tablename = tablename.substr(0, tablename.find("."));
	QueryTreeNode *tmpNode;
	tmpNode = leafNodes[tablename];
	while (tmpNode->queryNode.parentNode != NULL)
	{
		tmpNode = tmpNode->queryNode.parentNode;
	}

	char *tempTableName = strdup(tablename.c_str());
	statistics->Apply(&tempWhere, &tempTableName, 1);

	return tmpNode;
}

// Returns the parent node of join node corresponding to the table node in leafNodes map
QueryTreeNode *MainQueryPlaner::joinParentNode(string tableName, map<string, QueryTreeNode*> leafNodes){
	QueryTreeNode *curPlanNode;
	string curTable = tableName.substr(0, tableName.find("."));

	curPlanNode = leafNodes[curTable];
	while (curPlanNode->queryNode.parentNode != NULL)
		curPlanNode = curPlanNode->queryNode.parentNode;
	return curPlanNode;
}

void MainQueryPlaner::planGenerator(int output, string outputFile)
{
    int pipeID = 1;
	//cout << "Enter the Query followed by Ctrl + D:" << endl;
	Statistics *stats = new Statistics();
	stats->Read("Statistics.txt");

	TableList *tempTblList = tables;
	map<string, QueryTreeNode*> leafNodes;
	QueryTreeNode *lastInserted = NULL;

	while (tempTblList)
	{
		string curTableName = tempTblList->aliasAs!=0?tempTblList->aliasAs:tempTblList->tableName;
		char * tableName = tempTblList->tableName;
		string tablePath = string(tableName).append(".bin");
		cout<< "table name --  :"<< tablePath << endl;
		leafNodes.insert(std::pair<string, QueryTreeNode*>(curTableName, new QueryTreeNode()));

		QueryTreeNode *curNode = leafNodes[tempTblList->aliasAs];
		curNode->nodeSchema = new Schema("catalog", tempTblList->tableName);
		
		if(tempTblList->aliasAs!=0){
			stats->CopyRel(tempTblList->tableName, tempTblList->aliasAs);
			curNode->nodeSchema->updateName(curTableName);
		}

		curNode->path = tablePath;

		cout << "Settign the path " << curNode->path << endl;
		curNode->outputPipes.pipeID = pipeID++;
		curNode->type = SELECTFILE;
		curNode->setType(SELECTFILE);
		tempTblList = tempTblList->next;
		lastInserted = curNode;

	}

	vector<AndList> joinlist = getJoinList();
	vector<AndList> havingList;
	vector<AndList> whereList;
	getWhereHavingList(havingList, whereList);

	for (unsigned i = 0; i < whereList.size(); i++)
	{
		QueryTreeNode *tmpNode = currentParent(whereList[i], leafNodes, stats);
		//topnode
		QueryTreeNode *newNode = new QueryTreeNode(tmpNode, SELECTPIPE, NULL, &whereList[i], pipeID++);
		tmpNode->queryNode.parentNode = newNode;
		lastInserted = newNode;
	}
	if (joinlist.size() > 1) {
		joinlist = optmizeJoins(stats, joinlist);
	}

	for (int i = 0; i < joinlist.size(); i++)
	{
		QueryTreeNode *curLeftParent = joinParentNode(joinlist[i].left->left->left->value, leafNodes);
		QueryTreeNode *curRightParent = joinParentNode(joinlist[i].left->left->right->value, leafNodes);
		QueryTreeNode *newNode = new QueryTreeNode(curLeftParent, curRightParent, JOIN, &joinlist[i], pipeID++);
	
		curLeftParent->queryNode.parentNode = newNode;
		curRightParent->queryNode.parentNode = newNode;
		lastInserted = newNode;
	}
	
	// Tree nodes for having
	for (int i = 0; i < havingList.size(); i++)
	{
		cout<< "Inside having one" << endl;
		QueryTreeNode *tmpNode = lastInserted;
		QueryTreeNode *curNode = new QueryTreeNode(tmpNode, SELECTPIPE, NULL, &havingList[i], pipeID++);
		tmpNode->queryNode.parentNode = curNode;
		lastInserted = curNode;
	}

	// Distinct(Duplicate) nodes
	bool distFlag = false;
	if (finalFunction != NULL&&distinctFunc == 1)
	{
		cout<< "Inside Dist one" << endl;
		QueryTreeNode *tmpNode = lastInserted;
		QueryTreeNode *curNode = new QueryTreeNode(tmpNode, DIST, NULL, NULL, pipeID++);
		tmpNode->queryNode.parentNode = curNode;
		lastInserted = curNode;
	}

    // Tree node for aggrigate functions
    bool groupingFlag = groupingAtts!=NULL;
	if (finalFunction != NULL)
	{
		cout<< "Inside Aggrigate one" << endl;
		NodeType curType = groupingAtts == NULL ? SUM: GROUPBY;
		QueryTreeNode *curparent = lastInserted;
		QueryTreeNode *curNode = new QueryTreeNode(curparent, curType, finalFunction, NULL, pipeID++);
		curparent->queryNode.parentNode = curNode;
		lastInserted = curNode;
		curparent = lastInserted;

		if(groupingAtts != NULL){
			Schema *nodeSchema = curparent->nodeSchema;
			int groupAttCount = 0;
			vector<int> attsToGroup;
			vector<Type> whichType;
			while (groupingAtts) {
				groupAttCount++;

				cout<<" group attribute:  " << groupingAtts->name<< endl;
				attsToGroup.push_back(nodeSchema->Find(groupingAtts->name));
				whichType.push_back(nodeSchema->FindType(groupingAtts->name));
				groupingAtts = groupingAtts->next;
			}
			curNode->setOrderMaker(groupAttCount, &attsToGroup[0], &whichType[0]);
		}

	}



	//Creating distinct nodes
	if (distinctAtts != 0)
	{
		distFlag = true;
		QueryTreeNode *tmpNode = lastInserted;
		QueryTreeNode *newNode = new QueryTreeNode(tmpNode, DIST, NULL, NULL, pipeID++);
		tmpNode->queryNode.parentNode = newNode;
		lastInserted = newNode;
	}

	//Creating Project nodes
	if (attsToSelect != NULL) {

		QueryTreeNode *tmpNode = lastInserted;

		cout << " Creating the PROJECT  NOde " << endl;
		QueryTreeNode *newNode = new QueryTreeNode(tmpNode, PROJECT, NULL, NULL, pipeID++);
		tmpNode->queryNode.parentNode = newNode;
		//newNode->GenerateSchema(attsToSelect);


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
        cout<< "Number of joins: "<<joinlist.size()<< endl;
        if(groupingFlag) cout<< "GROUPING ON ";
		Schema *nodeSchema = tmpNode->nodeSchema;



		NameList *attsTraverse = attsToSelect;
		string attribute;
		vector<int> indexOfAttsToKeep;
		Schema *oldSchema = tmpNode->nodeSchema;

		while(attsTraverse != 0)
		{
			attribute = attsTraverse->name;

			indexOfAttsToKeep.push_back(oldSchema->Find(const_cast<char*>(attribute.c_str())));
			attsTraverse = attsTraverse->next;
		}

		Schema *newSchema = new Schema(oldSchema, indexOfAttsToKeep);
		newNode->nodeSchema = newSchema;

		newNode->numAttsIn = tmpNode->nodeSchema->GetNumAtts();
		newNode->numAttsOut = newNode->nodeSchema->GetNumAtts();
		newNode->aTK = indexOfAttsToKeep;

		newNode->setType(PROJECT);

		lastInserted = newNode;
	}


   if(output == PIPE_NONE)
   {
	   if(lastInserted != NULL)
	   {
		   nodeOrderPrinter(lastInserted);
	   }
   }
	else if(output == PIPE_STDOUT)
	{
		nodeOrderPrinter(lastInserted);
		cout << "*************************************************" << endl;
		cout << "Running the Tree" << endl;
		//lastInserted->RunInOrder();
		nodeOrderExecution(lastInserted);
		int count = 0;
	
		lastInserted->nodeSchema->Print();
		cout << "Schema  type " << lastInserted->type<<endl;
		if(lastInserted->type == SUM)
			count = emptyPipeWithFunction(*(lastInserted->outputPipes.queryPipe), lastInserted->nodeSchema, *lastInserted->function);
		else
			count = emptyPipe(*(lastInserted->outputPipes.queryPipe), lastInserted->nodeSchema);
		lastInserted->WaitUntilDone();
		clog << "Result records: " << count << endl;
	}
	
}

QueryHandler::QueryHandler(){

}

QueryHandler::QueryHandler(FuncOperator *finalFunction, TableList *tables, AndList *boolean, NameList *groupingAtts, NameList *attsToSelect,
    NameList *sortAtts, int distinctAtts, int distinctFunc, InOutPipe *io, string createTableName, string createTableType,
    vector<string> atts, vector<string> attTypes, int queryType){
        this->finalFunction = finalFunction;
        this->tables = tables;
        this->boolean = boolean;
        this->groupingAtts = groupingAtts;
        this->attsToSelect = attsToSelect;
        this->sortAtts = sortAtts;
        this->distinctAtts = distinctAtts;
        this->distinctFunc = distinctFunc;
        this->io = io;
        this->createTableName = createTableName;
        this->createTableType = createTableType;
        this->atts = atts;
        this->attTypes = attTypes;
        this->queryType = queryType;
    }

int QueryHandler::StartQuery(){
    int output = PIPE_STDOUT;
    string outputFile = "";

    GetLocations();
    //cout<< "IN start Query"<< endl;
    cout << "Done reading the catalog, bin and tpch file locations" << endl;

    int option;
    bool started=false;
    cout<<"jkbejhfhkuhskuf==================     "<< queryType<< endl;
    switch (queryType){
        case QUERY_CREATE:
            {
                CreateTable();
                cout << "Table " << createTableName.c_str() << " created " << endl;
                atts.clear();
                attTypes.clear();
            }
            break;
        case QUERY_SELECT:
            {
                MainQueryPlaner *mainQPObject= new MainQueryPlaner(finalFunction, tables, boolean, groupingAtts, attsToSelect, distinctAtts, distinctFunc);
	            mainQPObject->planGenerator(output, outputFile);
            }
            break;
        case QUERY_DROP:
            {
                DropTable(tables->tableName);
                cout << "Table " << tables->tableName << " is dropped " << endl;
                free(tables);
            }
            break;
        case QUERY_SET:
            {
                output = io->type;
                if(output == PIPE_FILE)
                {
                    outputFile = io->file;
                }
                free(io);
            }
            break;
        case QUERY_INSERT:
            {
                InsertToTable();
            }
            break;
        case QUERY_QUIT:
            {
                cout << "Shutting down the database" << endl;
            }
            break;
    }
    clear();
}


int QueryHandler::InsertToTable(){
    bool statusFlag=true;
    DBFile table;
    char *tableBinPath = new char[100];
    sprintf(tableBinPath, "%s%s.%s", dbfile_dir, io->src, "bin");

    Schema schema("catalog", io->src);
    ifstream curFile(io->file);

    cout << "Data file: " << io->file << endl;
    cout << "Table bin path: " << tableBinPath << endl;
    if (curFile.is_open())
    {
        table.Open(tableBinPath);
        table.Load(schema, io->file);
        table.Close();
        cout << "Data loaded: " << io->src << " From: " << io->file << "." << endl;
    }
    else
    {
        statusFlag = false;
        cout << "File is missing" << endl;
    }
    curFile.close();
    free(io);
    delete[] tableBinPath;
    return statusFlag==true;
}

void QueryHandler::GetLocations()
{
    const char *locations = "test.cat";
    FILE *filePaths = fopen(locations, "r");


    if(!filePaths)
    {
        cerr << " Test settings files 'test.cat' missing \n";
		exit (1);
    }
    else
    {
        char *mem = (char *) malloc(80 * 3);
        catalog_path = &mem[0];
		dbfile_dir = &mem[80];
		tpch_dir = &mem[160];
		char line[80];

        fgets (line, 80, filePaths);
		sscanf (line, "%s\n", catalog_path);
		fgets (line, 80, filePaths);
		sscanf (line, "%s\n", dbfile_dir);
		fgets (line, 80, filePaths);
		sscanf (line, "%s\n", tpch_dir);
		fclose (filePaths);
		if (! (catalog_path && dbfile_dir && tpch_dir)) {
			cerr << " Test settings file 'test.cat' not in correct format.\n";
			free (mem);
			exit (1);
		}
    }
}


void QueryHandler::DropTable(string table)
{
    bool copyData = true;

    string readLine;
    string hold = "";
    ifstream catalog("catalog");
    ofstream tempCatalog("tempCatalog");


    if(catalog.is_open())
    {
        while(catalog.good())
        {
            getline(catalog, readLine);

            if (readLine.compare("BEGIN") == 0 && hold.compare("") != 0)
            {
                if(tempCatalog.good() && copyData)
                {
                    tempCatalog << hold;
                }

                hold = "";
                hold.append(readLine);
                hold.append("\n");
            }
            else if(copyData == false && readLine.compare("END") == 0)
            {
                copyData = true;
                continue;
            }
            else if(readLine.compare(table) == 0)
            {
                hold = "";
                copyData = false;
            }
            else if(copyData)
            {
                hold.append(readLine);
                hold.append("\n");
            }
        }


        if(tempCatalog.good() && copyData)
        {
            tempCatalog << hold;
        }

        rename("tempCatalog", "catalog");
        catalog.close();

        char *file = new char[100];
        sprintf(file, "%s%s.%s", dbfile_dir, (char*)table.c_str(), "bin");
        remove(file);
        delete[] file;
    }
}


int QueryHandler::CreateTable()
{
    int createStatus=0;
    string curSchema;
    string type;
    int size = (int) atts.size();

    ofstream create("catalog", ios::out | ios::app);

    curSchema = "BEGIN\n";

    curSchema.append(createTableName.c_str());
    curSchema.append("\n");
    curSchema.append(createTableName.c_str());
    curSchema.append(".tbl\n");

    for (int i = 0; i < size; i++)
    {
        curSchema.append(atts[i]);

        if (attTypes[i].compare("INTEGER") == 0){
            type = " Int";
        }
        else if (attTypes[i].compare("DOUBLE") == 0){
            type = " Double";
        }
        else {
            type = " String";
        }
		curSchema.append(type);
    	curSchema.append("\n");
	}

	curSchema.append("END\n\n");

	if(create.is_open() && create.good())
	{
    	create << curSchema;
	}

	create.close();
  
	char *binary = new char[100];
	cout << "table Type:" << (char*)createTableType.c_str() << endl;
  	DBFile dbFile;
    sprintf(binary, "%s%s.%s", dbfile_dir, (char*) createTableName.c_str(), "bin");
    cout << "The path to the binary files is " << binary << endl;
	struct NameList *sortkeys = sortAtts;
    fType ftype = heap;

    if(sortAtts != NULL && sortAtts != 0)
    {
		int i = 0;
		OrderMaker om = OrderMaker();
		int attList[MAX_ANDS];
		Type attType[MAX_ANDS];
		Schema *s = new Schema("catalog", (char*)createTableName.c_str());
		while (sortkeys != NULL)
		{
			int res = s->Find(sortkeys->name);
			int restype = s->FindType(sortkeys->name);
			if (res != -1)
			{
				attList[i] = res;
				attTypes[i] = restype;
				i++;
			}
			sortkeys = sortkeys->next;
		}
		om.SetAttributes(i, attList, attType);
		struct { OrderMaker *o; int l; } startup = { &om, 5 };
        ftype = sorted;
		createStatus = dbFile.Create(binary, ftype, &startup);
    }
	else
		createStatus = dbFile.Create(binary, ftype, NULL);
    dbFile.Close();

    delete[] binary;
    return createStatus;
}

void QueryHandler::clear() {
	tables = NULL;
	boolean = NULL;
	groupingAtts = NULL;
	attsToSelect = NULL;
	sortAtts = NULL;
	io = NULL;
	queryType = -1;
	distinctAtts = distinctFunc = 0;
}