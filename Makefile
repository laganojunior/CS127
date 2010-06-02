CFLAGS = -O2 -pipe

all: test test2 autogenmat

test: test.o LDPC.o LDPC.h Matrix.h
	g++ ${CFLAGS} -o test test.o LDPC.o

autogenmat: autogenmat.o GenParityCheck.o Matrix.h
	g++ ${CFLAGS} -o autogenmat autogenmat.o GenParityCheck.o

clean:
	rm *.o
	rm test

autogenmat.o: autogenmat.cpp GenParityCheck.h Matrix.h
	g++ ${CFLAGS} -c  -o autogenmat.o autogenmat.cpp

GenParityCheck.o: GenParityCheck.cpp GenParityCheck.h Matrix.h
	g++ ${CFLAGS} -c -o GenParityCheck.o GenParityCheck.cpp

LDPC.o: LDPC.cpp LDPC.h Matrix.h
	g++ ${CFLAGS} -c -o LDPC.o LDPC.cpp

test.o: test.cpp LDPC.h Matrix.h WeightDist.h
	g++ ${CFLAGS} -c -o test.o test.cpp
