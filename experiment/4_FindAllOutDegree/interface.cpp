// Copyright (C) 2019 Yueh-Ting Chen (eopXD)
/*! \file FindAllOutDegree/interface.cpp
	\brief Gameplay for testing
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
		cout << "Insert a 4x4 board 'B', 'W', '-'(dash)\n";
		for ( int i=0; i<4; ++i ) {
			cin >> s[i];
		}
		uint64_t serial = 0;
		bool bad = 0;
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
			cout << "Ko: " << (int)board.GetKo() << "\n";
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