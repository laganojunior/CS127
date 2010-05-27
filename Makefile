test: test.o GenParityCheck.o
	g++ -o test test.o GenParityCheck.o

test.o: test.cpp GenParityCheck.h Matrix.h
	g++ -c  -o test.o test.cpp

GenParityCheck.o: GenParityCheck.cpp GenParityCheck.h Matrix.h
	g++ -c -o GenParityCheck.o GenParityCheck.cpp
