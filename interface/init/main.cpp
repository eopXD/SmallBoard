// Copyright (C) 2019 Yueh-Ting Chen (eopXD)
/*! \file init/main.cpp
        \brief Test for functionality of SetStone and ResetStone
        \author Yueh-Ting Chen (eopXD)
        \project Efficient & Succinct Small Board Go
*/

#include <algorithm>
#include <cstdio>
#include <iostream>

#include "GoBoard.h"

using namespace GoConstant;
using namespace GoFunction;
using namespace std;

#define CLEAR_SCREEN cout << "\033[H\033[J";

void PressAnyKey()
{
    cout << "Press any key to continue...\n";
    getchar();
    getchar();
}

int main()
{
    GoBoard board(5039586ll);
    while (1) {
        CLEAR_SCREEN;
        cout << "Serial: " << board.GetSerial() << "\n";
        board.DisplayBoard();
        for (int blk_id = 0; blk_id < board.block_in_use; ++blk_id) {
            if (0 == board.block_pool[blk_id].in_use) {
                continue;
            }
            GoBlock &blk = board.block_pool[blk_id];
            cout << "BlockId: " << (int) blk_id << "\n";
            cout << "Color: " << (int)blk.color << "\n";
            cout << "Stone: " << (int) blk.CountStone() << "\n";
            blk.DisplayStone();
            cout << "Liberty: " << (int) blk.CountLiberty() << "\n";
            blk.DisplayLiberty();
            cout << "\n";
        }
        cout << "\n0: Reset Stone\n1: Set Stone\nChoose operation: ";
        int op;
        cin >> op;
        int target_id;
        int error = 0;
        if (op == 0) {
            cout << "Which stone to RESET? (0~" << BORDER_C * BORDER_R - 1
                 << ") ";
            cin >> target_id;
            error = board.ResetStone(target_id);
        } else if (op == 1) {
            cout << "Which stone to SET? (0~" << BORDER_C * BORDER_R - 1
                 << ") ";
            cin >> target_id;
            cout << "What color? (1: Black, 2: White) ";
            int stone_color;
            cin >> stone_color;
            error = board.SetStone(target_id, stone_color);
        } else {
            cout << "Unknown op\n\n";
            PressAnyKey();
        }

        cout << "Error code: " << error << "\n";
        cout << "Serial: " << board.GetSerial() << "\n";
        board.DisplayBoard();
        cout << "\n";
        PressAnyKey();
    }
    return (0);
}