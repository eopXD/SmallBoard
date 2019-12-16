// Copyright (C) 2019 Yueh-Ting Chen (eopXD)
/*! \file FindAllPossibleKoCoord/main.cpp
	\brief Find all serial numbers that is legal. (no block on the board
	 shall have zero liberty.

	\author Yueh-Ting Chen (eopXD)
	\project Efficient & Succinct Small Board Go
*/


#include <cstdio>
#include <iostream>
#include <algorithm>

#include "GoBoard.h"

using namespace GoConstant;
using namespace GoFunction;
using namespace std;

void display ( BIT_STATE bitstring ) {		
	FOR_EACH_COORD(id) {
		putchar(bitstring&1 ? '1' : '0');
		bitstring >>= 1;
		if ( id%BORDER_C == BORDER_C-1 ) {
			putchar('\n');
		}
	}
}
uint32_t GetAllKo ( GoSerial const serial ) {
	GoBoard board(serial);
	
	BIT_STATE ko_state = 0;
	
	board.DisplayBoard();
	puts("");
	
	FOR_EACH_COORD(id) {
		GoCoordId ko_position = board.CheckKoPosition(id);
		printf("%d: %d\n", id, ko_position);
		if ( ko_position < 0 ) {
			continue;
		}
		ko_state |= 1ull << ko_position;
	} 

	display(ko_state);
	return (ko_state);
}
int main ()
{
	GetAllKo(5664941825);
	GetAllKo(10023933850);
	return (0);
}