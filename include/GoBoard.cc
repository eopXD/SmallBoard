#include "GoBoard.h"

using namespace GoConstant;
using namespace GoFunction;

GoBoard::GoBoard () {
	CreateGlobalVariable();

	FOR_EACH_COORD(i) {
		stones[i].self_id = i;
	}
	FOR_EACH_BLOCK(i) {
		block_pool[i].in_use = false;
		block_pool[i].self_id = i;
		block_pool[i].stones = stones;
	}
	while ( not recycled_block.empty() ) {
		recycled_block.pop();
	}
	memset(board_state, 0, sizeof(board_state));
	memset(visited_position, 0, sizeof(visited_position));
	legal_move_map.reset();


	block_in_use = 0;
	
	current_player = BlackStone;
	ko_position = COORD_UNSET;
	is_double_pass = false;
	
	game_length = 0;
	current_zobrist_value = 0;
}
GoBoard::GoBoard ( const GoBoard &rhs ) : GoBoard() {
	CopyFrom(rhs);
}


void GoBoard::CopyFrom ( const GoBoard &src ) {
	*this = src;
	FixBlockInfo();
}

void FixBlockInfo () {
	FOR_EACH_BLOCK(i) {
		block_pool[i].stones = stones;
	}
}

// THE ACTUAL FUNCTION THAT MOVES THE BOARD
// be called after GetPossibleMove
// returns 0 if success
// error code:
//	-1: not a legal move
//	-2: a self-eat move
GoError GoBoard::Move ( const GoCoordId id ) {
	if ( not IsLegal(id) ) {
		return (-1);
	}
	++game_length;
	
	is_double_pass = IsPass(previous_move) and IsPass(id);
	previous_move = id;
	ko_position = COORD_UNSET;

// update Zobrist Hash
	{
		current_zobrist_value ^= zobrist_switch_player;
		if ( not IsPass(id) ) {
			current_zobrist_value ^= zobrist_board_hash_weight[SelfColor()][id];
		}
	}	
	if ( IsPass(id) ) {
		goto NEXT_TURN;
	}
// nb_id[0] and die_id[0] is counter, 
	GoBlockId blk_id, nb_id[5], die_id[5];
// opponent stone eaten when the move occurs
	GoCounter cnt; // 這個應該可以省略，留作之後參考
	
	GetNewBlock(blk_id);
	GoBlock &blk = block_pool[blk_id];

	if ( (cnt = TryMove(blk, id, nb_id, die_id)) < 0 ) { // this is a self-eat move
		return (-2);
	}
	stones[id].Reset(blk_id);
	blk.in_use = true;
	blk.head = blk.tail = id;
	for ( GoBlockId i=1; i<=nb_id[0]; ++i ) {
		GoBlock &nb_blk = block_pool[nb_id[i]];
		visited_position[nb_id[i]] = game_length;
		nb_blk.ResetLiberty(id);
		if ( SelfColor() == blk.color ) {
			blk.MergeBlocks(nb_blk);
			RecycleBlock(nb_id[i]);
		}
	}
	visited_position[blk_id] = game_length;
	for ( GoBlockId i=1; i<=die_id[0]; ++i ) {
		if ( 1 == block_pool[die_id[i]] and 1 == blk.stone_count 
		 and 1 == blk.CountLiberty() ) {
		// this is a Ko!
			ko_position = block_pool[die_id[i]].head;
		}
		// for the stones of the killed GoBlock...
		FOR_BLOCK_STONE(dead_stone, block_pool[die_id[i]], 
			// for the neighbor of the stones
			FOR_NEIGHBOR(dead_stone, nb) {
				// if the neighbor is my color, add liberty to my stone
				if ( SelfColor() == board_state[*nb] ) {
					GoBlockId my_blk_id = GetBlockIdByCoord(*nb);

					visited_position[my_blk_id] = game_length;
					block_pool[my_blk_id].SetLiberty(dead_stone);
				}
			}
			board_state[dead_stone] = EmptyStone;
			current_zobrist_value ^= 
			 zobrist_board_hash_weight[OpponentColor()][dead_stone];
		);
		RecycleBlock(die_id[i]);
	}
	// update the blocks that is effected in this move, update the liberty
	FOR_BLOCK_IN_USE(i) {
		if ( block_pool[i].in_use and visited_position[i] == game_length ) {
			block_pool[i].CountLiberty();
		}
	}
	board_state[id] = SelfColor();

NEXT_TURN:
	HandOff();
	GetPossibleMove();
	record_zobrist[(game_length-1+4)&3] = current_zobrist_value;
	return (0);

}
GoError GoBoard::Move ( const GoCoordId x, const GoCoordId y ) {
	return (Move(CoordToId(x, y)));
}

void GoBoard::DisplayBoard () const {
	FOR_EACH_COORD(id) {
		if ( (!id) and ((id%BORDER_C)==0) ) {
			putchar("\n");
		}
		putchar(COLOR_CHAR[board_state[id]]);
	}
}

// get serial number of this->board_state[][], also cache it into this->serial
GoSerial GoBoard::GetSerial () {
	serial = 0;
	for ( GoCoordId id=SMALLBOARDSIZE-1; i>=0; i-- ) {
		serial = serial*3 + board_state[id];
	}
	return (serial);
}

// rotate clock-wise (90 degree)
void GoBoard::RotateClockwise () {
	GoStoneColor tmp[SMALLBOARDSIZE];
	FOR_EACH_COORD(id) {
		int x, y;
		IdToCoord(id, x, y);
		
		tmp[CoordToId(y, BORDER_C-1-x)] = board_state[id];
	}
	memcpy(board_state, tmp, sizeof(tmp));
}

