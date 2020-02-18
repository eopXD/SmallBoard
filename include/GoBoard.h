// Copyright (C) 2019 Yueh-Ting Chen (eopXD)
/*! \file GoBoard.hpp
	\brief GoBoard is the Board that will be used to simulate gameplay.
	 Pre-allocate a few GoBlock structure for the Board, and can be reused 

	\author Yueh-Ting Chen (eopXD)
	\project Efficient & Succinct Small Board Go
*/
#ifndef SMALLBOARD_GOBOARD_H
#define SMALLBOARD_GOBOARD_H

#include <cstdio>
#include <cstring>
#include <iostream>
#include <stack>
#include <bitset>
#include <algorithm>

#include "comm.h"
#include "GoStone.h"
#include "GoBlock.h"

// Right here GoBoard is smallBoard, can be extended to official size of Go.
class GoBoard {

public:
// constructor
	GoBoard ();
	GoBoard ( const GoBoard &rhs );
// destructor
	~GoBoard ();
// copy
	void CopyFrom ( const GoBoard &src );
protected:
// redirect the '*stone' pointer in GoBlock to the stones[] after CopyFrom()
	void FixBlockInfo ();


/* Crucial function: Move */
// this function actually modifies the board_state, and stones with zero
// liberty after the move will be removed
public:
// THE ACTUAL FUNCTION THAT MOVES THE BOARD
// be called after GetPossibleMove
// error code:
//   0: success
//	-1: not a legal move
//	-2: a self-eat move
	GoError Move ( const GoCoordId id );
	GoError Move ( const GoCoordId x, const GoCoordId y );

/* Display, only for debug purpose */
public:
	void DisplayBoard ();
	void DisplayLegalMove ();
/* Board Manipulation for checking if state if reduce-able */
public:
// check if this board is in its minimal serial number
// NOTE: this function cannot be used in real gameplay
// because the flipping and rotation will messup the board
	GoSerial GetMinimal ();
// get serial of the current board
	GoSerial GetSerial ();
// get ko of the current board
	GoCoordId GetKo ();
//protected:
// rotate clock-wise (90 degree)
	void RotateClockwise ();
// flip in a LR symmetric matter
	void FlipLR ();
// returns if 2 passes is consecutively used (during gameplay)
	bool IsDoublePass ();


/* for initialization via serial number */
public: 
// Uses SetStone to setup the board, 'initialze' can be set to 1 if want
// the board details to be initialized for future play. 
// error code may be set 
	GoBoard ( const GoSerial _serial, bool initialize=0 );
	/* for error handling, can be direct accessed by outside */
	GoError error_code;
protected:
// Place stones onto the board, but don't need maintenance of detail
// board position or check if it is legal, only check for self-eat or eat move
// The stone will be placed onto the board_state.
// error code:
//   0: ok
//	-1: self-eat move 
//	-2: eat-opponent move
	GoError SetStone ( const GoCoordId id, const GoStoneColor stone_color );

/* for checking if this board can have a ko at the given position 'id' */
public:
// NOTE: 'opponent_color' is set WHITE, reduce small board's to black's turn

// call this function only when the GoBoard is constructed by GoSerial
// return value
// 			-1: no possible Ko for this stone's neighbor
//	 GoCoordId: the GoCoordId of the ko
	GoCoordId CheckKoPosition ( const GoCoordId id, 
 const GoStoneColor opponent_color=GoConstant::WhiteStone );

/* for whether this board is a terminate state */
public:
// Conditions for terminate board
	uint8_t CheckTerminate ( bool black_no_move, bool white_no_move );
// NOTE: reduce all boards to black's turn (same as previous phase CheckKo)
// NOTE: call this function after board is initialized
// returns a ploynomial of series of 5
// 0: NULL Value (illegal board, or this ko-position is not)
// 1: Not terminate
// 2: Win
// 3: Lose
// 4: Draw
	uint64_t CheckTerminates ( const uint32_t ko_state );

// returns current board score difference (BLACK-WHITE)
// result also cached in '.board_score' variable
// > 0: WIN  for current player
// < 0: LOSE for current player 
// = 0: DRAW for current player
	GoCoordId CalcScore ( 
 GoStoneColor opponent_color=GoConstant::WhiteStone );

/* Interface to interact with private assets */
	void SetTurn ( GoStoneColor me, GoStoneColor you ) {
		current_player = me;
		opponent_player = you;
	}
	void SetKo ( GoCoordId id ) {
		ko_position = id;
	}

protected:
// returns own color
	GoStoneColor SelfColor ();
// returns opponent color
	GoStoneColor OpponentColor ();
// Give the turn to opponent
	inline void HandOff ();
// return whether the move is legal
	inline bool IsLegal ( const GoCoordId id );
// finds ancestor stone (it is used to find block_id of the ancestor stone)
	GoCoordId FindCoord ( const GoCoordId id );
// get BlockId of some 'id' on the board
	GoBlockId GetBlockIdByCoord ( const GoCoordId id );
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
	GoError TryMove ( GoBlock &blk, const GoCoordId target_id, 
 GoBlockId *nb_id, GoBlockId* die_id, 
 GoCoordId max_lib=GoConstant::SMALLBOARDSIZE );
public:
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
	GoStone stones[GoConstant::SMALLBOARDSIZE]; // one-way linked-list
	GoStoneColor board_state[GoConstant::SMALLBOARDSIZE];
	GoStoneColor current_player, opponent_player;
	GoCoordId previous_move;
	GoCoordId ko_position;
// status of the board
	GoCounter game_length;
	// sequence of existence of blocks, to deternine if need to update block
	GoCounter visited_position[GoConstant::MAX_BLOCK_SIZE];
	bool is_double_pass;
/* Zobrist Hash to forbid Basic Ko (we allow Positional SuperKo) */
	GoHash record_zobrist[4];
	GoHash current_zobrist_value;	
// score calculation: only count stones, and empty intersection that is
// 'fully' surrounded by the same colour of stones.
// score is cached into this variable when CheckScore is called
// score = BLACK - WHITE (since we are reducing) 
	GoScore board_score;
/* important features */
// legal_move_map[idx] = 1 means we can play a stone onto that position
	std::bitset<GoConstant::SMALLBOARDSIZE> legal_move_map; 
};


/* conventional for-loop */

// for all the GoBlocks such that in_use = true
#define FOR_BLOCK_IN_USE(i)\
 for ( GoBlockId i=0; i<block_in_use; ++i )

// stones[] are maintained in a linked-list style
// iteration around stones of a GoBoard
#define FOR_BLOCK_STONE(id, blk, loop_body) {\
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