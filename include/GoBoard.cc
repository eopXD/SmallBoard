#include <queue>

#include "GoBoard.h"
#include "comm.h"

using namespace GoConstant;
using namespace GoFunction;
using namespace std;

GoBoard::GoBoard()
{
    CreateGlobalVariable();

    FOR_EACH_COORD(id) { stones[id].self_id = id; }
    FOR_EACH_BLOCK(blk_id)
    {
        block_pool[blk_id].in_use = false;
        block_pool[blk_id].self_id = blk_id;
        block_pool[blk_id].stones = stones;
    }
    while (not recycled_block.empty()) {
        recycled_block.pop();
    }
    memset(board_state, 0, sizeof(board_state));
    memset(visited_position, 0, sizeof(visited_position));
    legal_move_map.reset();
    block_in_use = 0;
    current_player = BlackStone;
    opponent_player = WhiteStone;
    ko_position = COORD_UNSET;
    is_double_pass = false;
    game_length = 0;
    current_zobrist_value = zobrist_empty_board;
}
GoBoard::GoBoard(const GoBoard &rhs) : GoBoard() { CopyFrom(rhs); }
GoBoard::~GoBoard(){};

void GoBoard::CopyFrom(const GoBoard &src)
{
    *this = src;
    FixBlockInfo();
}

void GoBoard::FixBlockInfo()
{
    FOR_EACH_BLOCK(i) { block_pool[i].stones = stones; }
}

/* return value:
 *  0: success
 * -1: illegal (non-self-eat)
 * -2: illegal (self-eat) */
GoError GoBoard::Move(const GoCoordId target_id)
{
    if (not IsLegal(target_id)) {
        return (-1);
    }
    ++game_length;

    is_double_pass = IsPass(previous_move) and IsPass(target_id);
    previous_move = target_id;
    ko_position = COORD_UNSET;

    // update Zobrist Hash
    {
        current_zobrist_value ^= zobrist_switch_player;
        if (not IsPass(target_id)) {
            current_zobrist_value ^=
                zobrist_board_hash_weight[board_state[target_id]][target_id];
            board_state[target_id] = SelfColor();
            current_zobrist_value ^=
                zobrist_board_hash_weight[board_state[target_id]][target_id];
        }
    }
    if (IsPass(target_id)) {
        HandOff();
        // GetPossibleMove();
        record_zobrist[(game_length - 1 + 4) & 3] = current_zobrist_value;
        return (0);
    }
    // nb_id[0] and die_id[0] is counter,
    GoBlockId blk_id, nb_id[5], die_id[5];
    GetNewBlock(blk_id);
    GoBlock &blk = block_pool[blk_id];

    // this is a self-eat move
    if (TryMove(blk, target_id, nb_id, die_id) < 0) {
        return (-2);
    }
    stones[target_id].Reset(blk_id);
    blk.head = blk.tail = target_id;
    for (int i = 1; i <= nb_id[0]; ++i) {
        GoBlock &nb_blk = block_pool[nb_id[i]];
        visited_position[nb_id[i]] = game_length;
        nb_blk.ResetLiberty(target_id);
        if (SelfColor() == nb_blk.color) {
            blk.MergeBlocks(nb_blk);
            RecycleBlock(nb_id[i]);
        }
    }
    visited_position[blk_id] = game_length;
    for (int i = 1; i <= die_id[0]; ++i) {
        if ((1 == block_pool[die_id[i]].stone_count) and
            (1 == blk.stone_count) and (1 == blk.CountLiberty())) {
            // this is a Ko!
            ko_position = block_pool[die_id[i]].head;
        }
        // for the stones of the killed GoBlock...
        FOR_BLOCK_STONE(
            dead_stone, block_pool[die_id[i]],
            // for the neighbor of the stones
            FOR_NEIGHBOR(dead_stone, nb) {
                // if the neighbor is my color, add liberty to my stone
                if (SelfColor() == board_state[*nb]) {
                    GoBlockId my_blk_id = GetBlockIdByCoord(*nb);

                    visited_position[my_blk_id] = game_length;
                    block_pool[my_blk_id].SetLiberty(dead_stone);
                }
            } board_state[dead_stone] = EmptyStone;
            current_zobrist_value ^=
            zobrist_board_hash_weight[OpponentColor()][dead_stone];);
        RecycleBlock(die_id[i]);
    }
    // update the blocks that is effected in this move, update the liberty
    FOR_BLOCK_IN_USE(i)
    {
        if (block_pool[i].in_use and visited_position[i] == game_length) {
            block_pool[i].CountLiberty();
        }
    }
    HandOff();
    record_zobrist[(game_length - 1 + 4) & 3] = current_zobrist_value;
    return (0);
}
GoError GoBoard::Move(const GoCoordId x, const GoCoordId y)
{
    return (Move(CoordToId(x, y)));
}

