#include "RelOp.h"

void SelectFile::Run (DBFile &inFile, Pipe &outPipe, CNF &selOp, Record &literal) {
	cout<< &inFile<< endl;
	this->inFile = &inFile;
  	this->outPipe = &outPipe;
  	this->selOp = &selOp;
  	this->literal = &literal;

  	int num = pthread_create(&workerThread, NULL, runHelper, (void*) this);
}

void SelectFile::WaitUntilDone () { 
	pthread_join (workerThread, NULL);
}

void SelectFile::Use_n_Pages (int runlen) {

}

void * SelectFile::runHelper(void *op)
{
    SelectFile *selectFile = static_cast<SelectFile*>(op);
    selectFile->selectFileFunction();
	selectFile->outPipe->ShutDown();

    pthread_exit(NULL);
}

//Record from file to out pipe
void SelectFile::selectFileFunction()
{
	int count = 0;
	//Schema sch = Schema("catalog", "nation");
	Record *record = new Record;
	if(!selOp){
		//cout<< "In not selOp ============"<< endl;
		while(inFile->GetNext(*record)){
			count++;
			Record *toPipe = new Record;
			toPipe->Consume(record);
			//toPipe->Print(&sch);
      		outPipe->Insert(toPipe);
  		}
	}
	else{
		//cout<< "In selOp ============"<< endl;
		while(inFile->GetNext(*record, *selOp, *literal)){
			count++;
			Record *toPipe = new Record;
			toPipe->Consume(record);
      		outPipe->Insert(toPipe);
  		}
	}

	//cout << " THe count of the records is " << count << endl;
	
	// Record *temp;
	// while(outPipe->Remove(temp)){
	// 	temp->Print(&sch);
	// }
}



void SelectPipe::Run (Pipe &inPipe, Pipe &outPipe, CNF &selOp, Record &literal) {
	this->inPipe = &inPipe;
  	this->outPipe = &outPipe;
  	this->selOp = &selOp;
  	this->literal = &literal;

  	int num = pthread_create(&workerThread, NULL, runHelper, (void*) this);
}

void SelectPipe::WaitUntilDone () { 
	pthread_join (workerThread, NULL);
}

void SelectPipe::Use_n_Pages (int runlen) {

}

void * SelectPipe::runHelper(void *op)
{
    SelectPipe *selectPipe = static_cast<SelectPipe*>(op);
    selectPipe->selectPipeFunction();
	selectPipe->outPipe->ShutDown();

    pthread_exit(NULL);
}


// Record from inpipe to outpipe
void SelectPipe::selectPipeFunction()
{
	Record *record = new Record;
	ComparisonEngine comparObj;
	int status = inPipe->Remove(record);
	while (status==1)
	{
		if (comparObj.Compare(record, literal, selOp))
		{
			outPipe->Insert(record);
		}
		status = inPipe->Remove(record);
	}
}



void Project::Run(Pipe &inPipe, Pipe &outPipe, int *keepMe, int numAttsInput, int numAttsOutput) {

	this->inPipe = &inPipe;
	this->outPipe = &outPipe;
	this->keepMe = keepMe;
	this->numAttsInput = numAttsInput;
	this->numAttsOutput = numAttsOutput;

	pthread_create(&workerThread, NULL, runHelper, (void*)this);
}

void Project::WaitUntilDone () { 
	cout << "Waiting for PROJECT to complete" << endl;
	pthread_join (workerThread, NULL);
}

void Project::Use_n_Pages (int runlen) {

}

void * Project::runHelper(void *op)
{
	Project *project = static_cast<Project*>(op);
	project->projectFunction();
	project->outPipe->ShutDown();

	pthread_exit(NULL);
}

void Project::projectFunction()
{
	Record *record = new Record;
	int status=inPipe->Remove(record);
	while(status==1)
	{
		record->Project(keepMe, numAttsOutput, numAttsInput);
		outPipe->Insert(record);
		status = inPipe->Remove(record);
	}

	return;
}



void Join::Run (Pipe &inPipeL, Pipe &inPipeR, Pipe &outPipe, CNF &selOp, Record &literal)
{
    this->inPipeL = &inPipeL;
    this->inPipeR = &inPipeR;
    this->outPipe = &outPipe;
    this->selOp = &selOp;
    this->literal = &literal;

    pthread_create(&workerThread, NULL, runHelper, (void *) this);
}

void Join::WaitUntilDone ()
{
	cout << "Waiting for JOIN to complete " << endl;
    pthread_join(workerThread, NULL);
}

void Join::Use_n_Pages (int n)
{
    numOfPages = n;
}

