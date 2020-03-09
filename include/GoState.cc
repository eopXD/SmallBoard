#include <inttypes.h>
#include <string>

#include "GoState.h"
#include "comm.h"

#define L first
#define R second

using namespace std;
using namespace GoConstant;
using namespace GoBitState;

/************************ Constructor/Destructor *************************/
// declaration gives a empty bit state.
GoState::GoState()
{
#ifdef BITSTATE_ULL
    bit_state = 0;
#endif
#ifdef BITSTATE_BST
    bit_state.reset();
#endif
}
/******************************** Decode *********************************/
// the general decode on bit_state [l, r] both END inclusive
inline uint64_t GoState::general_decode(uint8_t l, uint8_t r)
{
#ifdef BITSTATE_ULL
    return ((bit_state >> l) & ((1ull << (r - l + 1)) - 1));
#endif
#ifdef BITSTATE_BST
    uint64_t fetch = 0;
    for (uint8_t i = r; i >= l; i--) {
        fetch = (fetch << 1) + bit_state[i];
    }
    return (fetch);
#endif
}
// constants of fetch defined in comm.h
// the following all utilizes general_fetch
GoSerial GoState::get_serial()
{
    return (
        general_decode(encode_lr[ENCODE_SERIAL].L, encode_lr[ENCODE_SERIAL].R));
}
void GoState::get_kopass(GoCoordId &ko_position, GoCoordId &pass_count)
{
    GoCoordId kopass =
        general_decode(encode_lr[ENCODE_SERIAL].L, encode_lr[ENCODE_SERIAL].R);
    if (kopass > SMALLBOARDSIZE) {
        ko_position = 0;
        pass_count = kopass - SMALLBOARDSIZE;
    } else {
        ko_position = kopass;
        pass_count = 0;
    }
}
GoCoordId GoState::get_outdeg()
{
    return (
        general_decode(encode_lr[ENCODE_OUTDEG].L, encode_lr[ENCODE_OUTDEG].R));
}
GoCoordId GoState::get_result()
{
    return (
        general_decode(encode_lr[ENCODE_RESULT].L, encode_lr[ENCODE_RESULT].R));
}
GoScore GoState::get_board_score()
{
    return (
        general_decode(encode_lr[ENCODE_SCORE].L, encode_lr[ENCODE_SCORE].R));
}
/******************************** Encode *********************************/
// the general encode on bit_state [l, r] both END inclusive
inline void GoState::general_encode(uint8_t l, uint8_t r, uint64_t value)
{
#ifdef BITSTATE_ULL
    bit_state &= ~(((1ull << (r - l + 1)) - 1) << l);
    bit_state |= value << l;
#endif
#ifdef BITSTATE_BST
    for (uint8_t i = l; i <= r; ++i) {
        bit_state.set(i, value & 1);
        value >>= 1;
    }
#endif
}
void GoState::set_serial(GoSerial serial)
{
    general_encode(encode_lr[ENCODE_SERIAL].L, encode_lr[ENCODE_SERIAL].R,
                   serial);
}
void GoState::set_kopass(GoCoordId ko_position, GoCoordId pass_count)
{
    GoCoordId kopass =
        (pass_count > 0) ? SMALLBOARDSIZE + pass_count : ko_position;
    general_encode(encode_lr[ENCODE_KOPASS].L, encode_lr[ENCODE_KOPASS].R,
                   kopass);
}
void GoState::set_outdef(GoCoordId outdeg)
{
    general_encode(encode_lr[ENCODE_OUTDEG].L, encode_lr[ENCODE_OUTDEG].R,
                   outdeg);
}
void GoState::set_result(GoCoordId result)
{
    general_encode(encode_lr[ENCODE_RESULT].L, encode_lr[ENCODE_RESULT].R,
                   result);
}
void GoState::set_board_score(GoScore board_score)
{
    general_encode(encode_lr[ENCODE_SCORE].L, encode_lr[ENCODE_SCORE].R,
                   board_score);
}
/******************************** Display ********************************/
string GoState::bitstring()
{
    string str;
#ifdef BITSTATE_ULL
    uint64_t tmp = bit_state;
    while (str.size() < ENCODE_LENGTH) {
        str += (char) ('0' + (tmp & 1));
        tmp >>= 1;
    }
#endif
#ifdef BITSTATE_BST
    for (int i = 0; i < ENCODE_LENGTH; ++i) {
        str += (char) ('0' + (int) bit_state[i]);
    }
#endif
    return (str);
}
#undef R
#undef L