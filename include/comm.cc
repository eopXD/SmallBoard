// Copyright (C) 2019 Yueh-Ting Chen (eopXD)
/*! \file comm.cc
	\brief Implementations of comm.h

	\author Yueh-Ting Chen (eopXD)
	\project Efficient & Succinct Small Board Go
*/

#include "comm.h"

using namespace std;
using namespace GoComm;

uint64_t g_zobrist_player_hash_weight[3];
uint64_t g_zobrist_board_hash_weight[3][SMALLBOARDSIZE];
uint64_t g_zobrist_ko_hash_weight[SMALLBOARDSIZE];

namespace GoFunction {


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
		g_zobrist_player_hash_weight[i] = (uint64_t) rand_r(&seed) << 32 | rand_r(&seed);
		for ( int j=0; j<SMALLBOARDSIZE; ++j ) {
			g_zobrist_board_hash_weight[i][j] = (uint64_t) rand_r(&seed) << 32 | rand_r(&seed);
		}
	}
	for ( int i=0; i<SMALLBOARDSIZE; ++i ) {
		g_zobrist_ko_hash_weight[i] = (uint64_t) rand_r(&seed) << 32 | rand_r(&seed);
	}
}

} // namespace GoFunction

