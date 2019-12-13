// Copyright (C) 2019 Yueh-Ting Chen (eopXD)
/*! \file comm.h
	\brief Error logger - elog()
	 GoBitState: The structure associates I/O with disk. Information stored 
	  compact and MSB style.
	 GoConstant: Type definitions of variables, and constants of the board,
	  including the SmallBoard size.
	 GoFunction: Utility functions for some judgement on the board. Also 
	  there are preprocessed tables used for Zobrist Hash and precalculated
	  table that corresponds (id/coordinate) to its neighbor (id/coordinate).

	\author Yueh-Ting Chen (eopXD)
	\project Efficient & Succinct Small Board Go
*/
#ifndef SMALLBOARD_COMMON_H
#define SMALLBOARD_COMMON_H

#include <cstdio>
#include <cstring>
#include <cmath>
#include <inttypes.h>
#include <vector>
#include <utility>

// error logger
template <typename... Args> inline void elog(const char *s, Args... args) {
    fprintf(stderr, s, args...);
    fflush(stderr);
}

/***************************************************************************/
/****** TYPE DEFINITION ******/
/* the reason why we need different declaration names is to increase the 
   readability of the code */

// 3 stands for Black/White/Empty
// log2 ( 3^25 ) ~ 39.624
using GoSerial = uint64_t; // serial number to represent the board position
using GoHash = uint64_t; // for Zobrist Hash
// all declaration can fit into uint8_t because we are dealing with small
// boards right now.
using GoStoneColor = uint8_t; // color of stone on the board
using GoCoordId = int8_t; // Id that corresponds to the coordination
using GoScore = uint8_t; // Score of the board
using GoBlockId = int8_t; // Blocks on the board (check GoBlock)
using GoCounter = uint16_t; // size of counters

using GoPosition = std::pair<GoCoordId, GoCoordId>;
using GoHashPair = std::pair<uint64_t, uint64_t>;

// for error handling
using GoError = int8_t;

/****************************************************************************/

namespace MemoryConstant {

/* !!!! CAUTION !!!!
// THIS SETTING NEED TO BE ADJUSTED ACCORDING TO ENVIRONMENT
*/
const uint64_t MEM_BLOCK_SIZE = 16000000ull;

// and because we are not saving it compactly, so 8 byte servers for 1 state.
const uint64_t STATE_PER_FILE = MEM_BLOCK_SIZE / 8;

};

namespace GoConstant {

// GoStone (3 possibility as a Stone, reflects on zobrist_hash)
const GoStoneColor EmptyStone = 0;
const GoStoneColor BlackStone = 1;
const GoStoneColor WhiteStone = 2;
const char COLOR_CHAR[] = "EBW";
const char *const COLOR_STRING[] = {"Empty", "Black", "White"};

// GoCoordId
const GoCoordId BORDER_R = ROW;
const GoCoordId BORDER_C = COL;
const GoCoordId SMALLBOARDSIZE = ROW*COL; // specified in makefile
const GoCoordId COORD_PASS = -1; // pass
const GoCoordId COORD_UNSET = -2; // does not contain valid value.
const GoCoordId COORD_RESIGN = -3; // resign (surrender)

const GoCoordId DELTA_SIZE = 4;
const GoCoordId COORD_DX[DELTA_SIZE] = {0, 1, 0, -1};
const GoCoordId COORD_DY[DELTA_SIZE] = {-1, 0, 1, 0};

const GoBlockId MAX_BLOCK_SIZE = 20;
const GoBlockId BLOCK_UNSET = -1;

const GoSerial MAX_SERIAL = pow(3.0, GoConstant::SMALLBOARDSIZE);
} // namespace GoConstant


/***************************************************************************/
// turn bit is reduced, as we assume small boards are all black players turn
// we want to encode it as we MSB style, so we start from 63 and go down to 
// zero. The sequence is shown below in GenNeededBits.
// the following are their corresponding sequence in 
// vector 'encode_size' and 'encode_lr'

// needed bits generated at GenNeededBit
extern std::vector<uint8_t> encode_size; 
// start and end of the value stored
extern std::vector<std::pair<uint8_t, uint8_t>> encode_lr; 

namespace GoBitState {

extern void CreateEncoding ();
extern void CreateEncode ();

// the encoding sequence of values (also check CreateEncoding)
const int ENCODE_SERIAL = 0;
const int ENCODE_KOPASS = 1;
const int ENCODE_OUTDEG = 2;
const int ENCODE_RESULT = 3;
const int ENCODE_SCORE = 4;

} // namespace GoBitState

/***************************************************************************/
/* generated by CreateZobristHash */
// hash for [stone_color][board position]
extern GoHash zobrist_board_hash_weight[3][GoConstant::SMALLBOARDSIZE];
// hash for ko on [board position]
extern GoHash zobrist_ko_hash_weight[GoConstant::SMALLBOARDSIZE];
// hash for [stone_color]
extern GoHash zobrist_player_hash_weight[3];
// hash for switching turns
extern GoHash zobrist_switch_player;

/* generated by CreateNeighborCache */
// how many neighbors the [position] gets
extern uint8_t cached_neighbor_size[GoConstant::SMALLBOARDSIZE];
// pre-calculated neighbor ID
extern GoCoordId cached_neighbor_id[GoConstant::SMALLBOARDSIZE][4];
// pre-calculated neighbor Coord
extern std::vector<GoPosition> 
 cached_neighbor_coord[GoConstant::BORDER_R][GoConstant::BORDER_C];

namespace GoFunction {

extern bool InBoard ( const GoCoordId id );
extern bool InBoard ( const GoCoordId x, const GoCoordId y );

extern bool IsPass ( const GoCoordId id );
extern bool IsPass ( const GoCoordId x, const GoCoordId y );

extern bool IsUnset ( const GoCoordId id );
extern bool IsUnset ( const GoCoordId x, const GoCoordId y );

extern bool IsResign ( const GoCoordId id );
extern bool IsResign ( const GoCoordId x, const GoCoordId y );

extern void IdToCoord ( const GoCoordId id, GoCoordId &x, GoCoordId &y );
extern GoCoordId CoordToId ( const GoCoordId x, const GoCoordId y );

extern void CreateGlobalVariable ();
extern void CreateZobristHash ();
extern void CreateNeighborCache ();

} // namespace GoFunction

/***************************************************************************/
/* conventional for-loops */

// nb is short for neighbor
// for the neighbor of the (id), we put the neighbor's id into pointer nb
#define FOR_NEIGHBOR(id, nb) for ( GoCoordId *nb=cached_neighbor_id[(id)], i=0;\
 i<cached_neighbor_size[(id)]; ++i, ++nb )

// for all available 'id'
#define FOR_EACH_COORD(id) for ( GoCoordId id=0; \
 id<GoConstant::SMALLBOARDSIZE; ++id )

// for all available 'blk_id'
#define FOR_EACH_BLOCK(id) for ( GoBlockId id=0; \
 id<GoConstant::MAX_BLOCK_SIZE; ++id )

#endif