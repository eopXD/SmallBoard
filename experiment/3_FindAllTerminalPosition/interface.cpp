// Copyright (C) 2019 Yueh-Ting Chen (eopXD)
/*! \file FindAllTerminalPosition/interface.cpp
	\brief Interface for human assertion on this phase
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

void display2 ( uint32_t bitstring ) {		
	FOR_EACH_COORD(id) {
		putchar(bitstring&1 ? '1' : '0');
		bitstring >>= 1;
		if ( id%BORDER_C == BORDER_C-1 ) {
			putchar('\n');
		}
	}
}
void display5 ( uint64_t bitstring ) {
	for ( int i=0; i<26; ++i ) {
		cout << i << ": " << bitstring%5;
		bitstring /= 5;
	}
	cout << "================" << "\n";
}

/* GetAllKo() is the main thing we are doing in this phase */
uint32_t GetAllKo ( GoBoard &board ) {
	
	uint32_t ko_state = 0;
	
	//board.DisplayBoard();

	FOR_EACH_COORD(id) {
		GoCoordId ko_position = board.CheckKoPosition(id);
		if ( ko_position < 0 ) {
			continue;
		}
		ko_state |= 1ull << ko_position;
	} 
	//display(ko_state);
	return (ko_state);
}
uint32_t GetAllKo ( GoSerial const serial ) {
	GoBoard board(serial);
	return (GetAllKo(board));
}

#define CLEAR_SCREEN cout << "\033[H\033[J";

int main () 
{
/*
serial = 212254457379
-W-W-
WBWBW
B-B-B
WBWBW
-W-W-
serial = 212254634526
-W-W-
WBWBW
BBB-B
WBWBW
-W-W-
serial = 212256228849
-W-W-
WBWBW
BBBBB
WBWBW
-W-W-
*/

/*// Single serial number testing
	GoSerial serial = 180016049671ll;
	GoBoard board(serial);

	board.DisplayBoard();
	cout << "error code: " << (int)board.error_code << "\n";

	cout << "\n";
	uint32_t ko_state = GetAllKo(serial);
	display2(ko_state);

	board.CheckTerminates(ko_state);
	return (0);
*/

	char s[10][100];
	while ( 1 ) {
		CLEAR_SCREEN;
		cout << "Insert a 4x4 board 'B', 'W', '-'(dash)\n";
		for ( int i=0; i<4; ++i ) {
			cin >> s[i];
		}
		uint64_t serial = 0;
		for ( int i=3; i>=0; --i ) {
			if ( strlen(s[i]) != 4 ) {
				cout << "error: bad board size\n";
				exit(1);
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
					exit(1);
				}
			}
		}
		cout << "\n\nSerial: " << serial << "\n";
		GoBoard board(serial);
		if ( board.error_code < 0 ) {
			cout << (int)board.error_code << "\n";
			cout << "error: illegal board\n";
			exit(1);
		}
//		cout << "Display board:\n";
//		board.DisplayBoard(); cout << "\n";

		uint32_t ko_state = GetAllKo(serial);
		cout << "Ko State:\n";
		display2(ko_state); cout << "\n";
		
		board.CheckTerminates(ko_state);
		cout << "Press any key to continue...\n";
		getchar();
		getchar();

	}

	return (0);
}
