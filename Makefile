CC = g++ -O2 -Wno-deprecated 

tag = -i

ifdef linux
tag = -n
endif

a4-1.out: Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o Statistics.o y.tab.o yyfunc.tab.o lex.yy.o lex.yyfunc.o test.o DBClassifier.o HeapDBFile.o SortedDBFile.o BigQ.o Pipe.o RelOp.o Function.o
	$(CC) -o a4-1.out Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o Statistics.o y.tab.o yyfunc.tab.o lex.yy.o lex.yyfunc.o test.o DBClassifier.o HeapDBFile.o SortedDBFile.o BigQ.o Pipe.o RelOp.o Function.o -lfl -lpthread

a3test.out: Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o Pipe.o BigQ.o RelOp.o Function.o y.tab.o yyfunc.tab.o lex.yy.o lex.yyfunc.o a3test.o DBClassifier.o HeapDBFile.o SortedDBFile.o
	$(CC) -o a3test.out Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o Pipe.o BigQ.o RelOp.o Function.o y.tab.o yyfunc.tab.o lex.yy.o lex.yyfunc.o a3test.o DBClassifier.o HeapDBFile.o SortedDBFile.o -lfl -lpthread
	
a2test.out: Record.o Comparison.o ComparisonEngine.o Schema.o File.o BigQ.o DBFile.o Pipe.o y.tab.o lex.yy.o a2test.o DBClassifier.o HeapDBFile.o SortedDBFile.o
	$(CC) -o a2test.out Record.o Comparison.o ComparisonEngine.o Schema.o File.o BigQ.o DBFile.o Pipe.o y.tab.o lex.yy.o a2test.o DBClassifier.o HeapDBFile.o SortedDBFile.o -lfl -lpthread
	
a2BigQtest.out: Record.o Comparison.o ComparisonEngine.o Schema.o File.o BigQ.o DBFile.o Pipe.o y.tab.o lex.yy.o a2BigQtest.o DBClassifier.o HeapDBFile.o SortedDBFile.o
	$(CC) -o a2BigQtest.out Record.o Comparison.o ComparisonEngine.o Schema.o File.o BigQ.o DBFile.o Pipe.o y.tab.o lex.yy.o a2BigQtest.o DBClassifier.o HeapDBFile.o SortedDBFile.o -lfl -lpthread
	
a1test.out: Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o Pipe.o y.tab.o lex.yy.o a1test.o DBClassifier.o HeapDBFile.o SortedDBFile.o BigQ.o
	$(CC) -o a1test.out Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o Pipe.o y.tab.o lex.yy.o a1test.o DBClassifier.o HeapDBFile.o SortedDBFile.o BigQ.o -lfl -lpthread

gTesta4.out: gTesta4.o Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o Statistics.o y.tab.o lex.yy.o DBClassifier.o HeapDBFile.o SortedDBFile.o BigQ.o Pipe.o RelOp.o Function.o
	$(CC) -o gTesta4.out gTesta4.o Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o Statistics.o y.tab.o lex.yy.o DBClassifier.o HeapDBFile.o SortedDBFile.o BigQ.o Pipe.o RelOp.o Function.o -lfl -lgtest -lpthread

a42.out:   Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o Statistics.o DBClassifier.o HeapDBFile.o SortedDBFile.o BigQ.o Pipe.o RelOp.o Function.o mainQueryPlan.o y.tab.o lex.yy.o main.o
	$(CC) -o a42.out Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o Statistics.o DBClassifier.o HeapDBFile.o SortedDBFile.o BigQ.o Pipe.o RelOp.o Function.o mainQueryPlan.o y.tab.o lex.yy.o main.o -lfl -lpthread

main.out: Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o Statistics.o DBClassifier.o HeapDBFile.o SortedDBFile.o BigQ.o Pipe.o RelOp.o Function.o MainQueryPlaner.o y.tab.o yyfunc.tab.o lex.yy.o lex.yyfunc.o main.o
	$(CC) -o main.out Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o Statistics.o DBClassifier.o HeapDBFile.o SortedDBFile.o BigQ.o Pipe.o RelOp.o Function.o MainQueryPlaner.o y.tab.o yyfunc.tab.o lex.yy.o lex.yyfunc.o main.o -lfl -lpthread

a5.out: Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o Statistics.o DBClassifier.o HeapDBFile.o SortedDBFile.o BigQ.o Pipe.o RelOp.o Function.o MainQueryPlaner.o y.tab.o yyfunc.tab.o lex.yy.o lex.yyfunc.o main.o
	$(CC) -o a5.out Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o Statistics.o DBClassifier.o HeapDBFile.o SortedDBFile.o BigQ.o Pipe.o RelOp.o Function.o MainQueryPlaner.o y.tab.o yyfunc.tab.o lex.yy.o lex.yyfunc.o main.o -lfl -lpthread

