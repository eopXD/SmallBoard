/*! \file investigate/display_hc_ko.cpp
    \brief display the data by HC master thesis (4x4)
     shall have zero liberty.
*/


#include <algorithm>
#include <bitset>
#include <cstdio>
#include <iostream>

#include "GoBoard.h"

using namespace GoConstant;
using namespace GoFunction;
using namespace std;

void display(BIT_STATE bitstring)
{
    FOR_EACH_COORD(id)
    {
        putchar(bitstring & 1 ? '1' : '0');
        bitstring >>= 1;
        if (id % BORDER_C == BORDER_C - 1) {
            putchar('\n');
        }
    }
}

/* GetAllKo() is the main thing we are doing in this phase */
BIT_STATE GetAllKo(GoBoard &board)
{

    BIT_STATE ko_state = 0;

    // board.DisplayBoard();

    FOR_EACH_COORD(id)
    {
        GoCoordId ko_position = board.CheckKoPosition(id);
        if (ko_position < 0) {
            continue;
        }
        ko_state |= 1ull << ko_position;
    }
    // display(ko_state);
    return (ko_state);
}
BIT_STATE GetAllKo(GoSerial const serial)
{
    GoBoard board(serial);
    return (GetAllKo(board));
}
int main()
{

    char filename[105];
    sprintf(filename, "master_thesis_4x4/uncompress_ko_state_4x4-16.dat");
    FILE *in_file = fopen(filename, "rb");
    if (in_file == NULL) {
        puts("NANI open file fail");
        exit(1);
    }
    uint64_t hc_ko_state;
    unsigned char uc;
    for (int cnt = 0; cnt < 132657; ++cnt) {
        hc_ko_state = 0;
        for (int i = 0; i < 8; ++i) {
            uc = fgetc(in_file);
            hc_ko_state = (hc_ko_state << 8) + uc;
        }
        printf("full: %llu\n", hc_ko_state);
        bitset<64> bst(hc_ko_state);
        for (int i = 0; i < 64; ++i) {
            cout << bst[i];
        }
        puts("");

        GoCoordId ko_position = 0;
        for (int i = 59; i <= 63; ++i) {
            ko_position = (ko_position << 1) + bst[i];
        }
        GoSerial s = 0;
        for (int i = 19; i <= 58; ++i) {
            s = (s << 1) + bst[i];
        }
        printf("id: %d serial: %llu\n", ko_position, s);
        GoBoard b(s);
        b.DisplayBoard();
        display(GetAllKo(s));
        getchar();
    }
    return (0);
}
