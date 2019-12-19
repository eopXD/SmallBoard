// Copyright (C) 2019 Yueh-Ting Chen (eopXD)
/*! \file FindAllPossibleKoCoord/check_distribution.cpp
	\brief After running exec.main, check on the data for statistics
	 on number of ko for every cooridinate.

	\author Yueh-Ting Chen (eopXD)
	\project Efficient & Succinct Small Board Go
*/


#include <cstdio>
#include <iostream>
#include <algorithm>
#include <vector>

#include <omp.h>

#include "GoBoard.h"

using namespace GoConstant;
using namespace GoFunction;
using namespace std;

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
	char read_file_path[105] = "./data/";


// statistics
	uint64_t id_distribution[SMALLBOARDSIZE] = {};
	uint64_t id_distribution_of_file[1005][SMALLBOARDSIZE] = {};
// max_ko
	const uint32_t MAX_KO = TO_BE_FILLED;
	vector<GoSerial> serial_with_max_ko[1005];

#pragma omp parallel
{
#pragma omp for
	for ( GoSerial file_num=0; file_num<NUMBER_OF_FILE; ++file_num ) {
		char filename[105];
		uint32_t *read_buffer = new uint32_t [BUFFER_SIZE+5];
		
		sprintf(filename, "%sdata.SparseKoBitState.part%05lu",
		 read_file_path, file_num);
		FILE *input_file = fopen(filename, "rb");
		if ( input_file == NULL ) {
			printf("NANI!!!! open %s fail\n", filename);
			exit(1);
		}	

		GoSerial start_serial = STATE_PER_FILE*file_num;
		GoSerial end_serial = STATE_PER_FILE*(file_num+1ull);
		if ( MAX_SERIAL < end_serial ) {
			end_serial = MAX_SERIAL;
		}
		
		GoSerial serial = start_serial;
		while ( fread(read_buffer, BUFFER_SIZE, sizeof(uint32_t), input_file) ) {
			for ( int buf_idx=0; buf_idx<BUFFER_SIZE; ++buf_idx ) {
/* what you are going to do with the bit 'is_reduced_legal */
/* start */
				uint32_t ko_state = read_buffer[buf_idx];
				FOR_EACH_COORD(id) {
					if ( (ko_state>>id)&1 ) {
						++id_distribution_of_file[file_num][id];
					}
				}


				uint32_t num_of_ko = __builtin_popcount(ko_state);
				if ( num_of_ko == MAX_KO ) {
					serial_with_max_ko[file_num].push_back(serial);
				}
/* end */
				++serial;
			}

			if ( serial == end_serial ) {
				break;
			}
		}
//		assert(serial == end_serial);
		fclose(input_file);
		fclose(output_file);
	}

} // end of parallel
	
	for ( GoSerial file_num=0; file_num<NUMBER_OF_FILE; ++file_num ) {
		FOR_EACH_COORD(id) {
			id_distribution[id] += id_distribution_of_file[file_num][id];
		}
	}

	FOR_EACH_COORD(id) {
		printf("id: %d, number of ko_state: %lu\n", id_distribution[id]);
	}

	printf("\nserial with maximum ko %d\n", MAX_KO);
	for ( GoSerial file_num=0; file_num<NUMBER_OF_FILE; ++file_num ) {
		printf("part %lu: ", file_num);
		for ( GoSerial serial : serial_with_max_ko[file_num] ) {
			printf("%lu, ", serial);
		}
		printf("\n");
	}

	return (0);
}