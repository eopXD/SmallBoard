G_PLUS_PLUS=g++ -std=c++17 -O2 -Wall -pthread
STATE_ULL=-D BITSTATE_ULL
STATE_BITSET=-D BITSTATE_BST 

PARALLEL=-fopenmp

2x2=-D ROW=2 -D COL=2 -D ENCODE_LENGTH=19
3x3=-D ROW=3 -D COL=3 -D ENCODE_LENGTH=30
4x4=-D ROW=4 -D COL=4 -D ENCODE_LENGTH=44
5x5=-D ROW=5 -D COL=5 -D ENCODE_LENGTH=58 

INCLUDE=-I ../../include/ 
NOW=$(4x4)

interface:
	$(G_PLUS_PLUS) $(STATE_ULL) $(NOW) $(INCLUDE) -c ../../include/comm.cc
	$(G_PLUS_PLUS) $(STATE_ULL) -D DISPLAY_TERMINATE $(NOW) $(INCLUDE) comm.o -c ../../include/GoBoard.cc
	$(G_PLUS_PLUS) $(STATE_ULL) $(NOW) $(INCLUDE) *.o interface.cpp -o exec.interface


main:
	mkdir -p data
	$(G_PLUS_PLUS) $(STATE_ULL) $(NOW) $(INCLUDE) -c ../../include/comm.cc
	$(G_PLUS_PLUS) $(STATE_ULL) $(NOW) $(INCLUDE) comm.o -c ../../include/GoBoard.cc
	$(G_PLUS_PLUS) $(STATE_ULL) $(NOW) $(INCLUDE) *.o main.cpp -o exec.main	
	
clean: 
	rm -rf *.o
	rm -rf data/data.*
	rm -rf exec.*
