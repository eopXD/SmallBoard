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

##### Finished basic structure on `comm`.

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


##### Finish basic structure of GoBlock

Notable

- ❗️ if `SMALLBOARDSIZE` exceeds `64`, then there will be modification needed on bitstate storage of stone and liberty


## 2019/11/24

Measured performance on basic bit operation. I have to admit that the overhead is not as huge as expected when I concatenate the two operations in one code. The overhead seems to be small enough to emit... **_Maybe it will show greater difference when I aggregate the operation to more complicate structure._**

The `GoBitState` can be established now since I have done the experiment.

Also I started to build the move generation on `GoBoard`. The main operation is the move generation of the Board. To build the move generation, I hence found out that `GoStone` structure is crucial because I need to know which `GoBlock` this stone belongs to. This means that I need to do a report on the structure on how much space a `GoBoard` takes.

The move generation relies on the maintenance of **liberty(氣)**, which is maintained by `GoBlock`.

For a stone to be placed on some position on the board, it first check on its neighboring blocks to see whether it is an illegal move (如果那個地方下下去的話是會被吃的動作，就不能動那邊，所以要知道 neighbor block 是屬於誰的，要知道 neighbor block 資訊，這個 block 中含有的 stone 有哪些就也必須知道) 在實作的過程中一路往最底層蓋...

- 原本要蓋的是 move generation 函數: `TryMove`
- 因為要知道相鄰的 `GoBlock` 所以需要函數: `GetNeighborBlocks`
- 因為要知道這個 `GoBlock` 的 `GoBlockId`，所以需要函數: `GetBlockIdByCoord`
- 對這個 `GoBlock` 我需要知道他是誰，這個資訊被記錄在 ancestor stone （因為 stone 以 linked-list 的形式記錄起來) 所以需要函數: `FindCoord`

#### Notable ToDo

- [ ] Do measurement on `GoState` performance

- [ ] Do report on space on `GoBoard`

## 2019/11/25

Notable

- since i have id numbered from $[0, R \times C)$, so a board with no Ko will have `ko_position = R*C`, `pass_count = 0`.
- GoScore is unsigned integer with its range from $[0, 2 \times R \times C]$, where $R \times C$ represents $0$ and $0$ represents black loses the most... $-R \times C$

#### Compilation

- comm

```
g++ -std=c++11 -D ROW=5 -D COL=5 -c comm.cc
```

This compiles `comm.h` into object file that is link-able when we are going to compile an executable.

Today I am implemeting `GoState`, the structure to interact disk I/O.

I face 2 decisions, whether to do it in `bitset` or `uint64_t`. Let me list out the Pros and Cons...

##### `std::bitset`

Strength:

- compact, saves memory

Weakness:

- encode/decode goes through for-loop
- `reset` of a `std::bitset` is called by sending a pointer of the bitset to `memset`.
- need to transfer `bitset` to `uint64_t` when sorting.

##### `uint64_t`

Strength:

- sortState does not need to go through a transformation from `std::bitset`

Weakness:

- overhead of the extra memories (take example of 5x5 smallboard, the overhead is 7/64 ~ 10%)

##### 小結：在不同的 stage 使用不同的 GoState 感覺並不衝突。所以到時因應需求來使用需要的 GoState.



#### parameters I specified in `makefile`

- `ROW`: column of the board
- `COL`: row of the board
- `ENCODE_LENGTH`: the encoding length that can be calculated via easy math.

今天把 GoState 蓋好了！算是生產力不錯？XD 接下來就繼續蓋 `TryMove` ，目標是明天把 `TryMove` 完成。

把 `TryMove` 完成之後可以開始進行 SmallBoard 實驗的，在蓋實驗的過程中應該就會知道需要再多補充哪些函數了。

## 2019/11/26

今天要把 `TryMove` 還有 `GetPossibleMove` 完成。突然想到，在做 Retrograde Analysis 的時候，需要有 `GetPossibleParent`。

值得注意的是，

- `GoBoard::GetNeighborBlocks`，這個函數是要幫一個 `blk` 以及 `target_id` 來取得 `target_id` 周圍的 `GoBlock` ，這個函數也會在 `blk` 中的 stone_state 在 `target_id` 的位置加上一顆石頭。
- 如果 neighbor_block 只剩一個氣，然後我下在那裡就表示我把對方吃掉了，這樣我的 block 連在一起的東東也增加了。

剛剛做完 possible move generation ，接著做 actual moving ，然後再來是跟 GoState 之間的互動。

每個 `GoBoard` 裡面有 `SMALLBOARDSIZE` 這麼多個 `GoStone`， 石頭是用 Circular linked-list 串連起來的。然後 head 被記在 `GoBlock` 裡頭。

#### `GoBoard::Move`

`GoBoard::Move` 跟 `GoBoard::TryMove` 很像。

