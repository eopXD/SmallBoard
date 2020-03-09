// Copyright (C) 2019 Yueh-Ting Chen (eopXD)
/*! \file GoStone.h
	\brief Circular linked-list to connnect stones.
	 Disjoint set to find the ancestor stone of the block.
	 Each stone contains its own id, and holds a GoBlockId
	 that represents the GoBlock that contains the stone.

	\author Yueh-Ting Chen (eopXD)
	\project Efficient & Succinct Small Board Go
*/

#ifndef SMALLBOARD_GOSTONE_H
#define SMALLBOARD_GOSTONE_H

#include "comm.h"

struct GoStone {
	GoCoordId self_id;      // Id of this stone
	GoBlockId block_id;     // Id of its block
	GoCoordId next_id;      // circular linked-list to connect stones
	GoCoordId parent_id;    // Use like union-find-set (rooted by tail)

	// inline void SetSelfId(GoCoordId id) { self_id = id; }

// this function mainly used when... GoBoard::Move, 
// set the stone to the blk_id
	inline void Reset ( GoBlockId blk_id=GoConstant::BLOCK_UNSET ) {
// self_id shall be set initially when we allocate the stone
		next_id = parent_id = self_id;
		block_id = blk_id;
	}
};

#endif
