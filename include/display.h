/*! \file display.h
    \brief Linux terminal display interface and gameplay
*/

#ifndef SMALLBOARD_DISPLAY_H
#define SMALLBOARD_DISPLAY_H

#include <bitset>
#include <utility>

#include "comm.h"
#include "GoStone.h"
#include "GoBlock.h"
#include "GoBoard.h"

struct History {
    GoCoordId move;
    GoCoordId ko;
    GoCoordId ate[5];
    History () {}
    History ( GoCoordId m, GoCoordId k, GoCoordId *a = NULL ) {
        move = m;
        ko = k;
        if ( a != NULL ) {
            ate[0] = a[0];
            for ( int i=1; i<=a[0]; ++i ) {
                ate[i] = a[i];
            }
        }
    }
};

class GoBoardGui : public GoBoard
{
public:
    GoCoordId hl_id = GoConstant::COORD_UNSET; // current hl coord
    bool no_hl = 0; // option to close hl for pure display
    std::bitset<GoConstant::SMALLBOARDSIZE>
        available_hl; // availble hl coord
    GoStoneColor first_ply = GoConstant::BlackStone;
    GoStoneColor ply_turn = GoConstant::BlackStone;
    
    int clr_line_sz = GoConstant::BORDER_R+3; // how many lines to clear
    /* gameplay history*/
    int hs_idx = 0, hs_size = 0;
    History hs[1003];

    GoBoardGui () : GoBoard() {}
    GoBoardGui ( GoSerial serial ) : GoBoard(serial) {}
    
public:
    void Turn();
    
    /* modify hl_id
     * if this direction is not available, then hl_id remains unchanged */
    void RecvArrow ( int dir ); 

    /* if prev/next is available in history */
    void PrevMove ();
    void NextMove ( GoCoordId target_id = -1 ); 

};
/* high-lv, call this to start everything */
extern void Play (GoSerial serial = -1);

#endif