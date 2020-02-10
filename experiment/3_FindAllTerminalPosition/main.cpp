// Copyright (C) 2019 Yueh-Ting Chen (eopXD)
/*! \file FindAllTerminalPosition/main.cpp
	\brief Find all terminal position given 
	 reduced-legal serial and possible ko positions, 
	 for every serial outputs a 64-bit encoded data
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
*/

/* BEWARE THE CONSTANT OF THIS BEFORE COMPILE AND EXECUTION */
/* BEWARE THE CONSTANT OF THIS BEFORE COMPILE AND EXECUTION */
/* BEWARE THE CONSTANT OF THIS BEFORE COMPILE AND EXECUTION */
/* BEWARE THE CONSTANT OF THIS BEFORE COMPILE AND EXECUTION */
	const GoSerial STATE_PER_FILE = (1ll<<30); // 2^32 = 2G
	const GoSerial NUMBER_OF_FILE = MAX_SERIAL/STATE_PER_FILE + 1;
	const int BUFFER_SIZE = 65536;

	std::cout << "MAX_SERIAL: " << MAX_SERIAL << "\n";
	std::cout << "STATE_PER_FILE: " << STATE_PER_FILE << "\n";
	std::cout << "NUMBER_OF_FILE: " << NUMBER_OF_FILE << "\n";

// read path from previous phases
	char read_legal_reduced_filepath[105] = "../FindAllPossibleSerial/4x4data";
	char read_ko_state_filepath[105] = "../FindAllPossibleKo/4x4data";

// progress bar
	uint64_t total_finished_parts = 0;

	for ( GoSerial file_num=0; file_num<NUMBER_OF_FILE; ++file_num ) {
		char LegalStateFilename[105];
		char KoStateFilename[105];
		// read legal-reduced
		unsigned char *read_legal_reduce = new unsigned char [BUFFER_SIZE+5]; 
		// reads ko_state
		uint32_t *read_ko = new uint32_t [BUFFER_SIZE+5]; 
		
		// write terminal state
		char write_filename[105];
		uint64_t *write_buffer = new uint64_t [BUFFER_SIZE+5]; // write 
		
		sprintf(LegalStateFilename, "%s/data.SparseLegalState.part%05lu",
		 read_legal_reduced_filepath, file_num);
		sprintf(KoStateFilename, "%s/data.SparseKoBitState.part%05li",
		 read_ko_state_filepath, file_num);

		FILE *input_legal = fopen(LegalStateFilename, "rb");
		if ( input_legal == NULL ) {
			printf("NANI!!!! open %s fail\n", LegalStateFilename);
			exit(1);
		}	

		FILE *input_ko = fopen(KoStateFilename, "rb");
		if ( input_ko == NULL ) {
			printf("NANI!!!! open %s fail\n", KoStateFilename);
			exit(1);
		}

		sprintf(write_filename, "data/data.SparseTerminal.part%05lu", file_num);
		FILE *output_file = fopen(write_filename, "wb");
		if ( output_file == NULL ) {
			printf("open write file fail\n");
			exit(1);
		}

		GoSerial start_serial = STATE_PER_FILE*file_num;
		GoSerial end_serial = STATE_PER_FILE*(file_num+1ull);
		if ( MAX_SERIAL < end_serial ) {
			end_serial = MAX_SERIAL;
		}
		
		fprintf(stdout, "open file_num: %llu, start_serial: %llu\n"
			, file_num, start_serial);

		GoSerial serial = start_serial;
	
		// buffer index
		int legal_buf_idx;
		int ko_buf_idx = -1;
		int w_buf_idx = 0;

		while ( fread(read_legal_reduce, BUFFER_SIZE, sizeof(unsigned char), input_legal) ) {
			
			for ( legal_buf_idx=0; legal_buf_idx<BUFFER_SIZE; ++legal_buf_idx ) {
				unsigned char compact = read_legal_reduce[legal_buf_idx];
				for ( int pos=7; pos>=0; pos-- ) {
					bool is_reduced_legal = compact&(1<<pos);
					
					if ( ko_buf_idx < 0 or ko_buf_idx == BUFFER_SIZE ) {
						fread(read_ko, BUFFER_SIZE, sizeof(uint32_t), input_ko);			
						ko_buf_idx = 0;
					}
					uint32_t ko_state = read_ko[ko_buf_idx++];
					
/* what you are going to do with the bit 'is_reduced_legal */
/* start */
					if ( is_reduced_legal ) {

					} else {

					}
/* end */
					if ( w_buf_idx == BUFFER_SIZE ) {

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
			if ( w_buf_idx > 0 ) {
				fwrite(write_buffer, sizeof(uint64_t), 
				 BUFFER_SIZE, output_file);	
				w_buf_idx = 0;
			}
			if ( serial == end_serial ) {
				break;
			}

		}
//		assert(serial == end_serial);
		fclose(input_file);
		fclose(output_file);
		fprintf(stdout, "close file_num: %llu", file_num);
	}

	return (0);
}