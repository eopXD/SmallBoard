// Copyright (C) 2019 Yueh-Ting Chen (eopXD)
/*! \file GoStone.h
	\brief Do disjoint-set on the stones, also record the block_id of which 
	 the stone corresponds to, this can increase speed of move generation.

	\author Yueh-Ting Chen (eopXD)
	\project Efficient & Succinct Small Board Go
*/

#ifndef SMALLBOARD_GOSTONE_H
#define SMALLBOARD_GOSTONE_H

#include "comm.h"

struct GoStone {
	GoCoordId self_id;      // Id of this stone
	GoBlockId block_id;     // Id of its block
	GoCoordId next_id;      // Use like link-list
	GoCoordId parent_id;    // Use like union-find-set (rooted by tail)

	// inline void SetSelfId(GoCoordId id) { self_id = id; }

	inline void Reset ( GoBlockId id=GoConstant::BLOCK_UNSET ) {
		next_id = parent_id = self_id;
		block_id = id;
	}
};

#endif
