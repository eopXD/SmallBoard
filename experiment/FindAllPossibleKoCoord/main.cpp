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

#include <omp.h>

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
BIT_STATE GetAllKo ( GoBoard &board ) {
	
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
BIT_STATE GetAllKo ( GoSerial const serial ) {
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
	const GoSerial STATE_PER_FILE = (1ll<<20); // 2^32 = 2G
	const GoSerial NUMBER_OF_FILE = MAX_SERIAL/STATE_PER_FILE + 1;
	const int BUFFER_SIZE = 65536;

	std::cout << "MAX_SERIAL: " << MAX_SERIAL << "\n";
	std::cout << "STATE_PER_FILE: " << STATE_PER_FILE << "\n";
	std::cout << "NUMBER_OF_FILE: " << NUMBER_OF_FILE << "\n";

// read path from previous phase
	char read_file_path[105] = "../FindAllPossibleSerial/";

// statistics
	uint64_t total_legal_reduced_state = 0;
	uint64_t legal_reduced_state_of_file[1005] = {};
	uint64_t total_ko_state = 0;
	uint64_t ko_state_of_file[1005] = {};

// statistics of ko
	uint64_t num_of_max_ko_of_file[1005] = {};
	uint64_t maximum_ko_of_file[1005] = {};
	GoSerial maximum_ko_serial_of_file[1005] = {};

#pragma omp parallel
{
#pragma omp for
	for ( GoSerial file_num=0; file_num<NUMBER_OF_FILE; ++file_num ) {
		char filename[105];
		unsigned char *read_buffer = new unsigned char [BUFFER_SIZE+5];
		char write_filename[105];
		uint32_t *write_buffer = new uint32_t [BUFFER_SIZE+5];
		int w_buf_idx;

		sprintf(filename, "%sdata/data.SparseLegalState.part%05lu",
		 read_file_path, file_num);
		FILE *input_file = fopen(filename, "rb");
		if ( input_file == NULL ) {
			printf("NANI!!!! open %s fail\n", filename);
			exit(1);
		}	

		sprintf(write_filename, "data/data.SparseKoBitState.part%05lu", file_num);
		FILE *output_file = fopen(write_filename, "wb");

		GoSerial start_serial = STATE_PER_FILE*file_num;
		GoSerial end_serial = STATE_PER_FILE*(file_num+1ull);
		if ( MAX_SERIAL < end_serial ) {
			end_serial = MAX_SERIAL;
		}
		
		GoSerial serial = start_serial;
		while ( fread(read_buffer, BUFFER_SIZE, sizeof(unsigned char), input_file) ) {
			w_buf_idx = 0;
			for ( int buf_idx=0; buf_idx<BUFFER_SIZE; ++buf_idx ) {
				unsigned char compact = read_buffer[buf_idx];
				for ( int pos=7; pos>=0; pos-- ) {
					bool is_reduced_legal = compact&(1<<pos);
/* what you are going to do with the bit 'is_reduced_legal */
/* start */
					if ( is_reduced_legal ) {
						BIT_STATE ko_state = GetAllKo(serial);
						uint32_t num_of_ko = __builtin_popcount(ko_state);
						
#pragma omp atomic
						++total_legal_reduced_state;
						++legal_reduced_state_of_file[file_num];
						
#pragma omp atomic
						total_ko_state += num_of_ko;
						ko_state_of_file[file_num] += num_of_ko;

						if ( num_of_ko > maximum_ko_of_file[file_num] ) {
							maximum_ko_of_file[file_num] = num_of_ko;
							maximum_ko_serial_of_file[file_num] = serial;
							num_of_max_ko_of_file[file_num] = 1;
						} else if ( num_of_ko == maximum_ko_of_file[file_num] ) {
							++num_of_max_ko_of_file[file_num];
						}
						
						write_buffer[w_buf_idx++] = GetAllKo(serial);
					} else {
						write_buffer[w_buf_idx++] = 0;
					}
/* end */
					if ( w_buf_idx == BUFFER_SIZE ) {
						fwrite(write_buffer, sizeof(uint32_t), 
						 BUFFER_SIZE, output_file);	
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
				fwrite(write_buffer, sizeof(unsigned char), 
				 BUFFER_SIZE, output_file);	
				w_buf_idx = 0;
			}
		}
//		assert(serial == end_serial);
		fclose(input_file);
		fclose(output_file);
	}

} // end of parallel

	printf("\ntotal_legal_reduced_state: %lu\n", total_legal_reduced_state);
	for ( GoSerial file_num=0; file_num<NUMBER_OF_FILE; ++file_num ) {
		printf("SparseKoBitState.part%05lu: %lu\n", file_num, 
		 legal_reduced_state_of_file[file_num]);
	}

	printf("\ntotal_ko_state: %lu\n", total_ko_state);
	for ( GoSerial file_num=0; file_num<NUMBER_OF_FILE; ++file_num ) {
		printf("SparseKoBitState.part%05lu: %lu\n", file_num, 
		 ko_state_of_file[file_num]);
	}


	uint64_t maximum_ko_per_serial = 0;
	uint64_t num_of_max_ko = 0;
	for ( GoSerial file_num=0; file_num<NUMBER_OF_FILE; ++file_num ) {
		if ( maximum_ko_of_file[file_num] > maximum_ko_per_serial ) {
			maximum_ko_per_serial = maximum_ko_of_file[file_num];
			num_of_max_ko = num_of_max_ko_of_file[file_num];
		} else if ( maximum_ko_of_file[file_num] == maximum_ko_per_serial ) {
			num_of_max_ko += num_of_max_ko_of_file[file_num];
		}
	}

	printf("\nmaximum_ko_per_serial: %lu\n", maximum_ko_per_serial);
	for ( GoSerial file_num=0; file_num<NUMBER_OF_FILE; ++file_num ) {
		printf("SparseKoBitState.part%05lu: %lu\n", file_num, 
		 maximum_ko_of_file[file_num]);
	}

	printf("\nnum_of_max_ko: %lu\n", num_of_max_ko);
	for ( GoSerial file_num=0; file_num<NUMBER_OF_FILE; ++file_num ) {
		if ( maximum_ko_of_file[file_num] == maximum_ko_per_serial ) {
			printf("SparseKoBitState.part%05lu: %lu, %lu\n", file_num, 
			 num_of_max_ko_of_file[file_num], 
			 maximum_ko_serial_of_file[file_num]);
		}
	}


	return (0);
}