void* Join::runHelper(void *op)
{
    Join *join = static_cast<Join*>(op);
    join->joinFunction();
	join->outPipe->ShutDown();

    pthread_exit(NULL);
}

void Join::joinFunction()
{
    OrderMaker *leftOM = new OrderMaker();
    OrderMaker *rightOM = new OrderMaker();

	selOp->GetSortOrders(*leftOM, *rightOM) != 0? joinBySort(leftOM, rightOM) : defaultJoin();

    delete leftOM;
    delete rightOM;

	return;
}

void Join::joinBySort(OrderMaker *leftOM, OrderMaker *rightOM)
{
    //BigQ constructor with require parameters to fetch the records to left and right pipe
    Pipe leftPipe(1000);
	//cout << "Sorting left pipe in joint@@@@@@@@@@@@@@@@@@@@@@@@@@@" << endl;
	BigQ bigL(*inPipeL, leftPipe, *leftOM, numOfPages);
    Pipe rightPipe(1000);
	//cout << "Sorting right pipe in join@@@@@@@@@@@@@@@@@@@@@@@@@@" << endl;
    BigQ bigR(*inPipeR, rightPipe, *rightOM, numOfPages);

    // pipe specific flags and record holders
	Record *leftRecord = new Record;
    Record *rightRecord = new Record;
    bool leftFlag = (0 == leftPipe.Remove(leftRecord));
    bool rightFlag = (0 == rightPipe.Remove(rightRecord));
	//cout<< leftFlag<<" and "<< rightFlag<<endl;

	//cout<< "left att count: "<< left->GetAttributeCount()<< "  Right att count: "<< right->GetAttributeCount()<< endl;

	//cout<< "IN join sortmerge join"<< endl;
    ComparisonEngine comparObj;
    while(!leftFlag && !rightFlag)
    {
        int status = comparObj.Compare(leftRecord, leftOM, rightRecord, rightOM);
		if(status==0){
			vector<Record*> leftRecordList;
			leftFlag = sortedRecordList(leftPipe, leftRecord, leftRecordList, leftOM);

    		vector<Record*> rightRecordList;
            rightFlag = sortedRecordList(rightPipe, rightRecord, rightRecordList, rightOM);

            for(int i=0; i < leftRecordList.size(); i++)
            {
                for(int j=0; j<rightRecordList.size(); j++)
                {
                   insertMergeRecordToPipe(leftRecordList[i], rightRecordList[j]);
                }
            }
		}
		else{
			if(status<0) leftFlag = (0 == leftPipe.Remove(leftRecord));
			else rightFlag = (0 == rightPipe.Remove(rightRecord));
		}
        
    }

    Record record;
    if(leftFlag) while(leftPipe.Remove(&record));
    if(rightFlag) while(rightPipe.Remove(&record));

	//cout << "Records join completed " << endl;
}

bool Join::sortedRecordList(Pipe &pipe, Record *rec, vector<Record*> &vecOfRecs, OrderMaker *sortOrder)
{
	//cout<< "IN join getCommonRec"<< endl;
	Record *localRecord = new Record;
	localRecord->Consume(rec);
	vecOfRecs.push_back(localRecord);
	//vecOfRecs[vecOfRecs.size()-1]->Print(new Schema("catalog", "supplier"));

	// If pipe is empty return ture
	if(pipe.Remove(rec) == 0) return true;

	ComparisonEngine comparObj;
    while(comparObj.Compare(vecOfRecs[0], rec, sortOrder) == 0)
    {
		Record *localRecord = new Record();
		localRecord->Consume(rec);
		vecOfRecs.push_back(localRecord);

		// pipe is empty
		if(pipe.Remove(rec) == 0) return true;  
    }

	// compare fail but pipe is not empty
    return false;
}

int  GetNumAtts (char* bits) { 

	int numAtts = 0;

	if (bits != NULL){
		char *ptr = bits + sizeof(int);
		numAtts = *((int *) ptr) / sizeof(int) - 1;
	}
	return numAtts;
}

void Join::insertMergeRecordToPipe(Record *left, Record *right)
{
    int leftAttributeCount = GetNumAtts(left->GetBits());
    int rightAttributeCount = GetNumAtts(right->GetBits());
	// cout<< " In Creat fun Left: "<< numAttsLeft<< "   Right: "<<numAttsRight<< endl;
    int totalNumOfAttributes = leftAttributeCount + rightAttributeCount;

    int * resultAttributes = new int[totalNumOfAttributes];
	for(int i=0; i<totalNumOfAttributes; i++){
		if(i<leftAttributeCount) resultAttributes[i]=i;
		else resultAttributes[i]=i-leftAttributeCount;
	}

	Record *record = new Record;
    record->MergeRecords(left, right, leftAttributeCount, rightAttributeCount,
	    	       resultAttributes, totalNumOfAttributes, leftAttributeCount);

    outPipe->Insert(record);
}


