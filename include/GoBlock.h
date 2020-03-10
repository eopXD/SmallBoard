/*! \file GoBlock.hpp
    \brief GoStone together form 'string' (block). This structure maintins
   liberty and stone of a block. Block may merge after SetStone, may divide
   after ResetStone.
*/
#ifndef SMALLBOARD_GOBLOCK_H
#define SMALLBOARD_GOBLOCK_H

#include "GoStone.h"
#include "comm.h"
#include <iostream>

using BIT_STATE = uint32_t;

// lowest bit of the variable x
inline BIT_STATE LB(const BIT_STATE x) { return (x & (-x)); }

struct GoBlock {
    GoBlockId self_id;       // id of this block
    bool in_use;             // whether the block is in-use
    GoCounter liberty_count; // count of liberty
    GoCounter stone_count;   // count of stone
    GoStoneColor color;      // player that owns the block

    GoStone *stones;      // array pointer to GoBoard.stones
    GoCoordId head, tail; // head and tail position stones of the block

    /* LSB style, lowest bit is represents the 0th in 'id'
     * liberty_state[idx] = 1 means that there is liberty on the position 'idx'
     */
    BIT_STATE liberty_state;
    /* virtual liberty is possible liberty area of the GoBlock
     * maintained for convenience when MergeBlock is called
     */
    BIT_STATE virtual_liberty_state;
    // stone_state[idx] = 1 means that there us a stone on the position 'idx'
    BIT_STATE stone_state;

    void Reset()
    {
        in_use = true;
        liberty_count = stone_count = 0;
        liberty_state = virtual_liberty_state = stone_state = 0;
    }

    /* FirstLiberty and FirstStone are utilized in GetAllPossibleKo phase
     * returns liberty with the smallest 'id' */
    GoCoordId FirstLiberty()
    {
        return (cached_log2_table[LB(liberty_state) % 67]);
    }
    // return stone with the smallest 'id'
    GoCoordId FirstStone() { return (cached_log2_table[LB(stone_state) % 67]); }

    inline void SetLiberty(const GoCoordId id) { liberty_state |= 1ull << id; }
    inline void SetVirtLiberty(const GoCoordId id)
    {
        virtual_liberty_state |= 1ull << id;
    }
    inline void SetStone(const GoCoordId id) { stone_state |= 1ull << id; }
    inline void ResetLiberty(const GoCoordId id)
    {
        liberty_state &= ~(1ull << id);
    }
    inline void ResetVirtLiberty(const GoCoordId id)
    {
        virtual_liberty_state &= ~(1ull << id);
    }
    inline void ResetStone(const GoCoordId id) { stone_state &= ~(1ull << id); }
    inline bool GetLiberty(const GoCoordId id)
    {
        return (liberty_state & (1ull << id));
    }
    inline GoCounter CountLiberty()
    {
        return (liberty_count = __builtin_popcount(liberty_state));
    }
    inline GoCounter CountStone()
    {
        return (stone_count = __builtin_popcount(stone_state));
    }
    inline GoCounter CountVirtLiberty()
    {
        return (__builtin_popcount(virtual_liberty_state));
    }
    inline bool IsStone(const GoCoordId id)
    {
        return ((stone_state >> id) & 1);
    }
    bool IsNoLiberty() { return (0 == this->CountLiberty()); }

    GoStone *GetHead() const { return (this->stones + this->head); }
    GoStone *GetTail() const { return (this->stones + this->tail); }

    inline void TryMergeBlocks(const GoBlock &a)
    {
        this->stone_count += a.stone_count;
        this->stone_state |= a.stone_state;
        this->virtual_liberty_state |= a.virtual_liberty_state;
        this->virtual_liberty_state &= ~this->stone_state;
        this->liberty_state |= a.liberty_state;
        this->liberty_state &= virtual_liberty_state;
    }

    void MergeBlocks(const GoBlock &a)
    {
        GoStone *head = a.GetHead();
        while ( 1 ) {
            head->block_id = this->GetHead()->block_id;
            if ( head->next_id == head->self_id ) {
                break;
            }
            head = stones+head->next_id;
        }
        /* singly linked-list */
        a.GetTail()->next_id = this->GetHead()->self_id;
        this->head = a.head;
        /* disjoint-set */
        a.GetHead()->parent_id = a.GetTail()->parent_id =
            this->GetTail()->self_id;
    }

    /* display for debugging */
    void DisplayBitBoard(BIT_STATE bit_state) const
    {

        FOR_EACH_COORD(id)
        {
            if (id and id % GoConstant::BORDER_C == 0) {
                putchar('\n');
            }
            putchar(bit_state & 1 ? '1' : '0');
            bit_state >>= 1;
        }
        putchar('\n');
    }
    void DisplayStone() const { DisplayBitBoard(stone_state); }
    void DisplayLiberty() const { DisplayBitBoard(liberty_state); }
    void DisplayVirtLiberty() const { DisplayBitBoard(virtual_liberty_state); }

    void DisplayLinkedList() const {
        std::cout << "GoStone linked-list:=======\n";
        std::cout << "Head: " << (int)head << ", Tail: " << (int)tail << "\n"; 
        GoStone *now = this->GetHead();
        while ( 1 ) {
            std::cout << "id: " << (int)now->self_id << "\n";
            std::cout << "next_id: " << (int)now->next_id << "\n";
            std::cout << "block_id: " << (int)now->block_id << "\n";
            if ( now->next_id == now->self_id ) {
                break;
            }  
            now = stones+now->next_id;
        }
        std::cout << "GoStone linked-list:*******\n";
    }
};
#endif