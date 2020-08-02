#ifndef DB_UTIL_H
#define DB_UTIL_H

enum outputType { PIPE_STDOUT, PIPE_NONE, PIPE_FILE };
enum queryType {QUERY_SELECT, QUERY_CREATE, QUERY_INSERT, QUERY_DROP, QUERY_SET, QUERY_QUIT };

#include <string>

using std::string;

struct InOutPipe {

	// this indicates if the output is standard output, nothing, or
	// a specific file
	int type;

    // this indicates the file to which the output should go.
    // it has dummy information if type is PIPE_STDOUT or
    // PIPE_NONE
	char *file;

    // this indicates from where the input should come from.
    // it has dummy information if type is PIPE_STDOUT or
    // PIPE_NONE
	char *src;

};

struct tableArgs {

	// Name of the table
	string name;

    //Database type
	string dbType;
};

#endif