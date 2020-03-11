/*! \file play/main.cpp
    \brief Test for gameplay
*/

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>

#include <omp.h>

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
    ofstream inlog;

    inlog.open("input.log");
    char s[10][100];

    while (1) {
        CLEAR_SCREEN;
        cout << "\n1: Input 4x4 board\n2: Input serial number\nInsert 1 or 2: ";
        int num;
        cin >> num;
        inlog << num << endl;
        uint64_t serial;
        bool bad = 0;

        if (num == 1) {
            cout << "Insert a 4x4 board 'B', 'W', '-'(dash)\n";
            for (int i = 0; i < 4; ++i) {
                cin >> s[i];
                inlog << s[i] << endl;
            }
            serial = 0;
            for (int i = 3; i >= 0; --i) {
                if (strlen(s[i]) != 4) {
                    cout << "error: bad board size\n";
                    PressAnyKey();
                    bad = 1;
                }
                for (int j = 3; j >= 0; --j) {
                    serial *= 3;
                    if (s[i][j] == 'B') {
                        serial += 1;
                    } else if (s[i][j] == 'W') {
                        serial += 2;
                    } else if (s[i][j] == '-') {
                        serial += 0;
                    } else {
                        cout << "error: illegal symbol\n";
                        PressAnyKey();
                        bad = 1;
                    }
                }
            }
        } else if (num == 2) {
            cout << "Maximum serial: " << MAX_SERIAL - 1 << "\n";
            cout << "Insert serial number: ";
            cin >> serial;
            inlog << serial << endl;
        } else {
            cout << "Received unknonwn number\n";
            PressAnyKey();
            continue;
        }
        if (bad) {
            continue;
        }
        cout << "\n\nSerial: " << serial << "\n";
        GoBoard board(serial);
        board.SetTurn(BlackStone, WhiteStone);
        board.SetKo(COORD_UNSET);

        bool turn = 0;
        while (1) {
            if (board.IsDoublePass()) {
                break;
            }
            cout << "===============================\n";
            cout << "Current serial: " << board.GetSerial() << "\n";
            cout << "Current player: " << COLOR_STRING[board.current_player]
                 << "\n";
            if (board.GetKo() == -2) {
                cout << "Currently no ko\n";
            } else {
                cout << "Ko: " << (int) board.GetKo() << "\n";
            }
            cout << "Score: " << (int) board.CalcScore() << "\n";
            board.DisplayBoard();

            if (turn == 0) {
                cout << "\nBlack's turn...\n";
            } else {
                cout << "\nWhite's turn...\n";
            }
            board.GetPossibleMove();
            cout << "Possible Moves: "
                 << "\n";
            board.DisplayLegalMove();
            board.DisplayGoBlock();
            int id;
            while (1) {
                cout
                    << "\nInsert your move 'id' (0~15, -1 = Pass, -2 = Undo): ";
                cin >> id;
                inlog << id << endl;
                if (id == -2) {
                    if (board.UndoMove(board.previous_move, board.previous_ko,
                                       board.prev_eat_from) < 0) {
                        cout << "Error: Unsuccessful undo\n";
                    } else {
                        cout << "Undo previous move\n";
                    }
                    break;
                } else if (board.Move(id) < 0) {
                    cout << "Error: " << id << " is not a legal move\n";
                } else {
                    if (turn == 0) {
                        cout << "Black ";
                    } else {
                        cout << "White ";
                    }
                    cout << "moves at " << id << "\n";
                    break;
                }
            }
            turn = !turn;
        }
        cout << "\n\nGameplay is ended by double pass\n\n";
//        break;
        PressAnyKey();
    }
    inlog.close();
    return (0);
}