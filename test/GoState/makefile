G_PLUS_PLUS=g++ -std=c++11 -O2 -Wall
ROW_COL=-D ROW=5 -D COL=5 
STATE_ULL=-D BITSTATE_ULL
STATE_BITSET=-D BITSTATE_BST 
5x5=-D ENCODE_LENGTH=58 
INCLUDE=-I ../../include/ 

all:
	$(G_PLUS_PLUS) $(ROW_COL) $(STATE_ULL) $(5x5) $(INCLUDE) test.cpp ../../include/comm.cc ../../include/GoState.cc -o exec.test
	