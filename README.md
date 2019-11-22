## Structure

```
GoStone --> GoBlock -> GoBoard
```

The game board, `GoBoard` are consists of `GoBlock`s, and `GoBlock` is consist of `GoStone`s.

It seems like we can reduce on recording stone details on the board.
The Block structure shall not be reduced because it is crucial for recording the liberty.

## `GoState`

`GoState` is the bit state we are saving to small boards. Needed bits are are calculated with `GoBitState::GenNeededBits`.


## Zobrist Hash

The Z-Hash is used to check if the board position is repeated. In our small board analysis, Positional SuperKo is allowed, meaning that we only have to check whether there is a Ko in our move to determine whether the move is legal.

The Z-Hash is initially generated using `GoFunction::CreateZobristHash`, with fixed `uint32_t seed = 0xdeadbeef;`. We prepare Z-Hash for the every possible stone of appearance on the board, which includes "Empty"(0), "Black"(1), "White"(2).