void GoBoard::DisplayBoard()
{
    FOR_EACH_COORD(id)
    {
        if (id and ((id % BORDER_C) == 0)) {
            putchar('\n');
        }

        putchar(COLOR_CHAR[board_state[id]]);
    }
    putchar('\n');
}
void GoBoard::DisplayLegalMove()
{
    FOR_EACH_COORD(id)
    {
        if (id and ((id % BORDER_C) == 0)) {
            putchar('\n');
        }
        if (legal_move_map[id] == 0) {
            putchar('0');
        } else {
            putchar('1');
        }
    }
    putchar('\n');
}

// get the minimal representation serial representation of this board
GoSerial GoBoard::GetMinimal()
{
    GoSerial current = serial;
    for (int i = 0; i < 4; ++i) {
        RotateClockwise();
        for (int j = 0; j < 2; ++j) {
            FlipLR();
            if (GetSerial() < current) {
                return (serial);
            }
        }
    }
    return (current);
}


// get serial number of this->board_state[][], also cache it into this->serial
GoSerial GoBoard::GetSerial()
{
    serial = 0;
    for (GoCoordId id = SMALLBOARDSIZE - 1; id >= 0; id--) {
        serial = serial * 3 + board_state[id];
    }
    return (serial);
}

GoCoordId GoBoard::GetKo() { return (ko_position); }

// rotate clock-wise (90 degree)
void GoBoard::RotateClockwise()
{
    GoStoneColor tmp[SMALLBOARDSIZE];
    FOR_EACH_COORD(id)
    {
        GoCoordId x, y;
        IdToCoord(id, x, y);
        tmp[CoordToId(y, BORDER_C - 1 - x)] = board_state[id];
    }

    memcpy(board_state, tmp, sizeof(tmp));
}

// flip in a LR symmetric matter
void GoBoard::FlipLR()
{
    for (GoCoordId x = 0; x < BORDER_R; ++x) {
        for (GoCoordId y = 0; y < BORDER_C / 2; ++y) {
            swap(board_state[CoordToId(x, y)],
                 board_state[CoordToId(x, BORDER_C - 1 - y)]);
        }
    }
}

bool GoBoard::IsDoublePass() { return (is_double_pass); }

/* calls SetStone to setup the board
 * initialize = 1, for gameplay
 * initialize = 0, for pure calculation
 * return value:
 *   0: success
 *	-1: construct fail */
GoBoard::GoBoard(GoSerial _serial, bool initialize) : GoBoard()
{
    serial = _serial;
    FOR_EACH_COORD(id)
    {
        GoStoneColor stone_color = _serial % 3;
        _serial /= 3;
        if (stone_color == EmptyStone) {
            // don't need to set stone for empty stone.
            continue;
        }
        GoError err = SetStone(id, stone_color);
        if (err != 0) { // see error code below
            error_code = -1;
            goto END_CONSTRUCT;
        }
    }
    error_code = 0;
END_CONSTRUCT:
    /* build initialization of board detail for
     * playing on the phase 'CheckKoStates'*/
    if (initialize) {
        /* do some initialization */
    }
}

