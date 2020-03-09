// Copyright (C) 2019 Yueh-Ting Chen (eopXD)
/*! \file play/main.cpp
	\brief Test for gameplay
	\author Yueh-Ting Chen (eopXD)
	\project Efficient & Succinct Small Board Go
*/

#include <cstdio>
#include <iostream>
#include <algorithm>

#include <omp.h>

#include "GoBoard.h"

using namespace GoConstant;
using namespace GoFunction;
using namespace std;


#define CLEAR_SCREEN cout << "\033[H\033[J";

void PressAnyKey () {
	cout << "Press any key to continue...\n";
	getchar();
	getchar();
}

int main ()
{
	char s[10][100];

	while ( 1 ) {
		CLEAR_SCREEN;
		cout << "\n1: Input 4x4 board\n2: Input serial number\nInsert 1 or 2: ";
		int num; cin >> num;
		uint64_t serial;
		bool bad = 0;

		if ( num == 1 ) {
			cout << "Insert a 4x4 board 'B', 'W', '-'(dash)\n";
			for ( int i=0; i<4; ++i ) {
				cin >> s[i];
			}
			serial = 0;
			for ( int i=3; i>=0; --i ) {
				if ( strlen(s[i]) != 4 ) {
					cout << "error: bad board size\n";
					PressAnyKey();
					bad = 1;
				}
				for ( int j=3; j>=0; --j ) {
					serial *= 3;
					if ( s[i][j] == 'B' ) {
						serial += 1;
					} else if ( s[i][j] == 'W' ) {
						serial += 2;
					} else if ( s[i][j] == '-' ) {
						serial += 0;
					} else {
						cout << "error: illegal symbol\n";
						PressAnyKey();
						bad = 1;
					}
				}
			}
		} else if ( num == 2 ) {
			cout << "Maximum serial: " << MAX_SERIAL-1 << "\n";
			cout << "Insert serial number: ";
			cin >> serial;
		} else {
			cout << "Received unknonwn number\n";
			PressAnyKey();
			continue;
		}
		if ( bad ) {
			continue;
		}
		cout << "\n\nSerial: " << serial << "\n";
		GoBoard board(serial);
		board.SetTurn(BlackStone, WhiteStone);
		board.SetKo(COORD_UNSET);

		bool turn = 0;
		while ( 1 ) {
			if ( board.IsDoublePass() ) {
				break;
			}
			cout << "===============================\n";
			cout << "Current serial: " << board.GetSerial() << "\n";
			if ( board.GetKo() == -2 ) {
				cout << "Currently no ko\n";
			} else {
				cout << "Ko: " << (int)board.GetKo() << "\n";
			}
			cout << "Score: " << (int)board.CalcScore() << "\n";
			board.DisplayBoard();
			if ( turn == 0 ) {
				cout << "\nBlack's turn...\n";
			} else {
				cout << "\nWhite's turn...\n";
			}
			board.GetPossibleMove();
			cout << "Possible Moves: " << "\n";
			board.DisplayLegalMove();

			int id;
			while  ( 1 ) {
				cout << "\nInsert your move 'id' (0~15, -1 = Pass): ";
				cin >> id;
				if ( board.Move(id) < 0 ) {
					cout << "Error: Not a legal move\n";
				} else {
					break;
				}
			}

			if ( turn == 0 ) { cout << "Black "; } 
			else { cout << "White "; }
			cout << "moves at " << id << "\n";

			turn = !turn;
		}
		cout << "\n\nGameplay is ended by double pass\n\n";

		PressAnyKey();
	}

	return (0);
}