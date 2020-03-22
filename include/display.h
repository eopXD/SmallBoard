/*! \file display.h
    \brief Linux terminal display interface and gameplay
*/

#ifndef SMALLBOARD_DISPLAY_H
#define SMALLBOARD_DISPLAY_H

#include <bitset>
#include <utility>
#include <iostream>
#include <vector>

#include "comm.h"
#include "GoStone.h"
#include "GoBlock.h"
#include "GoBoard.h"

struct History {
    GoCoordId move;
    GoCoordId ko;
    GoCoordId ate[5];
    History () {}
    History ( GoCoordId m, GoCoordId k, GoCoordId a[] ) {
        move = m;
        ko = k;
        ate[0] = a[0];
        for ( int i=1; i<=a[0]; ++i ) {
            ate[i] = a[i];
        }
    }
};

class GoBoardGui : public GoBoard
{
public:
    GoCoordId hl_id = GoConstant::COORD_UNSET; // current hl coord
    bool no_hl = 0; // option to close hl for pure display
    GoStoneColor first_ply = GoConstant::BlackStone;
    GoStoneColor ply_turn = GoConstant::BlackStone;
    
    int clr_line_sz = GoConstant::BORDER_R+4; // how many lines to clear
    /* gameplay history*/
    int hs_idx = 0, hs_size = 0;
    History hs[1003];

    GoBoardGui () : GoBoard() {}
    GoBoardGui ( GoSerial serial, bool initialize = 0 ) : GoBoard(serial, initialize) {}
    
public:
    void Turn();
    
    /* modify hl_id
     * if this direction is not available, then hl_id remains unchanged */
    void RecvArrow ( int dir ); 

    /* if prev/next is available in history */
    void PrevMove ();
    void NextMove ( GoCoordId target_id = -10 ); 

    void RefreshHL();
    /* high-lv to refresh current GoBoard display */
    void Refresh();

};
/* high-lv, call this to start everything */
extern void Play (GoSerial serial = GoConstant::MAX_SERIAL);

#endif