/* Place stones onto the board, but don't need maintenance of detail
 * board position or check if it is legal
 * only check for self-eat or eat move
 * The stone will be placed onto the board_state.
 * error code:
 *  0: success
 * -1: self-eat move
 * -2: eat-opponent move 
 * -3: target_id is occupied */
GoError GoBoard::SetStone(const GoCoordId target_id,
                          const GoStoneColor stone_color)
{
	if ( board_state[target_id] != EmptyStone ) {
		return (-3);
	}
    // nb_id[0] is counter,
    GoBlockId blk_id, nb_id[5];
    GetNewBlock(blk_id);
    GoBlock &blk = block_pool[blk_id];

    blk.stone_count = 1;
    blk.color = stone_color;

    GetNeighborBlocks(blk, target_id, nb_id);
    // check if we are killing anybody
    for (GoBlockId i = 1; i <= nb_id[0]; ++i) {
        GoBlock &nb_blk = block_pool[nb_id[i]];
        if (stone_color != nb_blk.color) {
            if (1 == nb_blk.CountLiberty()) {
                // eat-able!
                return (-2);
            }
        } else {
            blk.TryMergeBlocks(nb_blk);
        }
    }

    blk.ResetLiberty(target_id);

    if (0 == blk.CountLiberty()) {
        // self-eat!
        return (-1);
    }

    stones[target_id].Reset(blk_id);
    blk.head = blk.tail = target_id;
    for (GoBlockId i = 1; i <= nb_id[0]; ++i) {
        GoBlock &nb_blk = block_pool[nb_id[i]];
        nb_blk.ResetLiberty(target_id);
        if (stone_color == nb_blk.color) {
            blk.MergeBlocks(nb_blk);
            FOR_BLOCK_STONE(id, nb_blk, stones[id].block_id = blk_id;);
            RecycleBlock(nb_id[i]);
        }
    }
    current_zobrist_value ^=
        zobrist_board_hash_weight[board_state[target_id]][target_id];
    board_state[target_id] = stone_color;
    current_zobrist_value ^=
        zobrist_board_hash_weight[board_state[target_id]][target_id];
    return (0);
    /* NOTES: you can compare this function to GoBoard::Move(id), because this
     * is a reduced version of move, because we only care about initializing the
     * stones onto the board and don't need to do any maintanence on the gaming
     * detail. */
}

/* This function is called when so GoBlocks maitain their correctness
 * A broken GoBlock at most breaks into 4 components */
void GoBoard::RefreshBlock(GoBlock &blk)
{
    /* count for seperate components */
    int comp[SMALLBOARDSIZE], comp_cnt = 0;
    int used[SMALLBOARDSIZE];
    memset(comp, -1, sizeof(comp));
    memset(used, 0, sizeof(used));

    FOR_EACH_COORD(id)
    {
        if (!blk.IsStone(id) or comp[id] != -1 or used[id]) {
            continue;
        }
        queue<int> q;
        q.push(id);
        used[id] = 1;
        comp[id] = comp_cnt;
        while (!q.empty()) {
            int now_id = q.front();
            q.pop();
            FOR_NEIGHBOR(now_id, nb)
            {
                if (blk.IsStone(*nb) and used[*nb] == 0) {
                    used[*nb] = 1;
                    comp[*nb] = comp_cnt;
                    q.push(*nb);
                }
            }
        }
        ++comp_cnt;
    }
    assert(comp_cnt <= 4);

    GoBlockId blk_id[4];
    GoCoordId prev[4];
    for (int i = 0; i < comp_cnt; ++i) {
        GetNewBlock(blk_id[i]);
        prev[i] = -1;
    }
    FOR_EACH_COORD(id)
    {
        if (comp[id] < 0) {
            continue;
        }
        GoBlock &new_blk = block_pool[blk_id[comp[id]]];
        stones[id].Reset(blk_id[comp[id]]);
        new_blk.SetStone(id);
        FOR_NEIGHBOR(id, nb)
        {
            new_blk.SetVirtLiberty(*nb);
            if (board_state[*nb] == EmptyStone) {
                new_blk.SetLiberty(*nb);
            }
        }
        if (prev[comp[id]] == -1) {
            new_blk.head = id;
        } else {
            stones[prev[comp[id]]].next_id = id;
        }
        prev[comp[id]] = id;
    }
    for (int i = 0; i < comp_cnt; ++i) {
        GoBlock &new_blk = block_pool[blk_id[i]];
        new_blk.color = blk.color;
        new_blk.tail = prev[comp[i]];
        stones[prev[comp[i]]].next_id = new_blk.head;
    }
}

