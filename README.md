## 2019/11/06

Before the starting date I have read the master thesis "Memory efficient algorithms and implementations for solving small-board-sized Go". The program of this thesis works correctly but rather inefficient and have space for improvement.

I will implement a efficient and succint program on calculating small boards of Go for further usage of Monte-Carlo or Neural Network.

## 2019/11/08

The class hierarchy and methods are developed with me learning from [PheonixGo](https://github.com/Tencent/PhoenixGo/), which is an implementation on `AlphaGo Zero` and the paper "Mastering the Game of Go without Human Knowledge". Tracing through the repository may help you to understand my code.

## Structure

```
GoStone --> GoBlock -> GoBoard
```

The game board, `GoBoard` are consists of `GoBlock`s, and `GoBlock` is consist of `GoStone`s.

## `GoState`

`GoState` is the bit state we are saving to small boards.

## Zobrist Hash

The Z-Hash is used to check if the board position is repeated. In our small board analysis, Positional SuperKo is allowed, meaning that we only have to check whether there is a Ko in our move to determine whether the move is legal.

The Z-Hash is initially generated using `GoFunction::CreateZobristHash`, with fixed `uint32_t seed = 0xdeadbeef;`. We prepare Z-Hash for the every possible stone of appearance on the board, which includes "Empty"(0), "Black"(1), "White"(2).