void Join::defaultJoin()
{
    DBFile leftFile;
	char *leftPath = "tempLeft.bin";

	DBFile rightFile;
	char *rightPath = "tempRight.bin";

	inPipeToFile(leftFile, leftPath, true);
	inPipeToFile(rightFile, rightPath, false);
	cout<< "In block next"<< endl;

	ComparisonEngine comparObj;
    Record recFromLeftFile;
    while(leftFile.GetNext(recFromLeftFile))
    {
		Record recFromRightFile;
        while(rightFile.GetNext(recFromRightFile))
        {
            if(!comparObj.Compare(&recFromLeftFile, &recFromRightFile, literal, selOp))
            {
                insertMergeRecordToPipe(&recFromLeftFile, &recFromRightFile);
            }
        }

        rightFile.MoveFirst();
    }

    remove(leftPath);
	remove(rightPath);
    return;

}

void Join::inPipeToFile(DBFile file, char* path, bool flag){
	// heap file for sorting the records from pipe
    file.Create(path, heap, NULL);

    Record *recToFile = new Record;
	if(flag){
		while(inPipeL->Remove(recToFile))
    	{
        	file.Add(*recToFile);
    	}
	}
	else{
		while(inPipeR->Remove(recToFile))
    	{
        	file.Add(*recToFile);
    	}
	}
}




void DuplicateRemoval::Run (Pipe &inPipe, Pipe &outPipe, Schema &mySchema)
{
	this->inPipe = &inPipe;
	this->outPipe = &outPipe;
	this->mySchema = &mySchema;
	myOrder = new OrderMaker(this->mySchema);

	pthread_create(&workerThread, NULL, runHelper, (void*) this);
}

void DuplicateRemoval::WaitUntilDone () { 
	pthread_join (workerThread, NULL);
}

void DuplicateRemoval::Use_n_Pages (int runlen) {
	this->runLength=runlen;
}

void * DuplicateRemoval::runHelper(void *op)
{
	DuplicateRemoval *duplicateRem = static_cast<DuplicateRemoval*>(op);
	duplicateRem->duplicateFunction();
	duplicateRem->outPipe->ShutDown();
	
	pthread_exit(NULL);
}

void DuplicateRemoval::duplicateFunction()
{
	//BigQ constructor with require parameters to fetch the records in sorted pipe
	Pipe sortedRecords(100);
	BigQ big(*inPipe, sortedRecords, *myOrder, runLength);

	Record *currentRecord = new Record();
	Record *lastRecord = new Record();
	bool flag=true;
	ComparisonEngine compareObj;
	while (sortedRecords.Remove(currentRecord))
	{
		if (flag || compareObj.Compare(lastRecord, currentRecord, myOrder) != 0)
		{
			Record *RecordtoInsert = new Record();
			RecordtoInsert->Copy(currentRecord);
			lastRecord->Consume(currentRecord);
			outPipe->Insert(RecordtoInsert);
			flag=false;
		}
	}
}



void Sum::Run (Pipe &inPipe, Pipe &outPipe, Function &computeMe){
	this->inPipe = &inPipe;
  	this->outPipe = &outPipe;
  	this->computeMe = &computeMe;

  	pthread_create(&workerThread, NULL, runHelper, (void*) this);
}

void Sum::WaitUntilDone () {
  	pthread_join (workerThread, NULL);
}

void Sum::Use_n_Pages (int runlen) {

}

void* Sum::runHelper (void *op){
  	Sum *sum = static_cast<Sum*>(op);
  	sum->sumFunction();
	sum->outPipe->ShutDown();

  	pthread_exit(NULL);
}

void Sum::sumFunction()
{
    Record pipeRecord;
	Type type ;

    int intTypeSum = 0;
    double doubleTypeSum = 0.0;
	// Computing the sum operation on all the records from the inpipe
    while(inPipe->Remove(&pipeRecord))
    {
		type = computeMe->Apply(pipeRecord, intTypeSum, doubleTypeSum);
    }
    Attribute *attribute = new Attribute;
    attribute->name = "sum_sch";
    attribute->myType = type;
    char *fileName = "schemaFile";
    Schema schemaObj(fileName, 1, attribute);

    //Record in string form
    string str;
	if (type == DOUBLE)
		str = std::to_string(doubleTypeSum)+"|";
	else
		str = std::to_string(intTypeSum)+"|";

	//cout<< str<< " -------------"<< endl;
	Record *record = new Record;
    record->ComposeRecord(&schemaObj, str.c_str());
    outPipe->Insert(record);
}



