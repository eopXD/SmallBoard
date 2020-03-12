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
    cout << "Apply/Undo/Next move: \033[1mZ/X/C\033[0m\n";
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
    os << ", HL: " << (int) b.hl_id;
    os << ", hs_idx: " << (int)b.hs_idx;
    os << "\n\n";

    FOR_EACH_COORD(id) {
        if (id and id % BORDER_C == 0) {
            os << "\n";
        }
        if ( b.available_hl[id] ) {
            os << 1;
        } else {
            os << 0;
        }
    } os << "\n";

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
    os << "\n";
    return (os);
}

void GoBoardGui::RecvArrow(int dir)
{
    GoCoordId nxt_hl = max(0, hl_id + dir);
    while (nxt_hl >= 0 and nxt_hl < SMALLBOARDSIZE) {
        if (available_hl[nxt_hl]) {
            hl_id = nxt_hl;
            break;
        }
        nxt_hl += dir;
    }
}

void GoBoardGui::NextMove(GoCoordId target_id)
{
    if (target_id == -1) {
        if (hs_idx == hs_size) {
            cout << "\a";
            return;
        } else {
            Move(hs[hs_idx].move);
            hs_idx++;
        }
    } else {
        Move(target_id);
        hs[hs_idx] = History(target_id, ko_position, prev_eat_from);
        cout << "Record move: " << (int)hs[hs_idx].move << " " << (int)hs[hs_idx].ko << " " << (int)hs[hs_idx].ate[0] << "\n";    
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
    cout << "Trigger undo: " << (int)hs[hs_idx].move << " " << (int)hs[hs_idx].ko << " " << (int)hs[hs_idx].ate[0] << "\n";
    UndoMove(hs[hs_idx].move, hs[hs_idx].ko, hs[hs_idx].ate);
}

void Play(GoSerial serial)
{
    if (serial == MAX_SERIAL) {
        serial = dialog0();
    }
    GoBoardGui b(serial);
    prologue0();
    int ____ = 1;
    while (1) {
        if (____) {
            b.hs_idx = 0;
            b.GetPossibleMove();
            b.available_hl = b.legal_move_map;
            b.RecvArrow(+1);
        } else {
            //CLR_LINES(b.clr_line_sz);
        }
        cout << b;
        char c = getch();
        switch (c) {
        case 'z': // Apply move
            b.NextMove(b.hl_id);
            b.GetPossibleMove();
            b.available_hl = b.legal_move_map;
            b.RecvArrow(-1), b.RecvArrow(+1);
            break;
        case 'x': // Prev move
            b.PrevMove();
            b.GetPossibleMove();
            b.available_hl = b.legal_move_map;
            b.RecvArrow(-1), b.RecvArrow(+1);
            break;
        case 's': // Next move
            b.NextMove();
            b.GetPossibleMove();
            b.available_hl = b.legal_move_map;
            b.RecvArrow(-1), b.RecvArrow(+1);
            break;
        case 'j':
        case 'k': // Modify highlight
            b.available_hl = b.legal_move_map;
            b.RecvArrow(c == 'j' ? -1 : +1);
            break;
        default:
            cout << "\a";
            break;
        }
        ____ = 0;
    }
}