gTest5.out: gTest5.o Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o Statistics.o DBClassifier.o HeapDBFile.o SortedDBFile.o BigQ.o Pipe.o RelOp.o Function.o MainQueryPlaner.o y.tab.o yyfunc.tab.o lex.yy.o lex.yyfunc.o
	$(CC) -o gTest5.out gTest5.o Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o Statistics.o DBClassifier.o HeapDBFile.o SortedDBFile.o BigQ.o Pipe.o RelOp.o Function.o MainQueryPlaner.o y.tab.o yyfunc.tab.o lex.yy.o lex.yyfunc.o -lfl -lgtest -lpthread

gTest42.out: gTest42.o Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o Statistics.o DBClassifier.o HeapDBFile.o SortedDBFile.o BigQ.o Pipe.o RelOp.o Function.o mainQueryPlan.o y.tab.o lex.yy.o
	$(CC) -o gTest42.out gTest42.o Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o Statistics.o DBClassifier.o HeapDBFile.o SortedDBFile.o BigQ.o Pipe.o RelOp.o Function.o mainQueryPlan.o y.tab.o lex.yy.o -lfl -lgtest -lpthread
	
main.o : main.cc
	$(CC) -g -c main.cc

test.o: test.cc
	$(CC) -g -c test.cc

gTesta4.o: gTesta4.cc
	$(CC) -g -c gTesta4.cc

gTest42.o: gTest42.cc
	$(CC) -g -c gTest42.cc

gTest5.o: gTest5.cc
	$(CC) -g -c gTest5.cc

a2test.o: a2test.cc
	$(CC) -g -c a2test.cc

a2BigQtest.o: a2BigQtest.cc
	$(CC) -g -c a2BigQtest.cc

a1test.o: a1test.cc
	$(CC) -g -c a1test.cc

Pipe.o: Pipe.cc
	$(CC) -g -c Pipe.cc

BigQ.o: BigQ.cc
	$(CC) -g -c BigQ.cc

RelOp.o: RelOp.cc
	$(CC) -g -c RelOp.cc

Function.o: Function.cc
	$(CC) -g -c Function.cc

Statistics.o: Statistics.cc
	$(CC) -g -c Statistics.cc

Comparison.o: Comparison.cc
	$(CC) -g -c Comparison.cc
	
ComparisonEngine.o: ComparisonEngine.cc
	$(CC) -g -c ComparisonEngine.cc
	
DBFile.o: DBFile.cc
	$(CC) -g -c DBFile.cc

File.o: File.cc
	$(CC) -g -c File.cc

Record.o: Record.cc
	$(CC) -g -c Record.cc

Schema.o: Schema.cc
	$(CC) -g -c Schema.cc

DBClassifier.o: DBClassifier.cc
	$(CC) -g -c DBClassifier.cc

HeapDBFile.o: HeapDBFile.cc
	$(CC) -g -c HeapDBFile.cc

SortedDBFile.o: SortedDBFile.cc
	$(CC) -g -c SortedDBFile.cc

mainQueryPlan.o: mainQueryPlan.cc
	$(CC) -g -c mainQueryPlan.cc

MainQueryPlaner.o: MainQueryPlaner.cc
	$(CC) -g -c MainQueryPlaner.cc
	
y.tab.o: Parser.y
	yacc -d Parser.y
	sed $(tag) y.tab.c -e "s/  __attribute__ ((__unused__))$$/# ifndef __cplusplus\n  __attribute__ ((__unused__));\n# endif/" 
	g++ -c y.tab.c

yyfunc.tab.o: ParserFunc.y
	yacc -p "yyfunc" -b "yyfunc" -d ParserFunc.y
	sed $(tag) yyfunc.tab.c -e "s/  __attribute__ ((__unused__))$$/# ifndef __cplusplus\n  __attribute__ ((__unused__));\n# endif/" 
	g++ -c yyfunc.tab.c

lex.yy.o: Lexer.l
	lex  Lexer.l
	gcc  -c lex.yy.c

lex.yyfunc.o: LexerFunc.l
	lex -Pyyfunc LexerFunc.l
	gcc  -c lex.yyfunc.c

clean: 
	rm -f *.o
	rm -f *.out
	rm -f y.tab.*
	rm -f yyfunc.tab.*
	rm -f lex.yy.*
	rm -f lex.yyfunc*
