// Copyright (C) 2019 Yueh-Ting Chen (eopXD)
/*! \file FindAllPossibleState/main.cpp
	\brief Find all serial numbers that is legal. (no block on the board
	 shall have zero liberty.

	\author Yueh-Ting Chen (eopXD)
	\project Efficient & Succinct Small Board Go
*/

#include <cstdio>
#include <iostream>
#include <bitset>

#include "GoBoard.h"

using namespace GoConstant;
using namespace GoFunction;
using namespace std;

using BITSET = bitset<MemoryConstant::MEM_BLOCK_SIZE>;


// utility: write std::bitset into binary file
void write_bitset ( BITSET &bst, char *filename ) {
	FILE *fp;
	fp = fopen(filename, "wb");
	uint64_t tmp = bst[0];
	for ( int i=1; i<bst.size(); ++i ) {
		tmp = (tmp<<1) + bst[i];
		if ( (i&63) == 0 ) {
			fprintf(fp, "%llu", tmp);
		}
	}
	fclose(fp);
}

int main ()
{
	//char file_name[105];
	//BITSET legal_state;	

	const GoSerial a = 6981;
/* this is a legal board
EWB
EWB
EEB
*/
	GoBoard test_board(a);
	test_board.DisplayBoard();
	std::cout << "error code: " << (int) test_board.error_code << "\n";

	const GoSerial b = 11355;
/*
EWB
EWB
EWB
*/
	GoBoard another_test_board(b);
	another_test_board.DisplayBoard();
	std::cout << "error code: " << (int) another_test_board.error_code << "\n";

	const GoSerial c = 5299;
/*
BWE
BWE
BWE
*/
	GoBoard another_another_test_board(c);
	another_test_board.DisplayBoard();
	std::cout << "error code: " << (int) another_another_test_board.error_code << "\n";
	
	//return (0);

	uint64_t total_legal_state = 0;	
	//uint64_t reduced_rotate_state = 0;
	//uint64_t reduced_symmetric_state = 0;

	for ( int i=0; i<MAX_SERIAL; ++i ) {
		GoBoard board(i);
		if ( board.error_code == 0 ) {
			++total_legal_state;
		}
	}
	printf("total legal states: %lld\n", total_legal_state);

	return (0);
}