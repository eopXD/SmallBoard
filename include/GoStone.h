/*! \file GoStone.h
    \brief Circular-singly linked-list to connnect stones.
     Disjoint set to find the ancestor stone of the block.
     Each stone contains its own id, and holds a GoBlockId
     that represents the GoBlock that contains the stone.
*/

#ifndef SMALLBOARD_GOSTONE_H
#define SMALLBOARD_GOSTONE_H

#include "comm.h"

struct GoStone {
    GoCoordId self_id;   // id of this stone
    GoBlockId block_id;  // id of its block
    GoCoordId next_id;   // circular linked-list to connect stones
    GoCoordId parent_id; // use like union-find-set (rooted by head)

    // set stone into blk_id
    inline void Reset(GoBlockId blk_id = GoConstant::BLOCK_UNSET)
    {
        // self_id shall be set initially when we allocate the stone
        next_id = parent_id = self_id;
        block_id = blk_id;
    }
};

#endif
