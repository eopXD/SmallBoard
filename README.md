## Structure

```
GoStone --> GoBlock -> GoBoard
```

The game board, `GoBoard` are consists of `GoBlock`s, and `GoBlock` is consist of `GoStone`s.

I do static allocation of `GoBlocks` and `GoStone` per `GoBoard`.

`GoState` is the interface to react between the saved states. 

## Zobrist Hash

The Z-Hash is used to check if the board position is repeated. In our small board analysis, Positional SuperKo is allowed, meaning that we only have to check whether there is a Ko in our move to determine whether the move is legal.

The Z-Hash is initially generated using `GoFunction::CreateZobristHash`, with fixed `uint32_t seed = 0xdeadbeef;`. We prepare Z-Hash for the every possible stone of appearance on the board, which includes "Empty"(0), "Black"(1), "White"(2).

## $1_{st}$ Phase: Find All Possible Serial

Find all possible serial numbers for the given board size.

This phase I created a constructor that takes serial numbers in. It calles `GoBoard::SetStone` to place the stones onto the board without maintaining gaming details of the board. (so it is a modification version of `GoBoard::Move`). If anything eat or self-eat move happens when `SetStone`, the construction will end and the for-loop in `FindAllPossibleSerial/main.cpp` will go on to try the next serial number.

### Remarks

Actually not that many function is related in this phase, because I only want to initialize the board. However the correctness of this phase also have asserted that `GoBlock` and `GoStone` maintenance is **bug-free**.

### Some Psuedo Codes

- for all possible `serial`...
	- `GoBoard(serial)`
		- calls `SetStone` for each stone on the `serial`, which the `GoStone` is on position `id`
			- Create a new `GoBlock`
			- Get neighboring `GoBlocks` of `id` using `GetNeighborBlocks`
				- `GetNeighborBlocks` calls `GetBlockIdByCoord` for each neighboring position
					- `GetBlockIdByCoord` calls `FindCoord`, which stones are herded together using **disjoint-set**
			- If neighboring block is of the same color, call `GoBlock::TryMergeBlock`
			- If neighboring block is opponent and with only 1 liberty left, the construction fails (**ERROR CODE -2**)
			- If the merged block of my own have no liberty, then the construction fails (**ERROR CODE -1**)
			- After `GoBlock::MergeBlock`, use `GoBoard::RecycleBlock` to recycle the blocks.


