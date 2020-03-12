/*! \file getch.hpp
    \brief gain terminal keystroke in linux
*/
#ifndef GETCH_HPP
#define GETCH_HPP

#include <stdio.h>
#include <termios.h>

struct termios old, neww;

// initialize new terminal i/o settings
void initTermios(int echo)
{
    tcgetattr(0, &old);      // grab old terminal i/o settings
    neww = old;              // make new settings same as old settings
    neww.c_lflag &= ~ICANON; // disable buffered i/o
    if (echo) {
        neww.c_lflag |= ECHO; // set echo mode
    } else {
        neww.c_lflag &= ~ECHO; // set no echo mode
    }
    tcsetattr(0, TCSANOW, &neww); // use these new terminal i/o settings now
}

// Restore old terminal i/o settings
void resetTermios() { tcsetattr(0, TCSANOW, &old); }
// return the character read after a keystroke immediately
// similar to the 'getch()' defined in conio.h on Windows
// read 1 character - echo defines echo mode
char getch_(int echo)
{
    char ch;
    initTermios(echo);
    ch = getchar();
    resetTermios();
    return (ch);
}

// read 1 character without echo
char getch() { return (getch_(0)); }

// read 1 character with echo
char getche() { return (getch_(1)); }
#endif
