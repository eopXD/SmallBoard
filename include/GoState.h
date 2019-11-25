// Copyright (C) 2019 Yueh-Ting Chen (eopXD)
/*! \file GoState.hpp
	\brief GoState interacts with the disk. In other words, values are stored
	 in the most compact form. (encoding by GoBitState::CreateEncoding) Then 
	 extracted to this class, and then interpretted into play-able boards,
	 which is GoBoard.

	\author Yueh-Ting Chen (eopXD)
	\project Efficient & Succinct Small Board Go
*/

#ifndef SMALLBOARD_GOSTATE_H
#define SMALLBOARD_GOSTATE_H

#include <bitset>

#include "comm.h"

struct GoState {
	std::bitset<GoConstant::SMALLBOARDSIZE> bit_state;

	GoState () { // declaration gives a empty bit state.
		bit_state.reset();
	}
	GoState ( GoSerial serial, GoCoordId ko_position=0, 
	 GoCoordId pass_count=0 ) {
	}

/******************************** Decode *********************************/
// the general fetch on bit_state [l, r] both end inclusive
	uint64_t general_decode ( uint8_t l, uint8_t r ) {
		/* to be decisioned after experiment */
	}
// constants of fetch defined in comm.h
// the following all utilizes general_fetch
	GoSerial get_serial ();
	void get_kopass ( GoCoordId &ko_position, GoCoordId &pass_count );
	GoCoordId get_outdeg (); 
	GoCoordId get_result ();
	GoCoordId get_board_score ();
/******************************** Encode *********************************/
	void general_encode ( uint8_t l, uint8_t r, uint64_t value );
	void set_serial ( GoSerial serial );
	void set_kopass ( GoCoordId ko_position, GoCoordId pass_count );
	void set_outdef ( GoCoordId outdeg );
	void set_result ( GoCoordId result );
	void set_board_score ( GoCoordId board_score );

}
#endif