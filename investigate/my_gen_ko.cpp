// Copyright (C) 2019 Yueh-Ting Chen (eopXD)
/*! \file investigate/my_gen_ko.cpp
	\brief output the ko states in the way similar to  HC master thesis
	 to find out who is wrong
	 shall have zero liberty.

	\author Yueh-Ting Chen (eopXD)
	\project Efficient & Succinct Small Board Go
*/


#include <cstdio>
#include <iostream>
#include <algorithm>
#include <bitset>

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
/* reading the data from previous phase */

/* BEWARE THE CONSTANT OF THIS BEFORE COMPILE AND EXECUTION */
/* BEWARE THE CONSTANT OF THIS BEFORE COMPILE AND EXECUTION */
/* BEWARE THE CONSTANT OF THIS BEFORE COMPILE AND EXECUTION */
/* BEWARE THE CONSTANT OF THIS BEFORE COMPILE AND EXECUTION */
	const GoSerial STATE_PER_FILE = 2000000; // 2^32 = 2G
	const GoSerial NUMBER_OF_FILE = MAX_SERIAL/STATE_PER_FILE + 1;

	std::cout << "MAX_SERIAL: " << MAX_SERIAL << "\n";
	std::cout << "STATE_PER_FILE: " << STATE_PER_FILE << "\n";
	std::cout << "NUMBER_OF_FILE: " << NUMBER_OF_FILE << "\n";

	char read_filename[105];
	unsigned char c;
	uint64_t *result;
	result = new uint64_t [200000];
	uint64_t idx = 0;

	for ( GoSerial file_num=0; file_num<NUMBER_OF_FILE; ++file_num ) {
		sprintf(read_filename, 
		 "master_thesis_4x4/legal_board/legal_board.part%03llu", file_num);
		FILE *input_file = fopen(read_filename, "rb");

		GoSerial start_serial = STATE_PER_FILE*file_num;
		GoSerial end_serial = STATE_PER_FILE*(file_num+1ull);
		if ( MAX_SERIAL < end_serial ) {
			end_serial = MAX_SERIAL;
		}
		printf("start %s\n%lld %lld\n", read_filename, start_serial, end_serial);
		for ( GoSerial serial=start_serial; serial<end_serial; ++serial ) {
			c = fgetc(input_file);
			if ( c == 0 ) {
				continue;
			}

			GoBoard board(serial);
			BIT_STATE ko_state = GetAllKo(board);
			uint32_t num_of_ko = __builtin_popcount(ko_state);
			if ( num_of_ko ) {
				FOR_EACH_COORD(id) {
					if ( (ko_state>>id)&1 ) {
						printf("serial: %llu, id: %d\n", serial, id);
						board.DisplayBoard();
						display(ko_state);
						
						getchar();
						uint64_t full_state = 0;
						full_state = ((uint64_t)id<<59) + (serial<<19);
						result[idx++] = full_state;
					}
				}
			}
		}
		fclose(input_file);
		printf("%s done\n", read_filename);
	}

	printf("total ko state: %llu\n", idx);

	char write_filename[105];
	sprintf(write_filename, "data.my_ko");
	FILE *output_file = fopen(write_filename, "wb");
	fwrite(result, sizeof(uint64_t), idx, output_file);	
	fclose(output_file);
	return (0);
}
