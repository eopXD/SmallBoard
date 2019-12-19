// Copyright (C) 2019 Yueh-Ting Chen (eopXD)
/*! \file FindAllPossibleState/main.cpp
	\brief Find all serial numbers that is legal. (no block on the board
	 shall have zero liberty.

	\author Yueh-Ting Chen (eopXD)
	\project Efficient & Succinct Small Board Go
*/

#include <cstdio>
#include <iostream>
#include <bitset>
#include <algorithm>

//#ifdef USE_OPENMP
#include <omp.h>
//#endif

#include "GoBoard.h"

using namespace GoConstant;
using namespace GoFunction;
using namespace std;

int main ()
{

/*
// test on a certain serial number
	const GoSerial a = 5664941825;
	GoBoard test_board(a);
	test_board.DisplayBoard();
	std::cout << "error code: " << (int) test_board.error_code << "\n";
	std::cout << "current serial: " << test_board.GetSerial() << "\n";
	for ( int i=0; i<4; ++i ) {
		test_board.RotateClockwise();
		//test_board.FlipLR();
		//test_board.DisplayBoard();
		//puts("OAO~~~~~~~~~~~~~~~~~~~~~~~~~");
		for ( int j=0; j<2; ++j ) {
			std::cout << "serial: " << test_board.GetSerial() << "\n";
			test_board.DisplayBoard();
			test_board.FlipLR();
		}
		puts("==============");
	}
	return (0);
*/


// the real process to select out the legal boards
/* BEWARE THE CONSTANT OF THIS BEFORE COMPILE AND EXECUTION */
/* BEWARE THE CONSTANT OF THIS BEFORE COMPILE AND EXECUTION */
/* BEWARE THE CONSTANT OF THIS BEFORE COMPILE AND EXECUTION */
/* BEWARE THE CONSTANT OF THIS BEFORE COMPILE AND EXECUTION */

	const GoSerial STATE_PER_FILE = (1ll<<20); // 2^32 = 2G
	const GoSerial NUMBER_OF_FILE = MAX_SERIAL/STATE_PER_FILE + 1;
	const int BUFFER_SIZE = 65536;
// statistics of state
	uint64_t total_legal_state = 0;
	uint64_t total_illegal_state = 0;
	uint64_t total_reduced_legal_state = 0;
	uint64_t total_remain_legal_state = 0;

	uint64_t illegal_state_of_file[1005] = {};
	uint64_t legal_state_of_file[1005] = {};
	uint64_t reduced_legal_state_of_file[1005] = {};
	uint64_t remain_legal_state_of_file[1005] = {};

	std::cout << "MAX_SERIAL: " << MAX_SERIAL << "\n";
	std::cout << "STATE_PER_FILE: " << STATE_PER_FILE << "\n";
	std::cout << "NUMBER_OF_FILE: " << NUMBER_OF_FILE << "\n";

//#ifdef USE_OPENMP
#pragma omp parallel
//#endif
{

//#ifdef USE_OPENMP
#pragma omp for
//#endif
	for ( GoSerial file_num=0; file_num<NUMBER_OF_FILE; ++file_num ) {
		char filename[105];
		unsigned char *buffer = new unsigned char [BUFFER_SIZE+5];
		int buf_idx = 0;
		unsigned char compact = 0;
		sprintf(filename, "data/data.SparseLegalState.part%05lu", file_num);
		
		FILE *output_file = fopen(filename, "wb");
		
		GoSerial start_serial = file_num * STATE_PER_FILE;
		GoSerial end_serial = STATE_PER_FILE*(file_num+1ull);
		if ( MAX_SERIAL < end_serial ) {
			end_serial = MAX_SERIAL;
		}
		//printf("%lld %lld\n", start_serial, end_serial);
		for ( GoSerial serial=start_serial; serial<end_serial; ++serial, compact<<=1 ) {
			bool is_smallest = 1;
			GoBoard board(serial);

			if ( board.error_code != 0 ) {
				is_smallest = 0;
//#ifdef USE_OPENMP
#pragma omp atomic
//#endif
				++total_illegal_state;
				++illegal_state_of_file[file_num];
			} else {
//#ifdef USE_OPENMP
#pragma omp atomic
//#endif
				++total_legal_state;
				++legal_state_of_file[file_num];
				for ( int i=0; i<4; ++i ) { // rotate 4 times
					board.RotateClockwise();
					for ( int j=0; j<2; ++j ) { // flip 2 times
						board.FlipLR();
						if ( board.GetSerial() < serial ) {
							is_smallest = 0;
							break;
						}
					}
					if ( !is_smallest ) {
						break;
					}
				}
				if ( !is_smallest ) {
//#ifdef USE_OPENMP
#pragma omp atomic
//#endif
					++total_reduced_legal_state;
					++reduced_legal_state_of_file[file_num];
				} else {
//#ifdef USE_OPENMP
#pragma omp atomic
//#endif
					++total_remain_legal_state;
					++remain_legal_state_of_file[file_num];
				}
			}
			compact += is_smallest;
			if ( (serial&7ull) == 7ull ) {
				buffer[buf_idx++] = compact;
				if ( buf_idx == BUFFER_SIZE ) {
					fwrite(buffer, sizeof(unsigned char), BUFFER_SIZE, output_file);	
					buf_idx = 0;
				}
				compact = 0;
			}
		}
		if ( ((end_serial-1)&7) != 7ull ) {
			buffer[buf_idx++] = compact;
			compact = 0;
		}
		//printf("buf_idx: %d\n", buf_idx);
		if ( buf_idx > 0 ) {
			fwrite(buffer, sizeof(unsigned char), BUFFER_SIZE, output_file);	
			buf_idx = 0;
		}
		fclose(output_file);
		printf("write data %s\n", filename);
	}


} // end of parallel


	printf("\ntotal_illegal_state: %lu\n", total_illegal_state);
	for ( GoSerial file_num=0; file_num<NUMBER_OF_FILE; ++file_num ) {
		printf("SparseLegalState.part%05lu: %lu\n", file_num, illegal_state_of_file[file_num]);
	}

	printf("\ntotal_legal_state: %lu\n", total_legal_state);
	for ( GoSerial file_num=0; file_num<NUMBER_OF_FILE; ++file_num ) {
		printf("SparseLegalState.part%05lu: %lu\n", file_num, legal_state_of_file[file_num]);
	}
	printf("\ntotal_reduced_legal_state: %lu\n", total_reduced_legal_state);
	for ( GoSerial file_num=0; file_num<NUMBER_OF_FILE; ++file_num ) {
		printf("SparseLegalState.part%05lu: %lu\n", file_num, reduced_legal_state_of_file[file_num]);
	}

	printf("\ntotal_remain_legal_state: %lu\n", total_remain_legal_state);
	for ( GoSerial file_num=0; file_num<NUMBER_OF_FILE; ++file_num ) {
		printf("SparseLegalState.part%05lu: %lu\n", file_num, remain_legal_state_of_file[file_num]);
	}


	return (0);
}
