all: test autogenmat

test: test.o LDPC.o LDPC.h Matrix.h
	g++ -o test test.o LDPC.o
	
autogenmat: autogenmat.o GenParityCheck.o Matrix.h
	g++ -o autogenmat autogenmat.o GenParityCheck.o

clean:
	rm *.o
	rm test

autogenmat.o: autogenmat.cpp GenParityCheck.h Matrix.h
	g++ -c  -o autogenmat.o autogenmat.cpp

GenParityCheck.o: GenParityCheck.cpp GenParityCheck.h Matrix.h
	g++ -c -o GenParityCheck.o GenParityCheck.cpp

LDPC.o: LDPC.cpp LDPC.h Matrix.h
	g++ -c -o LDPC.o LDPC.cpp
