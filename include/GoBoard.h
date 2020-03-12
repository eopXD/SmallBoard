/*! \file GoBoard.hpp
    \brief Structure used to simulate gameplay, GoBlock is preallocated.
     Recycling mechanics of GoBlock is within the GoBoard
*/
#ifndef SMALLBOARD_GOBOARD_H
#define SMALLBOARD_GOBOARD_H

#include <algorithm>
#include <bitset>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <stack>

#include "GoBlock.h"
#include "GoStone.h"
#include "comm.h"

class GoBoard
{

    /************* class initialization *************/
public:
    /* default value:
     * current_player = BlackStone
     * ko_position = COORD_UNSET
     * game_length = 0 */
    GoBoard();
    GoBoard(const GoBoard &rhs);
    ~GoBoard();
    void CopyFrom(const GoBoard &src);
    /* call SetStone to setup
     * initialize = 1, for gameplay
     * initialize = 0, for pure calculation
     * error_code is set for error handling */
    GoBoard(const GoSerial _serial, bool initialize = 0);
    GoError error_code;

protected:
    /* redirect '*stone' pointer in GoBlock to the stones[] after CopyFrom() */
    void FixBlockInfo();

public:
    /* get all possible move for the current board position */
    void GetPossibleMove();
    /* move(modify) assets of the board
     * cannot be called before GetPossibleMove
     * return value:
     *  0: success
     * -1: illegal (non-self-eat)
     * -2: illegal (self-eat) */
    GoError Move(const GoCoordId target_id);
    GoError Move(const GoCoordId x, const GoCoordId y);

    GoError UndoMove(const GoCoordId target_id,
                     const GoCoordId set_ko = GoConstant::COORD_UNSET,
                     const GoCoordId *ate_from = NULL);
    // GoError UndoMove(const GoCoordId x, const GoCoordId y, const GoCoordId
    // set_ko, const GoCoordId *ate_from);

protected:
    /* try if move is possible on 'target_id' */
    GoError TryMove(GoBlock &blk, const GoCoordId target_id, GoBlockId *nb_id,
                    GoBlockId *die_id,
                    GoCoordId max_lib = GoConstant::SMALLBOARDSIZE);

    /************* display, only for debug purpose *************/
public:
    void DisplayBoard();
    void DisplayLegalMove();
    /* no_use = 1, also display used but not using GoBlocks */
    void DisplayGoBlock(bool no_use = 0);

    /******** board manipulation for checking if state is reduce-able ********/
public:
    /* NOTE: this function cannot be used in real gameplay because the
     * flipping and rotation will messup the board check if this board is its
     * minimal serial number representation */
    GoSerial GetMinimal();
    GoSerial GetSerial();
    GoCoordId GetKo();
    void RotateClockwise();
    /* flip in LR symmetric matter */
    void FlipLR();

    /* returns if 2 passes is consecutively used (during gameplay) */
    bool IsDoublePass();

    /************* stone initialization *************/
public:
    /* place stones onto the board, stone will be placed onto the board_state.
     * z-hash value of board will be updated.
     * return value:
     *  0: success
     * -1: illegal (self-eat)
     * -2: illegal (eat-opponent)
     * -3: target_id is occupied */
    GoError SetStone(const GoCoordId target_id, const GoStoneColor stone_color);
    /* removes the stone from board, calls RefreshBlock
     * return value:
     *  0: success
     * -1: target_id is already empty */
    GoError ResetStone(const GoCoordId target_id);

    /************* check ko position *************/
public:
    /* call this function only when the GoBoard is constructed by GoSerial
     * check for neighboring ko position around
     * return value:
     *        -1: no possible Ko for target_id neighbor
     * GoCoordId: the GoCoordId of the ko position */
    GoCoordId
    CheckKoPosition(const GoCoordId target_id,
                    const GoStoneColor opponent_color = GoConstant::WhiteStone);