// flip in a LR symmetric matter
void GoBoard::FlipLR () {
	GoCoordId id = 0;
	for ( GoCoordId x=0; x<BORDER_R; ++x ) {
		for ( GoCoordId y=0; y<BORDER_C/2; ++y ) {
			swap(board_state[id], board_state[id+BORDER_C-1-y]);
			++id;
		}
	}
}

// Uses SetStone to setup the board, 'initialze' can be set to 1 if want
// the board details to be initialized for future play. 
// error code may be set
//   0: success
//	-1: construct fail
GoBoard::GoBoard ( const GoSerial _serial, bool initialize=0 ) : GoBoard() {
	for ( GoCoordId id=SMALLBOARDSIZE-1; i>=0; i-- ) {
		GoStoneColor stone_color = serial%3;
		serial /= 3;
		if ( SetStone(id, stone_color) != 0 ) { // see error code below
			error_code = -1;
			goto END_CONSTRUCT;
		}
	}
	error_code = 0;
END_CONSTRUCT:
/* build initialization of board detail for 
playing on the phase 'CheckKoStates'*/
	if ( initialize ) {
		/* do some initialization */ 
	}
}

// Place stones onto the board, but don't need maintenance of detail
// board position or check if it is legal, only check for self-eat or eat move
// The stone will be placed onto the board_state.
// error code:
//   0: success
//	-1: self-eat move
//  -2: eat-opponent move
GoBoard::GoError SetStone ( const GoCoordId id, const GoStoneColor stone_color ) {
// nb_id[0] and die_id[0] is counter, 
	GoBlockId blk_id, nb_id[5], die_id[5];
// opponent stone eaten when the move occurs
	GoCounter cnt;
	
	GetNewBlock(blk_id);
	GoBlock &blk = block_pool[blk_id];

	if ( (cnt = TryMove(blk, id, nb_id, die_id)) < 0 ) { // this is a self-eat move
		return (-1);
	}
	if ( die_id[0] != 0 ) { // this is an eating move
		return (-2);
	}
	stones[id].Reset(blk_id);
	blk.in_use = true;
	blk.head = hlk.tail = id;
	for ( GoBlockId i=1; i<=nb_id[0]; ++i ) {
		GoBlock &nb_blk = block_pool[nb_id[i]];
		visited_position[nb_id[i]] = game_length;
		nb_blk.ResetLiberty(id);
		if ( SelfColor() == blk.color ) {
			blk.MergeBlocks(nb_blk);
			RecycleBlock(nb_id[i]);
		}
	}
	for ( GoBlockId i=1; i<=die_id[0]; ++i ) {
		// for the stones of the killed GoBlock...
		FOR_BLOCK_STONE(dead_stone, block_pool[die_id[i]], 
			// for the neighbor of the stones
			FOR_NEIGHBOR(dead_stone, nb) {
				// if the neighbor is my color, add liberty to my stone
				if ( SelfColor() == board_state[*nb] ) {
					GoBlockId my_blk_id = GetBlockIdByCoord(*nb);
					block_pool[my_blk_id].SetLiberty(dead_stone);
				}
			}
			board_state[dead_stone] = EmptyStone;
		);
		RecycleBlock(die_id[i]);
	}
	board_state[id] = stone_color;
	return (0);
/*
NOTES: you can compare this function to GoBoard::Move(id), because this
is a reduced version of move, because we only care about initializing the 
stones onto the board and don't need to do any maintanence on the gaming
detail.
*/
}

// returns own color
GoStoneColor GoBoard::SelfColor () {
	return (current_player);
}
// return opponent color
GoStoneColor GoBoard::OpponentColor () {
	return (opponent_player);
}
// give the turn to opponent
inline void GoBoard::HandOff () {
	swap(current_player, opponent)
}
// return whether the move is legal
inline bool GoBoard::IsLegal ( const GoCoordId id ) {
	return ((id == IsPass(id)) or legal_move_map[id]);
}

// this finds the most "head" parent_id of the block (since blocks are 
// connected by disjoint set)
GoCoordId GoBoard::FindCoord ( const GoCoordId id ) {
	if ( stones[id].parent_id == stones[id].self_id ) { // i am ancestor!
		return (stones[id].self_id);
	}
	return (stones[id].parent_id = FindCoord(stones[id].parent_id));
}

// get GoBlockId of some 'id' on the board
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

// blk is the GoBlock we are 'potentially' adding if we are placing the stone
// on 'target_id'
// if the move is illegal, return -1
// else, return the number of opponent stone can be eaten in this move
GoError GoBoard::TryMove ( GoBlock &blk, const CoordId target_id, 
 GoBlockId *nb_id, GoBlockId* die_id, GoCoordId max_lib=SMALLBOARDSIZE ) {
 	if ( !legal_move_map[target_id] ) {
 		return (-1);
 	}
 	GoCounter cnt = 0;
 	
 	blk.Reset();
 	blk.stone_count = 1;
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
void GoBoard::RecycleBlock ( const GoBlockId blk_id ) {
	block_pool[blk_id].in_use = false;
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
			
			new_zobrist_value ^= zobrist_board_hash_weight[SelfColor()][i];
			new_zobrist_value ^= zobrist_switch_player;

			GoBlockId *die_id = tmp[1];
			for ( GoBlockId i=1; i<=die_id[0]; ++i ) {
				FOR_BLOCK_STONE(id, block_pool[die_id[i]],
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