因為已經判斷出有哪些是合法的走步了，所以只要實際的把 board detail 做改變即可。其中複雜度很高的部分，是會把對方的 `GoBlock` 吃掉，而這部分對吃掉的 Block 需要看這個 Block 裏頭每個 stone 的 neighbor 是否含有友方 block ，如果是友方 block 的話需要更新他的 liberty 狀態。（見下方 code segment）

```
for ( GoBlockId i=1; i<=die_id[0]; ++i ) {
	if ( 1 == block_pool[die_id[i]] and 1 == blk.stone_count 
	 and 1 == blk.CountLiberty() ) {
	// this is a Ko!
		ko_position = block_pool[die_id[i]].head;
	}
	// for the stones of the killed GoBlock...
	FOR_BLOCK_STONE(dead_stone, block_pool[die_id[i]], 
		// for the neighbor of the stones
		FOR_NEIGHBOR(dead_stone, nb) {
			// if the neighbor is my color, add liberty to my stone
			if ( SelfColor() == board_state[*nb] ) {
				GoBlockId my_blk_id = GetBlockIdByCoord(*nb);

				visited_position[my_blk_id] = game_length;
				block_pool[my_blk_id].SetLiberty(dead_stone);
			}
		}
		board_state[dead_stone] = EmptyStone;
		current_zobrist_value ^= 
		 zobrist_board_hash_weight[OpponentColor()][dead_stone];
	);
	RecycleBlock(die_id[i]);
}
```

#### Notable ToDo

- [ ] `GetPossibleParent`


#### Interesting finding

##### Is it meanful to optimize swap?

CPU speeds compared to memory speeds have risen astronomically. Accessing memory has become the major bottleneck in application performance. All the swap algorithms will be spending most of their time waiting for data to be fetched from memory. Modern OS's can have up to 5 levels of memory:

- Cache Level 1 - runs at the same speed as the CPU, has negligible access time, but is small
- Cache Level 2 - runs a bit slower than L1 but is larger and has a bigger overhead to access (usually, data needs to be moved to L1 first)
- Cache Level 3 - (not always present) Often external to the CPU, slower and bigger than L2
- RAM - the main system memory, usually implements a pipeline so there's latency in read requests (CPU requests data, message sent to RAM, RAM gets data, RAM sends data to CPU)
- Hard Disk - when there's not enough RAM, data is paged to HD which is really slow, not really under CPU control as such.


##### 繼續寫進度⋯⋯

今天也順便完成了 `GoBoard` 的 constructor, destructor, 還也 copy constructor.


## 2019/11/27

今天開會討論到 BitState 的編碼，原本的編碼是 58 bit，但是其中 ko position 可以拆開來做，一個 serial number 最多不會有太多 ko （至多 6 個）因此在 ko position 的 bitstate 儲存方面可以思考其他的變化。

### Explanation on saving states containing Ko

一個盤面是以一個 serial number 來表示。一個盤面也可能同時在多個地方有 Ko 。在這裏的討論我們如果只關心 SerialNumber 還有 KoPass 的記錄。

#### Ko in master thesis

對一個盤面，假設這個盤面有 $k$ 種合法 Ko 位置，則碩論裡頭就會儲存 $k$ 個 BoardState 。這樣的花費是：

- $k \times $ (40 + log2(`BOARDSIZE+2`)) bit 

#### Space for Improvement

這裏學長他把 KoPass 綁在一起了，可是這樣其實很浪費，因為這樣等於是重複的記錄了 Serial 的部分。

當然在 KoPass 的值不一樣時...Degree, Game-Result, Score 也會不一樣，不過可以避免重複的方法就是把 KoPass 作為一個 object 抽出來。

#### Withdraw KoPass as an Object

好處：

- 省掉重複記錄 serial

壞處：

- linkage from serial number to its corresponding KoPass informations (這裏牽涉到 memory access ，因此 KoPass 的儲存也需要好好考慮）

KoPass 抽出來作為物件最簡單有兩種方法：（以 `BOARDSIZE = 25` 來做例子）
 
- 照著原本的紀錄，並且同個 Board 的 Ko 之間以 linked-list 串連。這樣子的話，這樣一個 KoPass 記錄要耗費 5 bit 。
- 用 0/1 bit array 來儲存對該個 serial board 有哪些位置可以是 Ko ，這樣耗費 25 bit 。

兩者的比較的話，在 BOARDSIZE = 25 時，第一個方法是相對實作簡單，且 trade-off 較省的，因為鮮少出現有 5 個 Ko 的 State 。

但是當 BOARDSIZE 延展到更多的時候，因為以 bit array 來儲存 PossibleKoPosition 的話，其實是以最低的 Entropy 來處理這個問題，並且作法可以擴充到任意的 BoardSize 。但是第一個方法就無法擴充，因為會耗費相較第二個方法更多的記憶體。（在盤面小的時候還適用，但是盤面變大時要用的記憶體大於 64 bit 的話也會變得難以操作）