/* Removes the stone from the board
 * error code:
 *  0: success
 * -1: target_id is already empty */
GoError GoBoard::ResetStone(const GoCoordId target_id)
{
    if (board_state[target_id] == EmptyStone) {
        return (-1);
    }

    current_zobrist_value ^=
        zobrist_board_hash_weight[board_state[target_id]][target_id];
    board_state[target_id] = EmptyStone;
    current_zobrist_value ^=
        zobrist_board_hash_weight[board_state[target_id]][target_id];

    GoBlock &blk = block_pool[stones[target_id].block_id];
    if (1 == blk.stone_count) {
        RecycleBlock(stones[target_id].block_id);
        stones[target_id].Reset();
        return (0);
    }

    stones[target_id].Reset();
    blk.ResetStone(target_id);
    RefreshBlock(blk);
    RecycleBlock(blk.self_id);
    return (0);
}

/* NOTE: call this function only when the GoBoard is constructed by GoSerial
 * return value
 *           -1: no possible Ko for this stone's neighbor
 *    GoCoordId: GoCoordId of ko */
GoCoordId GoBoard::CheckKoPosition(const GoCoordId target_id,
                                   const GoStoneColor opponent_color)
{
    GoBlockId blk_id = GetBlockIdByCoord(target_id);
    if (blk_id == BLOCK_UNSET) {
        return (-1);
    }
    GoBlock &blk = block_pool[blk_id];

    if (blk.color != opponent_color or blk.CountStone() != 1 or
        blk.CountLiberty() != 1) {
        return (-1);
    }
    // blk should have only 1 liberty (previous move is an eat-move)
    GoCoordId eat_me = blk.FirstLiberty();
    // get all neighboring BlockId of 'eat_me'
    int nb_id[5];
    nb_id[0] = 0;
    FOR_NEIGHBOR(eat_me, nb)
    {
        if (board_state[*nb] != EmptyStone) {
            nb_id[++nb_id[0]] = GetBlockIdByCoord(*nb);
        }
    }
    if (nb_id[0] != cached_neighbor_size[eat_me]) {
        // if it is not surrounded, it is impossible to be a Ko position
        return (-1);
    }
    /* if all surrounded by white stone, then liberty position is a potential Ko
     * for the black stone. */
    for (GoBlockId i = 1; i <= nb_id[0]; ++i) {
        GoBlock &nb_blk = block_pool[nb_id[i]];
        if (nb_blk.color != opponent_color) {
            return (-1);
        }
    }
    return (eat_me);
}

/* return value:
 * 0: NULL Value (illegal board)
 * 1: non-terminate
 * 2: win
 * 3: lose
 * 4: draw */
uint8_t GoBoard::CheckTerminate(bool black_no_move, bool white_no_move)
{
    if (board_score > 0 and white_no_move) {
        return (TERMINATE_WIN);
    } else if (board_score < 0 and black_no_move) {
        return (TERMINATE_LOSE);
    } else if (black_no_move and white_no_move) {
        if (board_score > 0) {
            return (TERMINATE_WIN);
        } else if (board_score == 0) {
            return (TERMINATE_DRAW);
        } else {
            return (TERMINATE_LOSE);
        }
    } else {
        return (NOT_TERMINATE);
    }
}

/* NOTE: assume black as the current player
 * return 64-bit with encoded as a polynomial of 5
 * for encode checkout CheckTerminate */
