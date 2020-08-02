#include "Statistics.h"

Statistics::Statistics()
{
	  initializeTableMap();	
}

// Constructor to perform the deep copy of the copyMe object
Statistics::Statistics(Statistics &copyMe)
{
	this->relationMap = copyMe.relationMap;
	this->relationsInfoMap = copyMe.relationsInfoMap;
  	this->relationsInfoVector = copyMe.relationsInfoVector;
  	this->tablesMap = copyMe.tablesMap;
}
Statistics::~Statistics()
{
	
}

// Adds the relation formed by the given relName and numTuples to the relationMap, if the relation already present updates the numTuples.
void Statistics::AddRel(char *relName, int numTuples)
{
	string relationName(relName);
	RelationInfo newRelation(numTuples);
	RelationsUnionInfo newRelationsInfo(relationName, numTuples);

	// Case for new relation
	if(relationMap.find(relationName) == relationMap.end())
	{	
		relationMap[relationName] = newRelation;		
		relationsInfoMap[relationName] = newRelationsInfo;
		relationsInfoVector.push_back(newRelationsInfo);
	}
	// if the relation is already present
	else
	{
		relationMap[relationName].setNumTuples(numTuples);
		relationsInfoMap[relationName].setNumTuples(numTuples);
		relationsInfoMap[relationName].insertToRelationsSet(relationName);
	}
}

// To add the given attribute along with its numDistances to the esixting relation
void Statistics::AddAtt(char *relName, char *attName, int numDistincts)
{		
	string relationName(relName);

	if(relationMap.find(relationName) != relationMap.end())
	{
		relationMap[relationName].addAttribute(attName, numDistincts);
	}
	else{
		cout<< "Given relation is not already exists"<< endl;
	}
}

// To copy the given existing relation (oldName) to a new relation
void Statistics::CopyRel(char *oldName, char *newName)
{

	if(relationMap.find(oldName) != relationMap.end())
	{
		RelationInfo newRelation;
		newRelation.setNumTuples(relationMap[oldName].getNumTuples());
		newRelation.setAttributesMap(relationMap[oldName].getAttributes());
		relationMap[newName] = newRelation;

		RelationsUnionInfo newRelationsInfo(newName, newRelation.getNumTuples());
		relationsInfoMap[newName] = newRelationsInfo;
		relationsInfoVector.push_back(newRelationsInfo);
	}
	else{
		cout<< "Old relation is not present in the relations map"<< endl;
	}
}

// To read the content of statics.txt file, this first ready the total number of relations and there details info objects then once after
// reading the relation it ready the relationsUnionInfo objects present in the file
void Statistics::Read(char *fromWhere)
{
	ifstream fileInReder(fromWhere);
	if(fileInReder.is_open())
	{
		string currentLine;

		getline(fileInReder, currentLine);
		int relationsCount = stoi(currentLine);

		for(int i=0; i < relationsCount; i++)
		{
			getline(fileInReder, currentLine);
			string relation = currentLine;
			RelationInfo relationToRead;
			relationToRead.readInputFileStream(fileInReder);

			relationMap[relation] = relationToRead;
		}

		getline(fileInReder, currentLine);
		// To differenciate the relation to relationsUnion
		if(currentLine.compare("RelationUnionInfo Start --") != 0)
		{
			cout<< "ResultSet results are not present"<< endl;
			return;
		}

		getline(fileInReder, currentLine);
		int resultSetsCount = stoi(currentLine);

		for(int i=0; i < resultSetsCount; i++)
		{
			RelationsUnionInfo relationsInfoToRead;

			relationsInfoToRead.readInputFileStream(fileInReder);
			relationsInfoVector.push_back(relationsInfoToRead);

			set<string> tempRelationsSet= relationsInfoToRead.getRelationsSet();

			for(auto entry = tempRelationsSet.begin(); entry != tempRelationsSet.end(); entry++)
			{
				relationsInfoMap[*entry] = relationsInfoToRead;
			}
		}
	}

	fileInReder.close();
}

// Funtion to write the relation infos of all the relation till moment and the relationsUnion information after the relations information
void Statistics::Write(char *fromWhere)
{
	ofstream fileOurWriter(fromWhere);
	
	if(fileOurWriter.good()){
		// Writing all the relation information that includes the relation name, number of tuples, attribute count, attribute name and attribute distincts
		if(relationMap.size()>0){
			fileOurWriter << relationMap.size()<< endl;

			for(auto entry= relationMap.begin(); entry!=relationMap.end(); entry++){

				fileOurWriter<< entry->first<< endl;

				fileOurWriter << entry->second.getNumTuples() << endl;
				fileOurWriter << entry->second.getAttributeMapSize() << endl;

				map<string, int> attributeMap=entry->second.getAttributes();
				for(auto localEntry = attributeMap.begin(); localEntry != attributeMap.end(); localEntry++)
				{
					fileOurWriter << localEntry->first << endl << localEntry->second << endl;
				}
			}
		}

		fileOurWriter << "RelationUnionInfo Start --" << endl;

		// writing the relationsUnion size, estimate value, number of relations present and relation names
		if(relationsInfoVector.size()>0){
			fileOurWriter<< relationsInfoVector.size()<< endl;

			for(auto entry= relationsInfoVector.begin(); entry!=relationsInfoVector.end(); entry++){

				fileOurWriter << entry->getNumTuples() << endl;
				fileOurWriter << entry->getRelationsSetSize() << endl;

				set<string> relationsInfo=entry->getRelationsSet();

				for(auto localEntry = relationsInfo.begin(); localEntry != relationsInfo.end(); localEntry++)
				{
					fileOurWriter << *localEntry << endl;
				}
			}
		}
	}
	fileOurWriter.close();
}