void GroupBy::Run(Pipe &inPipe, Pipe &outPipe, OrderMaker &groupAtts, Function &computeMe) {
	this->inPipe = &inPipe;
	this->outPipe = &outPipe;
	this->groupAtts = &groupAtts;
	this->computeMe = &computeMe;

	pthread_create(&workerThread, NULL, runHelper, (void*)this);
}

void GroupBy::WaitUntilDone() {
	pthread_join(workerThread, NULL);
}

void GroupBy::Use_n_Pages (int n) {
	this->numOfPages=n;
}

void * GroupBy::runHelper(void *op)
{
	GroupBy *groupBy = static_cast<GroupBy*>(op);
	groupBy->groupByFunction();
	groupBy->outPipe->ShutDown();

	pthread_exit(NULL);
}

void GroupBy::groupByFunction()
{
	//BigQ constructor with require parameters to fetch the records in sorted pipe
	Pipe sortedRecords(100);
	BigQ big(*inPipe, sortedRecords, *groupAtts, numOfPages);

	int attributeCount = groupAtts->GetAttributeCount();
	Type type;
	int attributeList[attributeCount+1];
	Type attributeTypeList[attributeCount];

	groupAtts->GetAttributes(attributeList, attributeTypeList);
	for (int i = attributeCount; i>0; i--)
	{
		attributeList[i] = attributeList[i-1];
	}
	attributeList[0]=0;

	int intTypeSum = 0;
	double doubleTypeSum = 0.0;

	Record *currentRecord = new Record;
	Record *lastRecord = NULL;

	ComparisonEngine comparObj;
	while (sortedRecords.Remove(currentRecord))
	{
		bool flag=false;
		if (lastRecord == NULL || comparObj.Compare(lastRecord,currentRecord,groupAtts) != 0)
		{
			if (lastRecord != NULL)
			{
				insertRecordToPipe(lastRecord, attributeCount, attributeList, intTypeSum, doubleTypeSum, type);

				intTypeSum = 0;
				doubleTypeSum = 0;
			}
			else
			{
				lastRecord = new Record;
			}
			flag=true;
		}

		type = computeMe->Apply(*currentRecord, intTypeSum, doubleTypeSum);
		lastRecord->Consume(currentRecord);
	}
	if(lastRecord != NULL)
		insertRecordToPipe(lastRecord, attributeCount, attributeList, intTypeSum, doubleTypeSum, type);
}

// Insert the lastrecord fetched into the outpipe
void GroupBy::insertRecordToPipe(Record *lastRecord,int numOfAttsOld,int *attListOld, int intSum, double doubleSum, Type type) {
	Attribute *attribute = new Attribute;
	attribute->name = "sum_sch";
	attribute->myType = type;
	char *fileName = "schemaFile";
	Schema schemaObj(fileName, 1, attribute);
	string str;
	if (type == DOUBLE)
		str = std::to_string(doubleSum)+"|";
	else
		str = std::to_string(intSum)+"|";

	Record *holdRecord = new Record;
	holdRecord->ComposeRecord(&schemaObj, str.c_str());
	int listLen = sizeof(attListOld)/sizeof(attListOld[0]);

	Record *insertRecord = new Record();
	insertRecord->MergeRecords(holdRecord, lastRecord, 1, listLen-1, attListOld,  listLen, 1);
	outPipe->Insert(insertRecord);
}




void WriteOut::Run (Pipe &inPipe, FILE *outFile, Schema &mySchema) {

  this->inPipe = &inPipe;
  this->outFile = outFile;
  this->mySchema = &mySchema;

  pthread_create(&workerThread, NULL, runHelper, (void*) this);
}

void WriteOut::WaitUntilDone () {
  pthread_join (workerThread, NULL);
}

void WriteOut::Use_n_Pages (int runlen) {

}

void* WriteOut::runHelper (void *op){

  WriteOut *print = static_cast<WriteOut*>(op);
  print->writeOutFunction();

  pthread_exit(NULL);
}

// To write the records from inpipe to outfile
void WriteOut::writeOutFunction (){

    string str;
    Record *record = new Record;
	int status=inPipe->Remove(record);
    while(status==1&&!feof(outFile) && !ferror(outFile))
    {
		str="";
        record->Print(mySchema, str);
		status=inPipe->Remove(record);
		if(status==1){
			str.append("\n");
		}
		fputs(str.c_str(), outFile);
    }

    delete record;
}