uint64_t GoBoard::CheckTerminates(const uint32_t ko_state)
{

    // black_no_move[SMALLBOARDSIZE] is when assuming no ko on board
    bool black_no_move[SMALLBOARDSIZE + 1];

    // NOTICE the assumption on evaluating on all boards in black's turn first
    current_player = BlackStone;
    opponent_player = WhiteStone;

    // check current board score
    CalcScore();

    // ko_state precalculated, no need to set ko_position
    ko_position = COORD_UNSET;

    // check possible move for BLACK
    GetPossibleMove(); // moves of BLACK
    uint8_t black_move_num = __builtin_popcountll(legal_move_map.to_ullong());

    FOR_EACH_COORD(id)
    {
        if (black_move_num == 0) {
            black_no_move[id] = true;
            continue;
        }

        bool can_be_ko = (ko_state >> id) & 1;
        if (can_be_ko and black_move_num == 1 and legal_move_map[id]) {
            black_no_move[id] = true;
        } else {
            black_no_move[id] = false;
        }
    }
    black_no_move[SMALLBOARDSIZE] = (black_move_num == 0);

#ifdef DISPLAY_TERMINATE
    cout << "Score: " << (int) board_score << "\n";
    cout << "\nBlack's legal move: " << (int) black_move_num << "\n";
    for (GoCoordId r = 0; r < BORDER_R; ++r) {
        for (GoCoordId c = 0; c < BORDER_C; ++c) {
            cout << (int) legal_move_map[r * BORDER_C + c];
        }
        cout << "\n";
    }
    cout << "\nTrue/False if black has no move: \n";
    cout << "If no ko: " << (int) black_no_move[SMALLBOARDSIZE] << "\n";
    for (GoCoordId r = 0; r < BORDER_R; ++r) {
        for (GoCoordId c = 0; c < BORDER_C; ++c) {
            cout << (int) black_no_move[r * BORDER_C + c];
        }
        cout << "\n";
    }
#endif

    // check possible move for WHITE (white don't need to deal with ko)
    HandOff();
    GetPossibleMove(); // moves of WHITE
    uint8_t white_move_num = __builtin_popcountll(legal_move_map.to_ullong());
    bool white_no_move = (white_move_num == 0);

    // result for return
    uint64_t result = 0;
    result = CheckTerminate(black_no_move[SMALLBOARDSIZE], white_no_move);

#ifdef DISPLAY_TERMINATE
    cout << "\nWHITE legal move: " << (int) white_move_num << "\n";
    for (GoCoordId r = 0; r < BORDER_R; ++r) {
        for (GoCoordId c = 0; c < BORDER_C; ++c) {
            cout << (int) legal_move_map[r * BORDER_C + c];
        }
        cout << "\n";
    }
    cout << "white_no_move: " << (int) white_no_move << "\n";
    cout << "\nResult:\n";
    cout << "If no ko: " << (int) result << "\n";
    uint64_t term_result[SMALLBOARDSIZE];
#endif
    if (ko_state == 0) {              // skip to check on any ko
        result *= 298023223876953125; // 5**25 = 298023223876953125
        return (result);
    }

    // for possible ko positions...
    REV_FOR_EACH_COORD(id)
    {
        bool can_be_ko = (ko_state >> id) & 1;
        if (can_be_ko) {
            result =
                result * 5 + CheckTerminate(black_no_move[id], white_no_move);
        } else {
            result = result * 5; // NULL value
        }
#ifdef DISPLAY_TERMINATE
        term_result[id] = result % 5;
#endif
    }
#ifdef DISPLAY_TERMINATE
    for (GoCoordId r = 0; r < BORDER_R; ++r) {
        for (GoCoordId c = 0; c < BORDER_C; ++c) {
            cout << term_result[r * BORDER_C + c];
        }
        cout << "\n";
    }
#endif
    return (result);
}

/* NOTE: this function cannot calculate non-terminal board score correctly
 * NOTE: only for calculation on terminal positions
 * NOTE: assume black is the current player
 * returns current board score difference (BLACK-WHITE)
 * result also cached in '.board_score' variable
 * > 0: WIN  for current player
 * < 0: LOSE for current player
 * = 0: DRAW for current player */
