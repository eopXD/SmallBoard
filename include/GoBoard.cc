#include "comm.h"
#include "GoBoard.h"

using namespace GoConstant;
using namespace GoFunction;
using namespace std;

GoBoard::GoBoard () {
	CreateGlobalVariable();

	FOR_EACH_COORD(id) {
		stones[id].self_id = id;
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

GoBoard::~GoBoard () {};


void GoBoard::CopyFrom ( const GoBoard &src ) {
	*this = src;
	FixBlockInfo();
}

void GoBoard::FixBlockInfo () {
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
		HandOff();
		GetPossibleMove();
		record_zobrist[(game_length-1+4)&3] = current_zobrist_value;
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
		if ( SelfColor() == nb_blk.color ) {
			blk.MergeBlocks(nb_blk);
			RecycleBlock(nb_id[i]);
		}
	}
	visited_position[blk_id] = game_length;
	for ( GoBlockId i=1; i<=die_id[0]; ++i ) {
		if ( (1 == block_pool[die_id[i]].stone_count) and (1 == blk.stone_count) 
		 and (1 == blk.CountLiberty()) ) {
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

	HandOff();
	GetPossibleMove();
	record_zobrist[(game_length-1+4)&3] = current_zobrist_value;
	return (0);

}
GoError GoBoard::Move ( const GoCoordId x, const GoCoordId y ) {
	return (Move(CoordToId(x, y)));
}

void GoBoard::DisplayBoard () {
	FOR_EACH_COORD(id) {
		if ( id and ((id%BORDER_C)==0) ) {
			putchar('\n');
		}
		
		putchar(COLOR_CHAR[board_state[id]]);
	} putchar('\n');
}

// get the minimal representation serial representation of this board
GoSerial GoBoard::GetMinimal () {
	GoSerial current = serial;
	for ( int i=0; i<4; ++i ) {
		RotateClockwise();
		for ( int j=0; j<2; ++j ) {
			FlipLR();
			if ( GetSerial() < current ) {
				return (serial);
			}
		}
	}
	return (current);
}


// get serial number of this->board_state[][], also cache it into this->serial
GoSerial GoBoard::GetSerial () {
	serial = 0;
	for ( GoCoordId id=SMALLBOARDSIZE-1; id>=0; id-- ) {
		serial = serial*3 + board_state[id];
	}
	return (serial);
}

// rotate clock-wise (90 degree)
void GoBoard::RotateClockwise () {
	GoStoneColor tmp[SMALLBOARDSIZE];
	FOR_EACH_COORD(id) {
		GoCoordId x, y;
		IdToCoord(id, x, y);	
		tmp[CoordToId(y, BORDER_C-1-x)] = board_state[id];
	}
	
	memcpy(board_state, tmp, sizeof(tmp));
}

// flip in a LR symmetric matter
void GoBoard::FlipLR () {
	for ( GoCoordId x=0; x<BORDER_R; ++x ) {
		for ( GoCoordId y=0; y<BORDER_C/2; ++y ) {
			swap(board_state[CoordToId(x, y)], board_state[CoordToId(x, BORDER_C-1-y)]);
		}
	}
}

// Uses SetStone to setup the board, 'initialze' can be set to 1 if want
// the board details to be initialized for future play. 
// error code may be set
//   0: success
//	-1: construct fail
GoBoard::GoBoard ( GoSerial _serial, bool initialize ) : GoBoard() {
	serial = _serial;
	FOR_EACH_COORD(id) {
		GoStoneColor stone_color = _serial%3;
		_serial /= 3;
		if ( stone_color == EmptyStone ) {
		// don't need to set stone for empty stone.
			continue;
		}
		GoError err = SetStone(id, stone_color);
		if ( err != 0 ) { // see error code below
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
GoError GoBoard::SetStone ( const GoCoordId id, const GoStoneColor stone_color ) {
// nb_id[0] is counter, 
	GoBlockId blk_id, nb_id[5];
	
	GetNewBlock(blk_id);
	GoBlock &blk = block_pool[blk_id];


// opponent stone eaten when the move occurs
	//blk.Reset();
	blk.stone_count = 1;
	blk.color = stone_color;

	GetNeighborBlocks(blk, id, nb_id);
// check if we are killing anybody
	for ( GoBlockId	i=1; i<=nb_id[0]; ++i ) {
		GoBlock &nb_blk = block_pool[nb_id[i]];
		if ( stone_color != nb_blk.color ) {
			if ( 1 == nb_blk.CountLiberty() ) {
			// eat-able!
				return (-2);
			}
		} else {
			blk.TryMergeBlocks(nb_blk);
		}
	}

	blk.ResetLiberty(id);

	if ( 0 == blk.CountLiberty() ) { 
	// self-eat!
		return (-1);
	}

	stones[id].Reset(blk_id);
	blk.in_use = true;
	blk.head = blk.tail = id;
	for ( GoBlockId i=1; i<=nb_id[0]; ++i ) {
		GoBlock &nb_blk = block_pool[nb_id[i]];
		nb_blk.ResetLiberty(id);
		if ( stone_color == nb_blk.color ) {
			blk.MergeBlocks(nb_blk);

			RecycleBlock(nb_id[i]);
		}
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

/* For checking all possible Ko position of a serial numebr */ 

// call this function only when the GoBoard is constructed by GoSerial
// return value
// 			-1: no possible Ko for this stone's neighbor
//	 GoCoordId: the GoCoordId of the 
GoCoordId GoBoard::CheckKoPosition ( const GoCoordId id, 
 const GoStoneColor opponent_color ) {
	GoBlockId blk_id = GetBlockIdByCoord(id);
	if ( blk_id == BLOCK_UNSET ) {
		return (-1);
	}
	GoBlock &blk = block_pool[blk_id];
	
	if ( blk.color!= opponent_color or blk.CountStone()!=1 or blk.CountLiberty()!=1 ) {
		return (-1);
	}
	GoCoordId eat_me = blk.FirstLiberty();
// get all neighboring BlockId of 'eat_me'
	int nb_id[5];
	nb_id[0] = 0;
	FOR_NEIGHBOR(eat_me, nb) {
		if ( board_state[*nb] != EmptyStone ) {
			nb_id[++nb_id[0]] = GetBlockIdByCoord(*nb);
		}
	}
	if ( nb_id[0] != cached_neighbor_size[eat_me] ) {
	// if it is not surrounded, it is impossible to be a Ko position
		return (-1);
	}
// if all surrounded by white stone, then the liberty position is a potential Ko
// for the black stone.
	for ( GoBlockId i=1; i<=nb_id[0]; ++i ) {
		GoBlock &nb_blk = block_pool[nb_id[i]];
		if ( nb_blk.color != opponent_color ) {
			return (-1);
		}
	}
	return (eat_me);
}

/* for checking whether a state is a terminal state */

// Encoding for CheckTerminate phase
// 0: NULL Value (illegal board, or this ko-position is not)
// 1: Not terminate
// 2: Win
// 3: Lose
// 4: Draw
uint8_t GoBoard::CheckTerminate ( bool black_no_move, bool white_no_move ) {
	if ( board_score>0 and white_no_move ) {
		return (TERMINATE_WIN);
	} else if ( board_score<0 and black_no_move ) {
		return (TERMINATE_LOSE);
	} else if ( black_no_move and white_no_move ) {
		if ( board_score>0 ) {
			return (TERMINATE_WIN);
		} else if ( board_score == 0 ) {
			return (TERMINATE_DRAW);
		} else {
			return (TERMINATE_LOSE);
		}
	} else {
		return (NOT_TERMINATE);
	}
}

// NOTE: assume black as the current player
// return 64-bit with encoded as a polynomial of 5
// for encode checkout CheckTerminate
uint64_t GoBoard::CheckTerminates ( const uint32_t ko_state ) {
	
// black_no_move[SMALLBOARDSIZE] is when assuming no ko on board
	bool black_no_move[SMALLBOARDSIZE+1]; 		

// NOTICE the assumption on evaluating on all boards in black's turn first
	current_player = BlackStone;
	opponent_player = WhiteStone;

// check current board score
	CalcScore();
	cerr << "score: " << (int)board_score << "\n";

// check possible move for BLACK
	ko_position = COORD_UNSET;
	GetPossibleMove(); // moves of BLACK
	uint8_t black_move_num = __builtin_popcountll(legal_move_map.to_ullong());
	cerr << "BLACK legal move: " << (int) black_move_num << "\n";
	for ( GoCoordId r=0; r<BORDER_R; ++r ) {
		for ( GoCoordId c=0; c<BORDER_C; ++c ) {
			cerr << (int)legal_move_map[r*BORDER_C+c];
		}
		cerr << "\n";
	}
	

/******************************************************/
// DELETE THIS AFTER SOLVING BUG IN GetPossibleMove
	return 0;
/******************************************************/

	FOR_EACH_COORD(i) {
		if ( black_move_num == 0 ) {
			black_no_move[i] = true;
			continue;
		}

		bool can_be_ko = (ko_state>>i)&1;
		if ( can_be_ko and black_move_num == 1 and legal_move_map[i] ) {
			black_no_move[i] = true;
		} else {
			black_no_move[i] = false;
		}
	}
	black_no_move[SMALLBOARDSIZE] = (black_move_num==0);
	cerr << "black_no_move: " << "\n";
	cerr << "no ko: " << (int)black_no_move[SMALLBOARDSIZE] << "\n";
	for ( GoCoordId r=0; r<BORDER_R; ++r ) {
		for ( GoCoordId c=0; c<BORDER_C; ++c ) {
			cerr << (int)black_no_move[r*BORDER_C+c];
		}
		cerr << "\n";
	}

// check possible move for WHITE (white don't need to deal with ko)
	HandOff();
	GetPossibleMove(); // moves of WHITE
	uint8_t white_move_num = __builtin_popcountll(legal_move_map.to_ullong());
	cerr << "WHITE legal move: " << (int) white_move_num << "\n";
	for ( GoCoordId r=0; r<BORDER_R; ++r ) {
		for ( GoCoordId c=0; c<BORDER_C; ++c ) {
			cerr << (int)legal_move_map[r*BORDER_C+c];
		}
		cerr << "\n";
	}
	
	bool white_no_move = (white_move_num==0);
	cerr << "white_no_move: " << (int)white_no_move << "\n";
	
// result for return
	uint64_t result = 0;
	result = result*5 + CheckTerminate(black_no_move[SMALLBOARDSIZE], white_no_move);

	// for possible ko positions...
	REV_FOR_EACH_COORD(i) {
		bool can_be_ko = (ko_state>>i)&1;
		if ( can_be_ko ) {
			CheckTerminate(black_no_move[i], white_no_move);
		} else {
			result *= 5; // NULL value
		}
	}
	return (result);
}

// NOTE: this function cannot calculate non-terminal board score correctly
// NOTE: only for calculation on terminal positions
// NOTE: assume black is the current player
// returns current board score difference (BLACK-WHITE)
// result also cached in '.board_score' variable
// > 0: WIN  for current player
// < 0: LOSE for current player 
// = 0: DRAW for current player
GoCoordId GoBoard::CalcScore ( GoStoneColor opponent_color ) {
	board_score = 0;
	FOR_EACH_COORD(i) {
		if ( board_state[i] != EmptyStone ) {
			if ( board_state[i] == opponent_color ) {
				board_score--;
			} else {
				++board_score;
			}
		} else {
			bool no_empty_neighbor = 1; // assume having no empty neighbor
			bool friendly_neighbor = 0;
			bool opponent_neighbor = 0;
			FOR_NEIGHBOR(i, nb) {
				if ( board_state[*nb] == EmptyStone ) {
					no_empty_neighbor = 0;
					break;
				} else {
					if ( board_state[*nb] == opponent_color ) {
						opponent_neighbor = 1;
					} else {
						friendly_neighbor = 1;
					}
				}
			}
			if ( no_empty_neighbor ) {
				if ( friendly_neighbor and !opponent_neighbor ) {
					++board_score;
				}
				if ( opponent_neighbor and !friendly_neighbor ) {
					--board_score;
				}
			}
		}
	}
	return (board_score);
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
	swap(current_player, opponent_player);
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
GoBlockId GoBoard::GetBlockIdByCoord ( const GoCoordId id ) {
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
	blk.SetStone(target_id);
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
GoError GoBoard::TryMove ( GoBlock &blk, const GoCoordId target_id, 
 GoBlockId *nb_id, GoBlockId* die_id, GoCoordId max_lib ) {
 	if ( !legal_move_map[target_id] ) {
 		return (-1);
 	}
 	GoCounter cnt = 0;
 	
 	blk.stone_count = 1;
 	blk.color = SelfColor();

 	GetNeighborBlocks(blk, target_id, nb_id);
 	cerr << "Initial Block:\n";
 	cerr << "Stone\n";
 	blk.DisplayStone();
 	cerr << "Liberty\n";
 	blk.DisplayLiberty();
 	cerr << "TryMove::nb_id[0]: " << (int)nb_id[0] << "\n";
 	for ( GoBlockId i=1; i<=nb_id[0]; ++i ) { // checked stone/liberty correct
 		cerr << "nb[" << (int)i << "]: " << (int)nb_id[i] << "\n";
 		GoBlock &nb_blk = block_pool[nb_id[i]];
 		cerr << "Stone\n";
 		nb_blk.DisplayStone();
 		cerr << "Liberty\n";
 		nb_blk.DisplayLiberty();
 	}
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
 			cerr << "merge!\n";
 			blk.TryMergeBlocks(nb_blk);
	 		cerr << "Stone: \n";
 			blk.DisplayStone();
 			cerr << "Liberty: \n";
 			blk.DisplayLiberty();
 			cerr << "===============\n";
 		}
 	}

 	blk.ResetLiberty(target_id);
 	if ( blk.CountLiberty() == 0 ) { // self-eat move
 		return (-1);
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
		blk_id = block_in_use++;
	}
	block_pool[blk_id].Reset();
}

// get all possible move for the current board position
void GoBoard::GetPossibleMove () {
	GoBlockId blk_id;
// [0][?] is neighbor block_id, [1][?] is block_id killed
// [?][0] is the counter, from [?][1]~[?][4] stores the value
	GoBlockId tmp[2][5];  
	
	bool have_empty_neighbor;
	legal_move_map.reset();

	FOR_EACH_COORD(id) {
		if ( EmptyStone != board_state[id] ) { // can't put stone on non-empty
			continue;
		}
		if ( ko_position == id ) { // can not put stone on ko
			continue;
		}
		GetNewBlock(blk_id);
		GoBlock &blk = block_pool[blk_id];

		cerr << (int) id << " " << (int)cached_neighbor_size[id] << "\n";
		have_empty_neighbor = 0;
		FOR_NEIGHBOR(id, nb) {
			cerr << (int)(id) << ": " << (int)(*nb) << "\n"; 
			if ( EmptyStone == board_state[*nb] ) {
				have_empty_neighbor = 1;
				//break;
			}
		} cerr << "\n";
		cerr << "\nempty neighbor: " << have_empty_neighbor << "\n";
		legal_move_map.set(id);
		if ( have_empty_neighbor ) {
			RecycleBlock(blk_id);
			continue;
		}
		// no empty neighbor
		TryMove(blk, id, tmp[0], tmp[1]);
		blk.CountLiberty();
		cerr << "block liberty: " << blk.liberty_count << "\n";
		if ( blk.liberty_count <= 0 ) {
			legal_move_map.reset(id);
			RecycleBlock(blk_id);
			continue;
		}
	// check for basic Ko
		if ( game_length > 2 ) {
			GoHash new_zobrist_value = current_zobrist_value;
			
			new_zobrist_value ^= zobrist_board_hash_weight[SelfColor()][id];
			new_zobrist_value ^= zobrist_switch_player;

			GoBlockId *die_id = tmp[1];
			for ( GoBlockId i=1; i<=die_id[0]; ++i ) {
				FOR_BLOCK_STONE(j, block_pool[die_id[i]],
					new_zobrist_value ^= zobrist_board_hash_weight[OpponentColor()][j];
				);
			}

			if ( record_zobrist[(game_length-1+4)&3] == record_zobrist[(game_length-3+4)&3] ) {
				legal_move_map.reset(id);
			}
		}
		RecycleBlock(blk_id);
	}
}

