// Copyright (C) 2019 Yueh-Ting Chen (eopXD)
/*! \file comm.hpp
	\brief Common definitions on types, also basic constants of the game.

	\author Yueh-Ting Chen (eopXD)
	\project Efficient & Succinct Small Board Go
*/
#ifndef SMALLBOARD_COMMON_H
#define SMALLBOARD_COMMON_H

#include <inttypes.h>
#include <cmath>

namespace GoState {
// turn bit is reduced, as we assume small boards are all black players turn
// we want to encode it as we MSB style, so we start from 63 and go down to 
// zero. The sequence is shown below in GenNeededBits.
// the following are their corresponding sequence in vector 'NeededBits' and 'Encode'
VI NeededBits;
VII Encode;
void GenNeededBits () {
	ULL diff_values;
	int now = 63;
// all possible board position
	diff_values = pow(3.0, ROW*COL);
	NeededBits.PB( ceil(log(diff_values)/log(3)) );
	Encode.PB( MP(now-NeededBits.back()+1, now) );
	now -= now-NeededBits.back();
// ko & pass
	diff_values = ROW*COL+3;
	NeededBits.PB( ceil(log(diff_values)/log(3)) );
	Encode.PB( MP(now-NeededBits.back()+1, now) );
	now -= now-NeededBits.back();
// degree to child board
	diff_values = ROW*COL+1;
	NeededBits.PB( ceil(log(diff_values)/log(3)) );
	Encode.PB( MP(now-NeededBits.back()+1, now) );
	now -= now-NeededBits.back();
// win/draw/lose/un-determined
	diff_values = 4;
	NeededBits.PB( ceil(log(diff_values)/log(3)) );
	Encode.PB( MP(now-NeededBits.back()+1, now) );
	now -= now-NeededBits.back();
// [-ROW*COL, ROW*COL]/un-determinded
	diff_values = 2*ROW*COL+2;
	NeededBits.PB( ceil(log(diff_values)/log(3)) );
	Encode.PB( MP(now-NeededBits.back()+1, now) );
	now -= now-NeededBits.back();
	if ( now < -1 ) {
		elog("64 bit is not enough to encode the given ROW & COL\n");
		exit(1);
	}
}

} // namespace GoState


/* the reason why we need different declaration names is to increase the 
   readability of the code */

// 3 stands for Black/White/Empty
// log2 ( 3^25 ) ~ 39.624
using GoSerial = uint64_t; // serial number to represent the board position

// all declaration can fit into uint8_t because we are dealing with small
// boards right now.
using GoStoneColor = uint8_t; // color of stone on the board
using GoCoordId = int8_t; // Id that corresponds to the coordination
using GoBlockId = int8_t; // Blocks on the board (check GoBlock)

namespace GoComm {

// GoStone (4 possibility as a Stone, reflects on zobrist_hash)
const GoStoneColor EmptyStone = 0;
const GoStoneColor BlackStone = 1;
const GoStoneColor WhiteStone = 2;
const char *const COLOR_STRING[] = {"Empty", "Black", "White"};

// GoCoordId
const GoCoordId BORDER_R = ROW;
const GoCoordId BORDER_C = COL;
const GoCoordId SMALLBOARDSIZE = ROW*COL;
const GoCoordId COORD_PASS = -1;
const GoCoordId COORD_UNSET = -2;
const GoCoordId COORD_RESIGN = -3;
const GoCoordId COORD_DX[] = {0, 1, 0, -1};
const GoCoordId COORD_DY[] = {-1, 0, 1, 0};

} // namespace GoComm

extern uint64_t zobrist_board_hash_weight[3][GoComm::SMALLBOARDSIZE];
extern uint64_t zobrist_ko_hash_weight[GoComm::SMALLBOARDSIZE];
extern uint64_t zobrist_player_hash_weight[3];

#endif