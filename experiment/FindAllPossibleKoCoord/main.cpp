// Copyright (C) 2019 Yueh-Ting Chen (eopXD)
/*! \file FindAllPossibleKoCoord/main.cpp
	\brief Find all serial numbers that is legal. (no block on the board
	 shall have zero liberty.

	\author Yueh-Ting Chen (eopXD)
	\project Efficient & Succinct Small Board Go
*/


#include <cstdio>
#include <iostream>
#include <algorithm>

#include "GoBoard.h"

using namespace GoConstant;
using namespace GoFunction;
using namespace std;

void display ( BIT_STATE bitstring ) {		
	FOR_EACH_COORD(id) {
		putchar(bitstring&1 ? '1' : '0');
		bitstring >>= 1;
		if ( id%BORDER_C == BORDER_C-1 ) {
			putchar('\n');
		}
	}
}

/* GetAllKo() is the main thing we are doing in this phase */
uint32_t GetAllKo ( GoBoard &board ) {
	
	BIT_STATE ko_state = 0;
	
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
int main ()
{
/*
	GetAllKo(5664941825);
	GetAllKo(10023933850);
*/

// the real process to find all possible Ko coordinates
/* reading the data from previous phase */

/* BEWARE THE CONSTANT OF THIS BEFORE COMPILE AND EXECUTION */
/* BEWARE THE CONSTANT OF THIS BEFORE COMPILE AND EXECUTION */
/* BEWARE THE CONSTANT OF THIS BEFORE COMPILE AND EXECUTION */
/* BEWARE THE CONSTANT OF THIS BEFORE COMPILE AND EXECUTION */
	const GoSerial STATE_PER_FILE = (1ll<<10); // 2^32 = 2G
	const GoSerial NUMBER_OF_FILE = MAX_SERIAL/STATE_PER_FILE + 1;
// read buffer
	char read_file_path = "../FindAllPossibleSerial/";
	char filename[105];
	const int BUFFER_SIZE = 65536;
	unsigned char *buffer;
	buffer = new unsigned char [BUFFER_SIZE+5];
// write buffer
	char write_filename[105];
	uint32_t *write_buffer;
	write_buffer = new uint32_t [BUFFER_SIZE+5];
	int w_buf_idx;

	for ( GoSerial file_num=0; file_num<NUMBER_OF_FILE; ++file_num ) {
		sprintf(filename, "%sdata/data.SparseLegalState.part%05d", read_file_path, file_num);
		FILE *input_file = fopen(filename, "rb");

		sprintf(write_filename, "data/data.SparseKoBitState.part%05d", file_num);
		FILE *output_file = fopen(write_filename, "wb");

		GoSerial start_serial = STATE_PER_FILE*file_num;
		GoSerial end_serial = STATE_PER_FILE*(file_num+1ull);
		if ( MAX_SERIAL < end_serial ) {
			end_serial = MAX_SERIAL;
		}
		
		GoSerial serial = start_serial;
		while ( fread(buffer, BUFFER_SIZE, sizeof(unsigned char), input_file) ) {
			w_buf_idx = 0;
			for ( int buf_idx=0; buf_idx<BUFFER_SIZE; ++buf_idx ) {
				unsigned char compact = buffer[buf_idx];
				for ( int pos=7; pos>=0; pos-- ) {
					bool is_reduced_legal = compact&(1<<pos);
/* what you are going to do with the bit 'is_reduced_legal */
/* start */
					if ( is_reduced_legal ) {
						write_buffer[w_buf_idx++] = GetAllKo(serial);
					} else {
						write_buffer[w_buf_idx++] = 0;
					}
/* end */
					if ( w_buf_idx == BUFFER_SIZE ) {
						fwrite(write_buffer, sizeof(uint32_t), BUFFER_SIZE, output_file);	
						w_buf_idx = 0;
					}
					++serial;
					if ( serial == end_serial ) {
						break;
					}
				}
				if ( serial == end_serial ) {
					break;
				}
			}
			if ( serial == end_serial ) {
				break;
			}
			if ( w_buf_idx > 0 ) {
				fwrite(write_buffer, sizeof(unsigned char), BUFFER_SIZE, output_file);	
				w_buf_idx = 0;
			}
		}
		assert(serial == end_serial);
		fclose(input_file);
		fclose(output_file);
	}
	return (0);
}