// This used the statistics formed and performs the join operation using the predicates from the parseTree on all the relation names given
void  Statistics::Apply(struct AndList *parseTree, char *relNames[], int numToJoin)
{
	// To store the final relations union formed from the relations that joined from the relNames
	RelationsUnionInfo relationsInfo;

	for(int i = 0; i < numToJoin; i++)
	{
		relationsInfo.insertToRelationsSet(relNames[i]);
	}

	double psrsedEstimate = getParseTreeEstimation(parseTree);
	psrsedEstimate=psrsedEstimate>0.0?psrsedEstimate:1.0;

	// To store the index of the relation names that satisfices the required criters as per compareRelationsInfo method
	vector<int> resultIndexes;
	double resultEstimate = compareRelationsInfo(relationsInfo, resultIndexes, parseTree, psrsedEstimate);

	vector<RelationsUnionInfo> temprelationsInfoVector;
	if(resultIndexes.size() > 1)
	{
		// To get the remaining relation that are not part of join
		int sizeBeforeClear = relationsInfoVector.size();
		int pos = 0;
		for(int i=0; i < sizeBeforeClear; i++)
		{
			if(i != resultIndexes[pos]) temprelationsInfoVector.push_back(relationsInfoVector[i]);
			else pos++;
		}

		relationsInfoVector.clear();
		int newSize = sizeBeforeClear - (int) resultIndexes.size() + 1;
		for(int i=0; i<newSize -1; i++)
		{
			relationsInfoVector.push_back(temprelationsInfoVector[i]);
		}

		// Saving the final tuples estimate generated for the given set of relation name as per the parse tree
		relationsInfo.setNumTuples(resultEstimate);
		relationsInfoVector.push_back(relationsInfo);

		temprelationsInfoVector.clear();
		set<string> tempRelationsSet=relationsInfo.getRelationsSet();
		for(auto entry = tempRelationsSet.begin(); entry != tempRelationsSet.end(); entry++)
		{
			relationsInfoMap[*entry] = relationsInfo;
		}

	}
}

// Method to caliculate the total number of tuples that results from the join on the relation from the relNames without changing the statistics object
double Statistics::Estimate(struct AndList *parseTree, char **relNames, int numToJoin)
{
	RelationsUnionInfo relationsInfo;

	for(int i=0; i < numToJoin; i++)
	{
		relationsInfo.insertToRelationsSet(relNames[i]);
	}

	double parsedEstimate = getParseTreeEstimation(parseTree);
	parsedEstimate = parsedEstimate>0?parsedEstimate:1.0;
	vector<int> indexesVector;

	return compareRelationsInfo(relationsInfo, indexesVector, parseTree, parsedEstimate);
}

double Statistics::getParseTreeEstimation(struct AndList *parseTree)
{
	double estimate = 0.0;
	// parsing the parseTree if the tree is not NULL
	if(parseTree!=NULL)
	{
		struct AndList *treeAndList = parseTree;

		while(treeAndList!=NULL)
		{
			struct OrList *orList;
			orList = treeAndList->left;

			while(orList!=NULL)
			{
				struct ComparisonOp *compOperation;
				compOperation = orList->left;

				if(compOperation!=NULL)
				{
					// Chekcing for the posibulity of join operation when the operation is not NULL and is a EQUALS operation
					if(compOperation->code == EQUALS)
					{
						if(compOperation->left->code == compOperation->right->code){
							// Generating the relations and there respective attributes that are joined
							// The left relation and its attribute name
							string leftRelation, leftRelationAttribute;
							string value1(compOperation->left->value);
							int _pos=value1.find("_");
							leftRelation=tablesMap[value1.substr(0, _pos)];
							int dotPos=value1.find(".");
							if(dotPos!=string::npos){
								leftRelationAttribute=value1.substr(dotPos+1);
							}
							else{
								leftRelationAttribute=value1;
							}

							// The right relation and its attribute name
							string rightRelation, rightRelationAttribute;
							string value2(compOperation->right->value);
							_pos=value2.find("_");
							rightRelation=tablesMap[value2.substr(0, _pos)];
							dotPos=value2.find(".");
							if(dotPos!=string::npos){
								rightRelationAttribute=value2.substr(dotPos+1);
							}
							else{
								rightRelationAttribute=value2;
							}

							// Creating the left and right relation info objects
							RelationInfo leftRelObj, rightRelObj;
							leftRelObj = relationMap[leftRelation];
							rightRelObj = relationMap[rightRelation];

							// Computing the tuples generated by this join
							estimate = relationsInfoMap[leftRelation].getNumTuples() * relationsInfoMap[rightRelation].getNumTuples();
							double denominator = leftRelObj.getNumTuples() > rightRelObj.getNumTuples()?leftRelObj.getAttribute(leftRelationAttribute):rightRelObj.getAttribute(rightRelationAttribute);
							
							estimate=estimate/(double) denominator;

							return estimate;
						}
						else{

						}
					}
				}
				if(orList->rightOr==NULL) break;
				orList = orList->rightOr;
			}
			if(treeAndList->rightAnd==NULL) break;
			treeAndList = treeAndList -> rightAnd;
		}
	}

	// Return 0 if there is no join present
	return 0.0;
}

