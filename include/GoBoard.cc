#include "GoBoard.h"

using namespace GoConstant;

// returns own color
GoStoneColor SelfColor () {
	return (current_player);
}
GoStoneColor OpponentColor () {
	return (opponent_player);
}

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


// in this function, we also place a stone ONTO 'blk', on position 'target_id'
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
// else, return the number of opponent stone can be eaten in this move
GoCounter GoBoard::TryMove ( GoBlock &blk, const CoordId target_id, 
 GoBlockId *nb_id, GoBlockId* die_id, GoCoordId max_lib=SMALLBOARDSIZE ) {
 	if ( !legal_move_map[target_id] ) {
 		return (-1);
 	}
 	GoCounter cnt = 0;
 	
 	blk.Reset();
 	blk.stone_count - 1;
 	blk.color = SelfColor();

 	GetNeighborBlocks(blk, target_id, nb_id);

 	die_id[0] = 0;
 	for ( GoBlockId i=1; i<=nb_id[0]; ++i ) {
 		GoBlock &nb_blk = block_pool[nb_id[i]];
 	// if neighbor block is not my color...
 		if ( SelfColor() != nb_blk.color ) {
 		// and my neighboring block is only 1 liberty...
 			if ( 1 == nb_blk.CountLiberty() ) {
 			// then it is eat-able!!!!
 				cnt += nb_blk.stone_count;
 				die_id[++die_id[0]] = nb_id[i];
 			}
 		} 
 	// else it is a friendly block, so I try to merge it...
 		else {
 			blk.TryMergeBlocks(nb_blk);
 		}
 	}

 	blk.ResetLiberty(target_id);
 	if ( blk.CountLiberty() >= lib_count ) {
 		return (cnt);
 	}
 	if ( 0 != die_id[0] ) {
 		for ( GoBlockId i=1; i<=die_id[0]; ++i ) {
 			const GoBlock &die_blk = block_pool[die_id[i]];
 			// opponent stones I removed will become my liberty
 			blk.liberty_state |= die_blk.stone_state;
 			blk.liberty_state &= blk.virtual_liberty_state;
 		}
 	}
 	return (cnt);
}

// recycle the block, save it into stack
void RecycleBlock ( const GoBlockId blk_id ) {
	block_pool[blk_id].in_used = false;
	recycled_block.push(blk_id);
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
// [0][[?] is neighbor block_id, [1][?] is block_id killed
// [?][0] is the counter, from [?][1]~[?][4] stores the value
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
		legal_move_map.set(i);
		if ( have_empty_neighbor ) {
			continue;
		}
		
		TryMove(blk, i, tmp[0], tmp[1]);
		blk.CountLiberty();
		if ( blk.liberty_count <= 0 ) {
			legal_move_map[i].reset(i);
			continue;
		}
	// check for basic Ko
		if ( game_length > 2 ) {
			GoHash new_zobrist_value = current_zobrist_value;
			new_zobrist_value ^= zobrist_switch_player;

			GoBlockId *die_id = tmp[1];
			for ( GoBlockId i=1; i<=die_id[0]; ++i ) {
				FOR_BOCK_STONE(id, block_pool[die_id[i]],
					new_zobrist_value ^= zobrist_board_hash_weight[OpponentColor()][id];
				);
			}

			if ( record_zobrist[(game_length-1+4)&3] == record_zobrist[(game_length-3+4)&3] ) {
				legal_move_map.reset(i);
			}
		}
	}
	RecycleBlock(blk_id);
}