GoCoordId GoBoard::CalcScore(GoStoneColor opponent_color)
{
    board_score = 0;
    FOR_EACH_COORD(id)
    {
        if (board_state[id] != EmptyStone) {
            if (board_state[id] == opponent_color) {
                board_score--;
            } else {
                ++board_score;
            }
        } else {
            bool no_empty_neighbor = 1; // assume having no empty neighbor
            bool friendly_neighbor = 0;
            bool opponent_neighbor = 0;
            FOR_NEIGHBOR(id, nb)
            {
                if (board_state[*nb] == EmptyStone) {
                    no_empty_neighbor = 0;
                    break;
                } else {
                    if (board_state[*nb] == opponent_color) {
                        opponent_neighbor = 1;
                    } else {
                        friendly_neighbor = 1;
                    }
                }
            }
            if (no_empty_neighbor) {
                if (friendly_neighbor and !opponent_neighbor) {
                    ++board_score;
                }
                if (opponent_neighbor and !friendly_neighbor) {
                    --board_score;
                }
            }
        }
    }
    return (board_score);
}


// returns own color
GoStoneColor GoBoard::SelfColor() { return (current_player); }
// return opponent color
GoStoneColor GoBoard::OpponentColor() { return (opponent_player); }
// give the turn to opponent
inline void GoBoard::HandOff() { swap(current_player, opponent_player); }
// return whether the move is legal
inline bool GoBoard::IsLegal(const GoCoordId id)
{
    return (IsPass(id) or legal_move_map[id]);
}

/* this finds the most "head" parent_id of the block (since blocks are
 * connected by disjoint set) */
GoCoordId GoBoard::FindCoord(const GoCoordId target_id)
{
    if (stones[target_id].parent_id ==
        stones[target_id].self_id) { // i am ancestor!
        return (stones[target_id].self_id);
    }
    return (stones[target_id].parent_id =
                FindCoord(stones[target_id].parent_id));
}

/* get GoBlockId of 'target_id' */
GoBlockId GoBoard::GetBlockIdByCoord(const GoCoordId target_id)
{
    if (EmptyStone == board_state[target_id]) {
        return (BLOCK_UNSET);
    }
    // find ancestor stone and its corresponding block_id
    return (stones[FindCoord(target_id)].block_id);
}


/* get neighbor blocks of 'target_id'
 * NOTE: stone and liberty is set into blk
 * NOTE: neighoring GoBlock saved into nb_id */
void GoBoard::GetNeighborBlocks(GoBlock &blk, const GoCoordId target_id,
                                GoBlockId *nb_id)
{
    nb_id[0] = 0;
    blk.SetStone(target_id);
    FOR_NEIGHBOR(target_id, nb)
    {
        blk.SetVirtLiberty(*nb);
        if (EmptyStone == board_state[*nb]) {
            blk.SetLiberty(*nb);
        } else {
            nb_id[++nb_id[0]] = GetBlockIdByCoord(*nb);
        }
    }
    sort(nb_id + 1, nb_id + 1 + nb_id[0]);
    nb_id[0] = unique(nb_id + 1, nb_id + nb_id[0] + 1) - nb_id - 1;
}

/* blk is the GoBlock we are 'potentially' adding if we are placing the stone
 * on 'target_id'.
 * return value:
 *  -1: illegal
 * >=0: number of opponent stone can be eaten in this move */
