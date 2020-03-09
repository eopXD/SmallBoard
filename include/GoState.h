/*! \file GoState.h
    \brief Interacts with the disk. In other words, values are stored
     in the most compact form. (encoding by GoBitState::CreateEncoding) Then
     extracted to this class, and then interpretted into play-able boards,
     which is GoBoard.
*/

#ifndef SMALLBOARD_GOSTATE_H
#define SMALLBOARD_GOSTATE_H

#include <bitset>
#include <string>

#include "comm.h"

struct GoState {

#ifdef BITSTATE_ULL
    uint64_t bit_state;
#endif

// 5x5: ENCODE_LENGTH = 58 (specified in makefile)
#ifdef BITSTATE_BST
    std::bitset<ENCODE_LENGTH> bit_state;
#endif
    /************************ Constructor/Destructor *************************/
    // declaration gives a empty bit state.
    GoState();
    /******************************** Decode *********************************/
    // the general fetch on bit_state [l, r] both end inclusive
    uint64_t general_decode(uint8_t l, uint8_t r);
    // constants of fetch defined in comm.h
    // the following all utilizes general_fetch
    GoSerial get_serial();
    void get_kopass(GoCoordId &ko_position, GoCoordId &pass_count);
    GoCoordId get_outdeg();
    GoCoordId get_result();
    GoScore get_board_score();
    /******************************** Encode *********************************/
    void general_encode(uint8_t l, uint8_t r, uint64_t value);
    void set_serial(GoSerial serial);
    void set_kopass(GoCoordId ko_position, GoCoordId pass_count);
    void set_outdef(GoCoordId outdeg);
    void set_result(GoCoordId result);
    void set_board_score(GoScore board_score);
    /******************************** Display ********************************/
    // this is only for debug purpose, to see whether the bit operations
    // performs as expected. This function is not used in any stage.
    // so don't be picky about its implementation, because I know that
    // std::string is slow as fuck.
    std::string bitstring();
};
#endif