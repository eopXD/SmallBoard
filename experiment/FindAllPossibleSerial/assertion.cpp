// Copyright (C) 2019 Yueh-Ting Chen (eopXD)
/*! \file FindAllPossibleState/assertion.cpp
	\brief Assert that the states are saved correctly.

	\author Yueh-Ting Chen (eopXD)
	\project Efficient & Succinct Small Board Go
*/

#include <cstdio>
#include <cassert>
#include <iostream>
#include <bitset>

#include "GoBoard.h"

using namespace GoConstant;
using namespace GoFunction;
using namespace std;

int main ()
{


// the real process to select out the legal boards
	const GoSerial STATE_PER_FILE = 2147483648ll * 8; // 2^32 = 2G
	const GoSerial NUMBER_OF_FILE = MAX_SERIAL/STATE_PER_FILE + 1;

	char filename[105];
// buffer
	const int BUFFER_SIZE = 65536;
	unsigned char *buffer;
	buffer = new unsigned char [BUFFER_SIZE+5];
	
	for ( int file_num=0; file_num<NUMBER_OF_FILE; ++file_num ) {
		sprintf(filename, "data.SparseLegalState.part%03d", file_num);
		FILE *input_file = fopen(filename, "rb");

		GoSerial start_serial = file_num * STATE_PER_FILE;
		GoSerial end_serial = STATE_PER_FILE*(file_num+1ull);
		if ( MAX_SERIAL < end_serial ) {
			end_serial = MAX_SERIAL;
		}
		
		//printf("start: %lld %lld\n", start_serial, end_serial);
		GoSerial serial = start_serial;
		
		while ( fread(buffer, BUFFER_SIZE, sizeof(unsigned char), input_file) ) {
			for ( int buf_idx=0; buf_idx<BUFFER_SIZE; ++buf_idx ) {
				unsigned char compact = buffer[buf_idx];
				for ( int pos=7; pos>=0; pos-- ) {
					bool is_reduced_legal = compact&(1<<pos);
					bool check = 1;
					GoBoard board(serial);
					//board.DisplayBoard();
					if ( board.error_code != 0 ) {
						check = 0;
					} else {
						for ( int i=0; i<4; ++i ) { // rotate 4 times
							board.RotateClockwise();
							for ( int j=0; j<2; ++j ) { // flip 2 times
								if ( board.GetSerial() < serial ) {
									check = 0;
									break;
								}
							}
							if ( !check ) {
								break;
							}
						}
					}
					//printf("serial: %lld, (%d, %d)\n", serial, is_reduced_legal, check);
					//getchar();
					assert(is_reduced_legal == check);
					++serial;
					if ( serial == end_serial ) {
						break;
					}
				}
				if ( serial == end_serial ) {
					break;
				}
			}

		}
		//printf("the end: %lld %lld\n", serial, end_serial);
		assert(serial == end_serial);
		fclose(input_file);
		printf("asserted %s\n", filename);
	}

	return (0);
}