    /************* check termination *************/
public:
    /* conditions for a terminating terminate board */
    uint8_t CheckTerminate(bool black_no_move, bool white_no_move);
    /* NOTE: reduce all boards to black's turn (same as previous phase CheckKo)
     * NOTE: call this function after board is initialized
     * return value: (a ploynomial of series of 5)
     * 0: NULL Value (illegal board)
     * 1: non-terminate
     * 2: win
     * 3: lose
     * 4: draw */
    uint64_t CheckTerminates(const uint32_t ko_state);

    /* returns current score difference (BlackStone-WhiteStone)
     * NOTE: result also cached into '.board_score'
     * > 0: black win
     * < 0: black lose
     * = 0: draw */
    GoCoordId CalcScore(GoStoneColor opponent_color = GoConstant::WhiteStone);

    /************* interface to modify asset *************/
public:
    void SetTurn(GoStoneColor me, GoStoneColor you)
    {
        current_player = me;
        opponent_player = you;
    }
    void SetKo(GoCoordId id) { ko_position = id; }
    /* return current_player */
    GoStoneColor SelfColor();
    /* return opponent_player */
    GoStoneColor OpponentColor();
    /* give the turn to opponent */
    inline void HandOff();
    /* return whether the move is legal
     * NOTE: only call after legal_move_map is set */
    inline bool IsLegal(const GoCoordId id);

protected:
    /* finds use to find block_id of the ancestor stone */
    GoCoordId FindCoord(const GoCoordId id);
    /* get GoBlockId of 'id' */
    GoBlockId GetBlockIdByCoord(const GoCoordId id);
    /* get neighbor blocks of 'target_id'
     * NOTE: stone and liberty is set into blk
     * NOTE: neighoring GoBlock saved into nb_id */
    void GetNeighborBlocks(GoBlock &blk, const GoCoordId target_id,
                           GoBlockId *nb_id);

    /************* GoBlock manipulation *************/
protected:
    /* refresh GoBlock, removing a stone may cause a GoBlock to split into at
     * most 4 GoBlock */
    void RefreshBlock(GoBlock &blk);
    /* recycle GoBlock for reuse */
    void RecycleBlock(const GoBlockId blk_id);
    /* get a new GoBlock */
    void GetNewBlock(GoBlockId &blk_id);

public:
    /************* GoBoard data *************/
    GoSerial serial;
    GoStone
        stones[GoConstant::SMALLBOARDSIZE]; /* circular-singly linked-list */
    /* NOTE: information of previous maybe extracted to a new class? */
    GoCoordId previous_move, previous_ko;
    GoCoordId prev_eat_from[5];
    /* GoBlock resource*/
    std::stack<GoBlockId> recycled_block;
    GoBlock block_pool[GoConstant::MAX_BLOCK_SIZE];
    GoCounter block_in_use;
    /* naiive information */
    GoStoneColor board_state[GoConstant::SMALLBOARDSIZE];
    GoStoneColor current_player, opponent_player;
    GoCoordId ko_position;
    GoCounter game_length;
    GoCounter visited_position[GoConstant::MAX_BLOCK_SIZE];
    bool is_double_pass;
    GoScore board_score;
    /* Zobrist Hash to forbid Basic Ko (we allow Positional SuperKo) */
    GoHash record_zobrist[4];
    GoHash current_zobrist_value;
    /* legal_move_map[idx] = 1 means we can play a stone onto that position */
    std::bitset<GoConstant::SMALLBOARDSIZE> legal_move_map;
};


/************* conventional for-loop *************/
/* for all GoBlock
 * NOTE: only for GoBoard internal use */
#define FOR_BLOCK_IN_USE(blk_id)                                               \
    for (GoBlockId blk_id = 0; blk_id < block_in_use; ++blk_id)

/* GoBoard.stones[] are maintained in a linked-list style
 * iteration around stones of a GoBoard 
 * NOTE: only for GoBoard internal use */
#define FOR_BLOCK_STONE(id, blk, loop_body)                                    \
    {                                                                          \
        GoCoordId id = (blk).head;                                             \
        while (1) {                                                            \
            loop_body if (id == stones[id].next_id) { break; }                 \
            id = stones[id].next_id;                                           \
        }                                                                      \
    }

#endif