GoError GoBoard::TryMove(GoBlock &blk, const GoCoordId target_id,
                         GoBlockId *nb_id, GoBlockId *die_id, GoCoordId max_lib)
{
    if (!legal_move_map[target_id]) {
        return (-1);
    }
    GoCounter cnt = 0;

    blk.stone_count = 1;
    blk.color = SelfColor();

    GetNeighborBlocks(blk, target_id, nb_id);
    die_id[0] = 0;
    for (GoBlockId i = 1; i <= nb_id[0]; ++i) {
        GoBlock &nb_blk = block_pool[nb_id[i]];
        // if neighbor block is not my color...
        if (SelfColor() != nb_blk.color) {
            // and my neighboring block is only 1 liberty...
            if (1 == nb_blk.CountLiberty()) {
                // then it is eat-able!!!!
                cnt += nb_blk.stone_count;
                die_id[++die_id[0]] = nb_id[i];
            }
        }
        // else it is a friendly block, so I try to merge it...
        else {
            blk.TryMergeBlocks(nb_blk);
        }
    }

    blk.ResetLiberty(target_id);
    if (blk.CountLiberty() >= max_lib) {
        return (cnt);
    }
    if (0 != die_id[0]) {
        for (GoBlockId i = 1; i <= die_id[0]; ++i) {
            const GoBlock &die_blk = block_pool[die_id[i]];
            // opponent stones I removed will become my liberty
            blk.liberty_state |= die_blk.stone_state;
            blk.liberty_state &= blk.virtual_liberty_state;
        }
    }
    return (cnt);
}

// recycle the block, save it into stack
void GoBoard::RecycleBlock(const GoBlockId blk_id)
{
    block_pool[blk_id].in_use = false;
    recycled_block.push(blk_id);
}

// get new block (from idx - 'block_in_use') or re-used GoBlock (from stack)
void GoBoard::GetNewBlock(GoBlockId &blk_id)
{
    blk_id = BLOCK_UNSET;
    if (!recycled_block.empty()) {
        blk_id = recycled_block.top();
        recycled_block.pop();
    } else {
        blk_id = block_in_use++;
    }
    if (blk_id >= MAX_BLOCK_SIZE) {
        cerr << "GoBlock insufficient to support for this board size\n";
        cerr << "Modify GoConstant::MAX_BLOCK_SIZE to continue\n";
        exit(1);
    }
    block_pool[blk_id].Reset();
}

// get all possible move for the current board position
void GoBoard::GetPossibleMove()
{
    GoBlockId blk_id;
    // [0][?] is neighbor block_id, [1][?] is block_id killed
    // [?][0] is the counter, from [?][1]~[?][4] stores the value
    GoBlockId tmp[2][5];

    bool have_empty_neighbor;
    legal_move_map.reset();

    FOR_EACH_COORD(id)
    {
        if (EmptyStone != board_state[id]) { // can't put stone on non-empty
            continue;
        }
        if (ko_position == id) { // can not put stone on ko
            continue;
        }
        GetNewBlock(blk_id);
        GoBlock &blk = block_pool[blk_id];

        have_empty_neighbor = 0;
        FOR_NEIGHBOR(id, nb)
        {
            if (EmptyStone == board_state[*nb]) {
                have_empty_neighbor = 1;
                break;
            }
        }
        legal_move_map.set(id);
        if (have_empty_neighbor) {
            RecycleBlock(blk_id);
            continue;
        }
        // no empty neighbor
        TryMove(blk, id, tmp[0], tmp[1]);
        blk.CountLiberty();
        if (blk.liberty_count <= 0) {
            legal_move_map.reset(id);
            RecycleBlock(blk_id);
            continue;
        }
        // check for basic Ko
        if (game_length > 2) {
            GoHash new_zobrist_value = current_zobrist_value;

            new_zobrist_value ^= zobrist_board_hash_weight[SelfColor()][id];
            new_zobrist_value ^= zobrist_switch_player;

            GoBlockId *die_id = tmp[1];
            for (GoBlockId i = 1; i <= die_id[0]; ++i) {
                FOR_BLOCK_STONE(j, block_pool[die_id[i]],
                                new_zobrist_value ^=
                                zobrist_board_hash_weight[OpponentColor()][j];);
            }

            if (record_zobrist[(game_length - 1 + 4) & 3] ==
                record_zobrist[(game_length - 3 + 4) & 3]) {
                legal_move_map.reset(id);
            }
        }
        RecycleBlock(blk_id);
    }
}
