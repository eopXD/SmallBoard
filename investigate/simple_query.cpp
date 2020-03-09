/*! \file investigate/simple_query.cpp
    \brief simple query on whether if this serial is legal and what is its
     ko positions
*/


#include <algorithm>
#include <bitset>
#include <cstdio>
#include <iostream>

#include "GoBoard.h"

using namespace GoConstant;
using namespace GoFunction;
using namespace std;

void display(BIT_STATE bitstring)
{
    FOR_EACH_COORD(id)
    {
        putchar(bitstring & 1 ? '1' : '0');
        bitstring >>= 1;
        if (id % BORDER_C == BORDER_C - 1) {
            putchar('\n');
        }
    }
}

/* GetAllKo() is the main thing we are doing in this phase */
BIT_STATE GetAllKo(GoBoard &board)
{

    BIT_STATE ko_state = 0;

    // board.DisplayBoard();

    FOR_EACH_COORD(id)
    {
        GoCoordId ko_position = board.CheckKoPosition(id);
        if (ko_position < 0) {
            continue;
        }
        ko_state |= 1ull << ko_position;
    }
    // display(ko_state);
    return (ko_state);
}
BIT_STATE GetAllKo(GoSerial const serial)
{
    GoBoard board(serial);
    return (GetAllKo(board));
}
int main()
{


    GoSerial s;
    while (cin >> s) {
        GoBoard b(s);
        b.DisplayBoard();
        display(GetAllKo(s));
    }

    return (0);
}
