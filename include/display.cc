#include "display.h"
#include "getch.hpp"

using namespace std;
using namespace GoConstant;
using namespace GoFunction;

#define CLR_LINES(n)                                                           \
    do {                                                                       \
        for (int cnt = (n); cnt--;) {                                          \
            cout << "\033[1F\033[2K";                                          \
        }                                                                      \
    } while (0)

GoSerial dialog0()
{
    cout << "\033[1;37mSmall Board Go " << (int) BORDER_R << "x"
         << (int) BORDER_C << "\033[0m\n";
    cout << "Insert 1 or 2 to proceed: \n";
    cout << "1: Serial number\n";
    cout << "2: Specify board\n";
    GoSerial serial;
    do {
        char c = getch();
        switch (c) {
        case '1':
            cout << "Insert serial (0~" << MAX_SERIAL << "): ";
            cin >> serial;
            getch();
            return serial;
            break;
        case '2':
            cout << "This feature not implemented yet! XD\n";
            continue;
        default:
            cout << "\a";
            continue;
        }
    } while (1);
    return (0);
}

void prologue0()
{
    cout << "\033[1;37mSmall Board Go " << (int)BORDER_R << "x" << (int)BORDER_C
         << "\033[0m\n";
    cout << "Move: \033[1mJ(up/left)/K(down/right)\033[0m\n";
    cout << "Apply/Undo/Next/Pass move: \033[1mZ/X/C/A\033[0m\n";
    // cout << "Add stone: \033[1mD\033[0m\n";
    // cout << "Restart game: \033[1mR\033[0m\n";
    cout << "\n";
}

std::ostream &operator<<(std::ostream &os, GoBoardGui &b)
{
    os << "Serial: " << b.GetSerial() << ", "
       << "Turn: " << COLOR_CHAR[b.SelfColor()] << "\n";
    os << "Score: " << (int) b.CalcScore() << ", ";
    os << "Ko: ";
    if (b.GetKo() == COORD_UNSET) {
        os << "X";
    } else {
        os << (int) b.GetKo();
    }
    //os << ", HL: " << (int) b.hl_id;
    //os << ", hs_idx: " << (int)b.hs_idx;
    os << "\n\n";

    /*FOR_EACH_COORD(id) {
        if (id and id % BORDER_C == 0) {
            os << "\n";
        }
        if ( b.legal_move_map[id] ) {
            os << 1;
        } else {
            os << 0;
        }
    } os << "\n";*/

    FOR_EACH_COORD(id)
    {
        if (id and id % BORDER_C == 0) {
            os << "\n";
        }
        if (id == b.hl_id and !b.no_hl) {
            os << "\033[47m";
        }
        os << COLOR_CHAR[b.board_state[id]];
        os << "\033[0m";
    }
    os << "\n\n";
    return (os);
}

void GoBoardGui::RecvArrow(int dir)
{
    GoCoordId nxt_hl = max(0, hl_id + dir);
    while (nxt_hl >= 0 and nxt_hl < SMALLBOARDSIZE) {
        if (legal_move_map[nxt_hl]) {
            hl_id = nxt_hl;
            break;
        }
        nxt_hl += dir;
    }
}

void GoBoardGui::NextMove(GoCoordId target_id)
{
    if (target_id == -10) {
        if (hs_idx == hs_size) {
            cout << "\a";
            return;
        } else {
            Move(hs[hs_idx].move);
            hs_idx++;
        }
    } else {
        Move(target_id);
        hs[hs_idx].move = target_id;
        hs[hs_idx].ko = ko_position;
        hs[hs_idx].ate[0] = prev_eat_from[0];
        for ( int i=1; i<=prev_eat_from[0]; ++i ) {
            hs[hs_idx].ate[i] = prev_eat_from[i];
        }
        //hs[hs_idx] = History(target_id, ko_position, prev_eat_from);
        //cout << "Record move: " << (int)hs[hs_idx].move << " " << (int)hs[hs_idx].ko << " " << (int)hs[hs_idx].ate[0] << "\n";    
        ++hs_idx;
        hs_size = hs_idx;
    }
}
void GoBoardGui::PrevMove()
{
    if (hs_idx == 0) {
        return;
    }
    hs_idx--;
    //cout << "Trigger undo: " << (int)hs[hs_idx].move << " " << (int)hs[hs_idx].ko << " " << (int)hs[hs_idx].ate[0] << "\n";
    UndoMove(hs[hs_idx].move, hs[hs_idx].ko, hs[hs_idx].ate);
}
void GoBoardGui::RefreshHL () {
    if ( legal_move_map.count() == 0 ) {
        no_hl = 1;
    } else {
        no_hl = 0;
        RecvArrow(-1), RecvArrow(+1);
    }
}
void GoBoardGui::Refresh () {
    GetPossibleMove();
    RefreshHL();
}

void Play(GoSerial serial)
{
    if (serial == MAX_SERIAL) {
        serial = dialog0();
    }
    GoBoardGui b(serial, 1);
    prologue0();
    int ____ = 1;
    while (1) {
        if (____) {
            b.hs_idx = 0;
            b.Refresh();
        } else {
            CLR_LINES(b.clr_line_sz);
        }
        cout << b;
        char c = getch();
        switch (c) {
        case 'z': // Apply move
            b.NextMove(b.hl_id);
            b.Refresh();
            break;
        case 'x': // Prev move
            b.PrevMove();
            b.Refresh();
            break;
        case 'c': // Next move
            b.NextMove();
            b.Refresh();
            break;
        case 'a': // Pass move
            b.NextMove(-1);
            b.Refresh();
            break;
        case 'j':
        case 'k': // Modify highlight
            b.RecvArrow(c == 'j' ? -1 : +1);
            break;
        default:
            cout << "\a";
            break;
        }
        ____ = 0;
        if ( b.is_double_pass ) {
            break;
        }
    }
    cout << "Receive double pass, game end.\n";
    cout << "Result: ";
    if ( b.CalcScore() > 0 ) {
        cout << "Black win\n";
    } else if ( b.CalcScore() < 0 ) {
        cout << "White win\n";
    } else {
        cout << "Draw\n";
    }
}
