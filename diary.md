## 2019/11/06

Before the starting date I have read the master thesis "Memory efficient algorithms and implementations for solving small-board-sized Go". The program of this thesis works correctly but rather inefficient and have space for improvement.

I will implement a efficient and succint program on calculating small boards of Go for further usage of Monte-Carlo or Neural Network.

## 2019/11/08

The class hierarchy and methods are developed with me learning from [PheonixGo](https://github.com/Tencent/PhoenixGo/), which is an implementation on `AlphaGo Zero` and the paper "Mastering the Game of Go without Human Knowledge". Tracing through the repository may help you to understand my code.

## 2019/11/20

I have finished works of being a Teaching Assistant of "Theory of Computer Game".(prepare midterm exam) This has been an inproductive 10 days for me. Since I have now gotton the image that there is massive workload waiting for me, that I may write up to 4000 lines of code to implement this SmallBoard Experiment.

Now I need to keep-on the trace and implementation.

以上廢話，接下來進入正題⋯⋯

- since the experiment allows superko, we only need to check if the the move violates **_"Basic Ko"_**, so we don't need unorder_map to record, but rather a game history of length 4. 這裡選長度為 4 是因為這樣 access 時，做同餘可以用 bit operation 代替。

- PheonixGo 中 GoState 含有若干個 GoBlock ，並且可以 reuse 。

```
once_flag CreateGlobalVariables_once;
void CreateGlobalVariables() {
    call_once(
        CreateGlobalVariables_once,
        []() {
            CreateNeighbourCache();
            CreateHashWeights();
            CreateQuickLog2Table();
            CreateZobristHash();
        }
    );
}
```

- 上面這段 code 中 `once_flag` 加上 `call_once` 保證以下這些 function 只被呼叫一次。以下四個 `CreateXXX` 是在 `go_comm.h` 中被定義的全域變數。這個技巧可以應用在之前的 `SuccinctWT` 來調整程式的可讀性 

- 最重要的是要記住為什麼我需要重寫 SmallBoard ，要增加編碼的 locality ，所以需要調查有無 Ko 的情況下盤面的分佈。

- 估計這應該一個月可以完成。

- 在 `GoState::GetSensibleMove` 中，進行一個 move (placing a new stone) 代表潛在 GoBlock 的增加

- `g_neighbour_cache_by_id` 是預處理該 id 四個方位的鄰居的相應 id ，省去每次搜過去鄰居時都需要的計算

## 2019/11/21

繼續幹！

### comm

`comm.h` is the shall be the first to be defined.

- 首先要完成的是 `comm.h/comm.cc` 這兩個檔案。
- 裡頭定義了一些預處理，只會處理一次的東東，像是 Zobrist Hash 中會用到的固定 hash 值，以及預先計算好的 id 所對應的 neighbor 的 id ，或是盤面上 (x, y) 所會有的 neighbor (x, y)。
- 另外還有一些簡易的 utility function ，會在標頭檔中含有補充說明。

- `cached_neighbor` 中並不是 [0]~[3] 依照四個方位，而是 compact 的，也就是搞不好只有 2 個或 3 個方位 available 。接下來對 neighbor 的詢問不需要再去判邊界。這也是為什麼有 `cached_neighbor_size` 來記錄總共有幾個在界內的方位。

### timer

the class `Timer` us constructed to measure performance.

## 2019/11/22

Finished basic structure on `comm`.

Notable

- Error logger - elog()
- GoBitState: The structure associates I/O with disk. Information stored 
	  compact and MSB style.
- GoConstant: Type definitions of variables, and constants of the board,
	  including the SmallBoard size.
- GoFunction: Utility functions for some judgement on the board. Also 
	  there are preprocessed tables used for Zobrist Hash and precalculated
	  table that corresponds (id/coordinate) to its neighbor (id/coordinate).
	  
`timer` also done.
