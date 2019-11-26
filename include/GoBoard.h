// Copyright (C) 2019 Yueh-Ting Chen (eopXD)
/*! \file GoBoard.hpp
	\brief GoBoard is the Board that will be used to simulate gameplay.
	 Pre-allocate a few GoBlock structure for the Board, and can be reused 

	\author Yueh-Ting Chen (eopXD)
	\project Efficient & Succinct Small Board Go
*/
#ifndef SMALLBOARD_GOBOARD_H
#define SMALLBOARD_GOBOARD_H

#include <stack>

#include "comm.h"
#include "GoBlock.h"

// Right here GoBoard is smallBoard, can be extended to official size of Go.
class GoBoard {

public:

// THE ACTUAL FUNCTION THAT MOVES THE BOARD
// be called after GetPossibleMove
// returns 0 if success
// error code:
//	-1: not a legal move
//	-2: a self-eat move
	int Move ( const GoCoordId id );
	int Move ( const GoCoordId x, const GoCoordId y );

protected:
// returns own color
	GoStoneColor SelfColor ();
// returns opponent color
	GoStoneColor OpponentColor ();
// Give the turn to opponen
	inline void HandOff ();
// return whether the move is legal
	inline bool IsLegal ( const GoCoordId id );
// finds ancestor stone (it is used to find block_id of the ancestor stone)
	GoCoordId FindCoord ( const GoCoordId id );
// get BlockId of some 'id' on the board
	void GetBlockIdByCoord ( const GoCoordId id );
// get neighbor blocks of of 'target_id', which is at most 4 blocks
// the BlockId saved into 'nb_id'
// a stone is placed on 'target_id' inside 'blk'
	void GetNeighborBlocks ( GoBlock &blk, const GoCoordId target_id, 
		GoBlockId *nb_id );	
// recycle the block, save it into stack
	void RecycleBlock ( const GoBlockId blk_id );
// get new block (from idx - 'block_in_use') or re-used GoBlock (from stack)
	void GetNewBlock ( GoBlockId &blk_id );
// try to do the move on 'target_id'
	GoCounter TryMove ();
// get all possible move for the current board position
	void GetPossibleMove ();

private :
// serial number
	GoSerial serial;
/* GoBlock pool and some maintenance */
	GoBlock block_pool[GoConstant::MAX_BLOCK_SIZE];
	std::stack<GoBlockId> recycled_block;
	GoCounter block_in_use;
/* Naiive informations of Board State */
	GoStone stones[GoConstant::SMALLBOARDSIZE]; // stones are like linked-list
	GoStoneColor board_state[GoConstant::SMALLBOARDSIZE];
	GoStoneColor current_player, opponent_player;
	GoCoordId previous_move;
	GoCoordId ko_position, pass_count;
// status of the board
	GoCounter game_length;
	// sequence of existence of blocks
	GoCounter visited_position[GoConstant::MAX_BLOCK_SIZE]; // 這個東東的用意還有待商榷
	bool is_double_pass;
/* Zobrist Hash to forbid Basic Ko (we allow Positional SuperKo) */
	GoHash record_zobrist[4];
	GoHash current_zobrist_value;
/* important features */
// legal_move_map[idx] = 1 means we can play a stone onto that position
	bitset<GoConstant::SMALLBOARDSIZE> legal_move_map; 
// score calculation: neighbors if consist both black and white, then both
// add 1, else add score to the corresponding color. Also add stones as
// scores. score = black - white (since we are reducing)
// let it be an unsigned integer, so we add ROW*COL to the score.
	GoScore board_score;
}


/* conventional for-loop */

// for all the GoBlocks such that in_use = true
#define FOR_BLOCK_IN_USE (i)\
 for ( GoBlockId i=0; i<block_in_use; ++i )

// stones[] are maintained in a linked-list style
// iteration around stones of a GoBoard
#define FOR_BLOCK_STONE (id, blk, loop_body) {\
	GoCoordId id = (blk).head;\
	while ( 1 ) {\
		loop_body\
		if ( id == stones[id].next_id) {\
			break;\
		}\
		id = stones[id].next_id;\
	}\
}
#endif