double Statistics::compareRelationsInfo(RelationsUnionInfo relationsInfo, vector<int> &resultIndexes, struct AndList *parseTree, double psrsedEstimate)
{
	int resultCount = 0;
	for(int i = 0; i < relationsInfoVector.size()&&resultCount!=relationsInfo.getRelationsSetSize(); i++)
	{
		// Fetching the relationsUnion sets from staticis relationsUnionVector and comparing each set with set of relation passed as parameter
		set<string> existingSet=relationsInfoVector[i].getRelationsSet();
		set<string> currentSet=relationsInfo.getRelationsSet();
		int commonCount=0;
		for(auto entry=currentSet.begin(); entry!=currentSet.end(); entry++){
			if(existingSet.find(*entry)!=existingSet.end()){
				commonCount++;
			}
		}

		// For the sets when join is not poosible, the common relation count should eight be zero or it is entire set 
		if(commonCount!=0&&commonCount!=existingSet.size()) break;

		else
		{
			resultIndexes.push_back(i);
			resultCount += commonCount;
		}
	}
	if(resultCount<=0){
		resultIndexes.clear();
		return 0.0;
	}	
	return compareEstimate(parseTree, psrsedEstimate);
}


double Statistics::compareEstimate(struct AndList *parseTree, double psrsedEstimate)
{
	double resultEstimate = psrsedEstimate;

	struct AndList *treeAndList = parseTree;
	bool flag = true;

	double defaultDivisor = 3.0l;
	double tuplesCount = 0.0l;
	while(treeAndList!=NULL)
	{
		struct OrList *orList=treeAndList->left;

		struct OrList *tempcurrOr = orList;
		string firstValue;
		bool firstOr=false;
		int counter=1;
		if(tempcurrOr!=NULL){
			firstValue=tempcurrOr->left->left->value;
			tempcurrOr=tempcurrOr->rightOr;
			// Seting the firstOr flag to true id the very first rightOr is NULL
			if(tempcurrOr==NULL) firstOr=true;
		}
		while(tempcurrOr){
			if(firstValue.compare(tempcurrOr->left->left->value)!=0) counter++;
			tempcurrOr=tempcurrOr->rightOr;
		}

		double localEstimate = counter>1?1.0:0.0;

		while(orList!=NULL)
		{
			struct ComparisonOp *compOperation =  orList->left;

			// Picking the operand with name code from left and right of the current compOperation
			Operand *operandObj = compOperation->left;
			if(operandObj->code != NAME)
				operandObj = compOperation->right;

			// Creating the relation info object for the name operand
			string relationName, relationAttribute;
			string value1(operandObj->value);
			int _pos=value1.find("_");
			relationName=tablesMap[value1.substr(0, _pos)];
			int dotPos=value1.find(".");
			if(dotPos!=string::npos){
				relationAttribute=value1.substr(dotPos+1);
			}
			else{
				relationAttribute=value1;
			}

			RelationInfo currentRelation = relationMap[relationName];
			if(compOperation->code == EQUALS)
			{
				// If the operation is EQUALS and both the left and right oprands are name it forms a join
				if(firstOr&&compOperation->right->code == NAME && compOperation->left->code == NAME)
				{
					flag = false;
					localEstimate = 1.0;
				}
				else if(counter>1){
					double const temp = 1.0l - (1.0l /currentRelation.getAttribute(relationAttribute));
					localEstimate *= temp;
				}
				else{
					double const temp = (1.0l / currentRelation.getAttribute(relationAttribute));
					localEstimate += temp;
				}
			}
			else
			{
				
				if(counter>1){
					double const temp = 1.0l - (1.0l / defaultDivisor);
					localEstimate *= temp;	
				}

				else{
					double const temp = 1.0l / defaultDivisor;
					localEstimate += temp;	
				}
				
			}

			orList = orList->rightOr;
			if(flag) tuplesCount = relationsInfoMap[relationName].getNumTuples();
		}

		treeAndList = treeAndList->rightAnd;
		if(counter>1){
			resultEstimate *= (1 - localEstimate);
		}
		else{
			resultEstimate *= localEstimate;
		}
	}

	if(!flag) return resultEstimate;

	resultEstimate = tuplesCount * resultEstimate;
	return resultEstimate;
}