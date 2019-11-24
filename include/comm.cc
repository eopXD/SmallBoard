// Copyright (C) 2019 Yueh-Ting Chen (eopXD)
/*! \file comm.cc
	\brief Implementations of comm.h

	\author Yueh-Ting Chen (eopXD)
	\project Efficient & Succinct Small Board Go
*/

#include "comm.h"

#define x first
#define y second

using namespace std;
using namespace GoConstant;

namespace GoBitState {


vector<uint8_t> encode_size; // needed bits generated at GenNeededBit
vector<pair<uint8_t, uint8_t>> encode_lr; // pair(start-bit, end-bit)

once_flag CreateEncoding_once;
void CreateEncoding {
	call_once(
		CreateEncoding_once,
		[] () {
			int start_bit = 63; // Most Significant Bit

			CreateEncode(pow(3.0, SMALLBOARDSIZE), start_bit);	// serial
			CreateEncode(SMALLBOARDSIZE+3, start_bit);			// ko_and_pass
			CreateEncode(SMALLBOARDSIZE+1, start_bit);			// out_degree
			CreateEncode(4, start_bit);							// game_result
			CreateEncode(2*SMALLBOARDSIZE+2, start_bit);		// board_score

			if ( start_bit < -1 ) {
				elog("64 bit is not enough to encode the given ROW & COL\n");
				exit(1);
			}
		}
	);
}
void CreateEncode ( uint64_t diff_value, int &start_bit ) {
	encode_size.push_back( ceil(log(diff_value)/log(2)) );
	encode_lr.push_back(make_pair(start_bit - encode_size.back() + 1, start_bit));
	start_bit -= encode_size.back();
}

} // namespace GoBitState

uint64_t g_zobrist_player_hash_weight[3];
uint64_t g_zobrist_board_hash_weight[3][SMALLBOARDSIZE];
uint64_t g_zobrist_ko_hash_weight[SMALLBOARDSIZE];

uint8_t cached_neighbor_size[SMALLBOARDSIZE];
uint64_t cached_neighbor_id[SMALLBOARDSIZE][4];
vector<GoPosition> cached_neighbor_coord[BORDER_R][BORDER_C];

namespace GoFunction {

bool InBoard ( const GoCoordId id ) {
	return (0<=id and id<SMALLBOARDSIZE);
}
bool InBoard ( const GoCoordId x, const GoCoordId y ) {
	return (0<=x and x<BORDER_R and 0<=y and y<BORDER_C);
}

bool IsPass ( const GoCoordId id ) {
	return (COORD_PASS == id);
}
bool IsPass ( const GoCoordId x, const GoCoordId y ) {
	return (COORD_PASS == CoordtoId(x, y));
}
bool IsUnset ( const GoCoordId id ) {
	return (COORD_UNSET == id);
}
bool IsUnset ( const GoCoordId x, const GoCoordId y ) {
	return (COORD_UNSET == CoordtoId(x, y));
}
bool IsResign ( const GoCoordId id ) {
	return (COORD_RESIGN == id);
}
bool IsResign ( const GoCoordId x, const GoCoordId y ) {
	return (COORD_RESIGN == CoordtoId(x, y));
}


void IdToCoord ( const GoCoordId id, GoCoordId &x, GoCoordId &y ) {
	if ( COORD_PASS == id ) {
		x = y = COORD_PASS;
	} 
	else if ( COORD_RESIGN == id ) {
		x = y = COORD_RESIGN;
	}
	else if ( !InBoard(x, y) ) {
		x = y = COORD_UNSET;
	}
	else {
		x = id / BORDER_C;
		y = id % BORDER_C;
	}
}
GoCoordId CoordToId ( const GoCoordId x, const GoCoordId y ) {
	if ( COORD_PASS == x and COORD_PASS == y ) {
		return (COORD_PASS);
	}
	if ( COORD_RESIGN == x and COORD_RESIGN == y ) {
		return (COORD_RESIGN);
	}
	if ( !InBoard(x, y) ) {
		return (COORD_UNSET);
	}
	return (x*BORDER_C + y);
}

once_flag CreateGlobalVariable_once;
void CreateGlobalVariable () {
	call_once(
		CreateGlobalVariable_once,
		[] () {
			CreateNeighborCache();
			CreateZobristHash();
		}
	);
}

#if defined(_WIN32) || defined(_WIN64)
static int rand_r(unsigned int *seed)
{
    unsigned int next = *seed;
    int result;

    next *= 1103515245;
    next += 12345;
    result = (unsigned int)(next / 65536) % 2048;

    next *= 1103515245;
    next += 12345;
    result <<= 10;
    result ^= (unsigned int)(next / 65536) % 1024;

    next *= 1103515245;
    next += 12345;
    result <<= 10;
    result ^= (unsigned int)(next / 65536) % 1024;

    *seed = next;

    return result;
}
#endif

void CreateZobristHash () {
	uint32_t seed = 0xdeadbeef;
	for ( int i=0; i<3; ++i ) {
		g_zobrist_player_hash_weight[i] = (uint64_t) rand_r(&seed) << 32 
		 | rand_r(&seed);
		for ( int j=0; j<SMALLBOARDSIZE; ++j ) {
			g_zobrist_board_hash_weight[i][j] = (uint64_t) rand_r(&seed) << 32 
			 | rand_r(&seed);
		}
	}
	for ( int i=0; i<SMALLBOARDSIZE; ++i ) {
		g_zobrist_ko_hash_weight[i] = (uint64_t) rand_r(&seed) << 32 
		 | rand_r(&seed);
	}
}

void CreateNeighborCache () {
	for ( GoCoordId x=0; x<BORDER_R; ++x ) {
		for ( GoCoordId y=0; y<BORDER_C; ++y ) {
			GoCoordId id = CoordToId(x, y);
			
			cached_neighbor_coord[x][y].clear();
			for ( int i=0; i<DELTA_SIZE; ++i ) {
				cached_neighbor_id[id][i] = COORD_UNSET;
			}

			for ( int i=0; i<DELTA_SIZE; ++i ) {
				GoCoordId xx = x+COORD_DX[i];
				GoCoordId yy = y+COORD_DY[i];
				if ( !InBoard(xx, yy) ) {
					continue;
				}
				cached_neighbor_coord[x][y].push_back(GoPosition(xx, yy));
			}
			cached_neighbor_size[id] = cached_neighbor_coord[x][y].size();
			for ( int i=0; i<cached_neighbor_size[id]; ++i ) {
				cached_neighbor_id[id][i] = CoordToId(
				 cached_neighbor_coord[x][y][i].x, 
				 cached_neighbor_coord[x][y][i].y);
			}
		}
	}
}


} // namespace GoFunction

#undef y
#undef x
