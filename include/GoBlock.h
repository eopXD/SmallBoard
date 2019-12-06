// Copyright (C) 2019 Yueh-Ting Chen (eopXD)
/*! \file GoBlock.hpp
	\brief GoBlock: stones together forms 'string', we mainly care about the
	 liberty of the string. We also need to maintain the merge between strings.

	\author Yueh-Ting Chen (eopXD)
	\project Efficient & Succinct Small Board Go
*/
#ifndef SMALLBOARD_GOBLOCK_H
#define SMALLBOARD_GOBLOCK_H

#include "comm.h"

struct GoBlock {
	GoBlockId self_id;			// id of this block
	bool in_use; 				// whether the block is in-use
	GoCounter liberty_count; 	// count of liberty
	GoCounter stone_count; 		// count of stone
	GoStoneColor color; 		// player that owns the block
	
	GoStone *stones; 			// array pointer to GoBoard.stones
	GoCoordId head, tail;		// head and tail position stones of the block

// LSB style, lowest bit is represents the 0th in 'id'
// liberty_state[idx] = 1 means that there is liberty on the position 'idx'
	uint32_t liberty_state;
/* virtual liberty represents the place not occupied by the GoBlock
    virtual_liberty_state[idx] = 1 means the position 'idx' is not occupied */
	uint32_t virtual_liberty_state;
// stone_state[idx] = 1 means that there us a stone on the position 'idx'
	uint32_t stone_state;

	void Reset () { // reset the block (for re-use)
		in_use = false;
		liberty_count = stone_count = 0;
		liberty_state = virtual_liberty_state = stone_state = 0;
	}
	inline void SetLiberty ( const GoCoordId id ) {
		liberty_state |= 1ull << id;
	}
	inline void SetVirtLiberty ( const GoCoordId id ) {
		virtual_liberty_state |= 1ull << id;
	}
	inline void SetStone ( const GoCoordId id ) {
		stone_state |= 1ull << id;
	}
	inline void ResetLiberty ( const GoCoordId id ) {
		liberty_state &= ~(1ull << id);
	}
	inline void ResetVirtLiberty ( const GoCoordId id ) {
		virtual_liberty_state &= ~(1ull << id);
	}
	inline void ResetStone ( const GoCoordId id ) {
		stone_state &= ~(1ull << id);
	}
	inline bool GetLiberty ( const GoCoordId id ) {
		return (liberty_state & (1ull << id));
	}
	inline GoCounter CountLiberty () {
		return (liberty_count = __builtin_popcount(liberty_state));
	}
	inline GoCounter CountStone () {
		return (stone_count = __builtin_popcount(stone_state));
	}
	bool IsNoLiberty () {
		return (0 == this->CountLiberty());
	}
	
	GoStone* GetHead () const {
		return (this->stones+this->head);
	}
	GoStone* GetTail () const {
		return (this->stones+this->tail);
	}

	inline void TryMergeBlocks ( const GoBlock &a ) {
		this->stone_count += a.stone_count;
		this->stone_state |= a.stone_state;
		this->virtual_liberty_state |= a.virtual_liberty_state;
		this->virtual_liberty_state &= ~this->stone_state;
		this->liberty_state |= a.liberty_state;
		this->liberty_state &= virtual_liberty_state;
	}

	// manipulating on GoBoard.stones[]
	void MergeBlocks ( const GoBlock &a ) { // chain the stones of the block
	/* this is one-way link list, so make a.head the head
	 and connect the tail to this->head */
		a.GetTail()->next_id = this->GetHead()->self_id;
		this->head = a.head;
	/* disjoint-set, let a's tail link to this->tail */
		a.GetHead()->parent_id = a.GetTail()->parent_id = this->GetTail()->self_id;
	}


};
#endif