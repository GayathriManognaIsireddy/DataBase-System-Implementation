#include <stdio.h>
#include "DbUtil.h"
#include "DBFile.h"
#include <fstream>
#include <iostream>
#include <cstdlib>
#include "MainQueryPlaner.h"

using namespace std;

extern "C" {
    int yyparse(void);
}

extern struct FuncOperator *finalFunction;
extern struct TableList *tables;
extern struct AndList *boolean;
extern struct NameList *groupingAtts;
extern struct NameList *attsToSelect;
extern struct NameList *sortAtts;
extern int distinctAtts;
extern int distinctFunc;

extern struct InOutPipe *io;

extern string createTableName;
extern string createTableType;
extern vector<string> atts;
extern vector<string> attTypes;
extern char *dbfile_dir;

extern int queryType;

int main()
{

//     bool flag=false;
//     int option;
//     bool started=false;
//    while(true)
//    {
//         cout<<"1: Start the Database"<<endl;
//         cout<<"2: Run a Query" <<endl;
//         cout << "3: Shutdown the Database"<<endl;;
//         cin>>option;

//         switch(option){
//             case 1: 
//                 {cout<<"-----------Stared the database---------------" << endl;
//                 started=true;}
//                 break;
//             case 2:
//                 {
                cout << "Please enter a query and press Ctrl+D to execute" << endl;
                yyparse();
                QueryHandler *queryHandlerObj = new QueryHandler(finalFunction, tables, boolean, groupingAtts, attsToSelect, sortAtts, distinctAtts, distinctFunc, 
                                                                io, createTableName, createTableType, atts, attTypes, queryType);
                queryHandlerObj->StartQuery();
                //}
        //         break;
        //     case 3:
        //         if (!started){cout<<"-----------Start the datbase before SHutting it down-----------"<<endl;}
        //         else{cout<< "-----------Shutting Down the database-----------"<<endl;
        //         exit(0);}
        //         break;
        // }
    //}

    return 0;

}