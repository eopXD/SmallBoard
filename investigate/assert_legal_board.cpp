/*! \file investigate/assert_legal_board.cpp
    \brief investigate on the experiment results from HC master thesis
     investigate on legalBoard, which is sparse bitstate of whether a serial
     is legal
*/

#include <bitset>
#include <cassert>
#include <cstdio>
#include <iostream>

#include "GoBoard.h"

using namespace GoConstant;
using namespace GoFunction;
using namespace std;

int main()
{
    const GoSerial STATE_PER_FILE = 2000000;
    const GoSerial NUMBER_OF_FILE = MAX_SERIAL / STATE_PER_FILE + 1;

    char read_filename[105];
    unsigned char c;

    for (GoSerial file_num = 0; file_num < NUMBER_OF_FILE; ++file_num) {
        sprintf(read_filename,
                "master_thesis_4x4/legal_board/legal_board.part%03llu",
                file_num);
        FILE *input_file = fopen(read_filename, "rb");

        GoSerial start_serial = file_num * STATE_PER_FILE;
        GoSerial end_serial = STATE_PER_FILE * (file_num + 1ull);

        if (MAX_SERIAL < end_serial) {
            end_serial = MAX_SERIAL;
        }
        printf("start %s\n%lld %lld\n", read_filename, start_serial,
               end_serial);

        for (GoSerial serial = start_serial; serial < end_serial; ++serial) {
            c = fgetc(input_file);
            bool is_reduced_and_legal = 1;
            GoBoard board(serial);
            if (board.error_code != 0) {
                is_reduced_and_legal = false;
            } else {
                for (int i = 0; i < 4; ++i) {
                    board.RotateClockwise();
                    for (int j = 0; j < 2; ++j) {
                        board.FlipLR();
                        if (board.GetSerial() < serial) {
                            is_reduced_and_legal = false;
                            break;
                        }
                    }
                    if (!is_reduced_and_legal) {
                        break;
                    }
                }
            }

            assert(c == is_reduced_and_legal);
        }
        fclose(input_file);
        printf("%s matched\n", read_filename);
    }

    return (0);
}