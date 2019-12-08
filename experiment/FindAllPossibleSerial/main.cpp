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
	char file_name[105];
	BITSET legal_state;	

	uint64_t total_legal_state = 0;	
	uint64_t reduced_rotate_state = 0;
	uint64_t reduced_symmetric_state = 0;

	cout << MAX_SERIAL << "\n";

	const GoSerial test_serial = 100120120;
	
	GoBoard test_board(test_serial);
	

	return (0);
}