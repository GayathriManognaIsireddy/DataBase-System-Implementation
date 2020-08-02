
CC = g++ -O2 -Wno-deprecated

tag = -i

ifdef linux
tag = -n
endif

test.out: Record.o Comparison.o ComparisonEngine.o Schema.o File.o BigQ.o DBFile.o Pipe.o y.tab.o lex.yy.o test.o DBClassifier.o HeapDBFile.o
	$(CC) -o test.out Record.o Comparison.o ComparisonEngine.o Schema.o File.o BigQ.o DBFile.o Pipe.o y.tab.o lex.yy.o test.o DBClassifier.o HeapDBFile.o -lfl -lpthread

a1test.out: Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o Pipe.o y.tab.o lex.yy.o a1test.o DBClassifier.o HeapDBFile.o
	$(CC) -o a1test.out Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o Pipe.o y.tab.o lex.yy.o a1test.o DBClassifier.o HeapDBFile.o -lfl

main: Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o y.tab.o lex.yy.o main.o DBClassifier.o HeapDBFile.o
	$(CC) -o main Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o y.tab.o lex.yy.o main.o DBClassifier.o HeapDBFile.o -lfl

gtest_BigQ.out: gtest_BigQ.o Record.o Comparison.o ComparisonEngine.o Schema.o File.o HeapDBFile.o DBClassifier.o DBFile.o BigQ.o Pipe.o y.tab.o lex.yy.o
	$(CC) -o gtest_BigQ.out gtest_BigQ.o Record.o Comparison.o ComparisonEngine.o Schema.o File.o HeapDBFile.o DBClassifier.o BigQ.o DBFile.o Pipe.o y.tab.o lex.yy.o -lfl -lgtest -lpthread

gtest_HeapDBFile.out: gtest_HeapDBFile.o Record.o Comparison.o ComparisonEngine.o Schema.o File.o HeapDBFile.o DBClassifier.o DBFile.o y.tab.o lex.yy.o
	$(CC) -o gtest_HeapDBFile.out gtest_HeapDBFile.o Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBClassifier.o HeapDBFile.o DBFile.o y.tab.o lex.yy.o -lfl -lgtest -lpthread

gtest_HeapDBFile.o: gtest_HeapDBFile.cc
	$(CC) -g -c gtest_HeapDBFile.cc

gtest_BigQ.o: gtest_BigQ.cc
	$(CC) -g -c gtest_BigQ.cc
	
test.o: test.cc
	$(CC) -g -c test.cc

a1test.o: a1test.cc
	$(CC) -g -c a1test.cc

Comparison.o: Comparison.cc
	$(CC) -g -c Comparison.cc
	
ComparisonEngine.o: ComparisonEngine.cc
	$(CC) -g -c ComparisonEngine.cc
	
Pipe.o: Pipe.cc
	$(CC) -g -c Pipe.cc

BigQ.o: BigQ.cc
	$(CC) -g -c BigQ.cc

DBFile.o: DBFile.cc
	$(CC) -g -c DBFile.cc

File.o: File.cc
	$(CC) -g -c File.cc

Record.o: Record.cc
	$(CC) -g -c Record.cc

Schema.o: Schema.cc
	$(CC) -g -c Schema.cc
	
y.tab.o: Parser.y
	yacc -d Parser.y
	sed $(tag) y.tab.c -e "s/  __attribute__ ((__unused__))$$/# ifndef __cplusplus\n  __attribute__ ((__unused__));\n# endif/" 
	g++ -c y.tab.c

lex.yy.o: Lexer.l
	lex  Lexer.l
	gcc  -c lex.yy.c

DBClassifier.o: DBClassifier.cc
	$(CC) -g -c DBClassifier.cc

HeapDBFile.o: HeapDBFile.cc
	$(CC) -g -c HeapDBFile.cc

clean: 
	rm -f *.o
	rm -f *.out
	rm -f y.tab.c
	rm -f lex.yy.c
	rm -f y.tab.h
