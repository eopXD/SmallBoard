#include "GoBoard.h"

using namespace GoConstant;

// this finds the most "head" parent_id of the block (since blocks are 
// connected by disjoint set)
GoCoordId GoBoard::FindCoord ( const GoCoordId id ) {
	if ( stones[id].parent_id == stones[id].self_id ) { // i am ancestor!
		return (stones[id].self_id);
	}
	return (stones[id].parent_id = FindCoord(stones[id].parent_id));
}

// get BlockId of some 'id' on the board
void GoBoard::GetBlockIdByCoord ( const GoCoordId id ) {
	if ( EmptyStone == board_state[id] ) {
		return (BLOCK_UNSET);
	} 
	// find ancestor stone and its corresponding block_id
	return (stones[FindCoord(id)].block_id);
}


// the number of unique neighbor blocks recorded in nb_id[0]
void GoBoard::GetNeighborBlocks ( GoBlock &blk, const GoCoordId target_id, 
 GoBlockId *nb_id ) {
	nb_id[0] = 0;
	blk.SetStoneState(target_id);
	FOR_NEIGHBOR(target_id, nb) {
		blk.SetVirtLiberty(*nb);
		if ( EmptyStone == board_state[*nb] ) {
			blk.SetLiberty(*nb);
		} else {
			nb_id[++nb_id[0]] = GetBlockIdByCoord(*nb);
		}
	}
	sort(nb_id+1, nb_id+1+nb_id[0]);
	nb_id[0] = unique(nb_id+1, nb_id+nb_id[0]+1) - nb_id - 1;
}

// blk is the GoBlock we are potentially adding if we are placing the stone on
// 'target_id'
// if the move is illegal, return -1
GoCounter GoBoard::TryMove ( GoBlock &blk, const CoordId target_id, 
 GoBlockId *nb_id, GoBlockId* die_id, GoCoordId lib_count ) {
 	if ( !legal_move_map[target_id] ) {
 		return (-1);
 	}
 	GoCounter cnt = 0;
 	blk.


}

// get new block (from idx - 'block_in_use') or re-used GoBlock (from stack)
void GoBoard::GetNewBlock ( GoBlockId &blk_id ) {
	blk_id = BLOCK_UNSET;
	if ( !recycled_block.empty() ) {
		blk_id = recycled_block.top();
		recycled_block.pop();
	} else {
		id = block_in_use++;
	}
	block_pool[blk_id].Reset();
}

// get all possible move for the current board position
void GoBoard::GetPossibleMove () {
	GoBlockId blk_id;
	GoBlockId tmp[2][5];
	
	bool have_empty_neighbor;

	legal_move_map.reset();
	GetNewBlock(blk_id);

	GoBlock &blk = block_pool[blk_id];

	FOR_EACH_COORD(i) {
		if ( EmptyStone != board_state[i] ) { // can't put stone on non-empty
			continue;
		}
		if ( ko_position == i ) { // can not put stone on ko
			continue;
		}
		have_empty_neighbor = 0;
		FOR_NEIGHBOR(i, nb) {
			if ( EmptyStone == board_state[*nb] ) {
				have_empty_neighbor = 1;
				break;
			}
		}
		if ( have_empty_neighbor ) {
			legal_move_map = 1;
			continue;
		}
		TryMove(blk, i, tmp[0], tmp[1]);

	}


}