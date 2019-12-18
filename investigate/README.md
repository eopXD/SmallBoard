Investigate on data by the master thesis to assert that my program is correct.

File explanation for HC's master thesis: (you can find the original files under `/tmp2/b01902059/4x4/smallBoardResult`)

- `legalBoard`: raw legal serial number
- `compressedBoard`: merged legal serial numbers
- `koStates`: reads `compressedBoard` and outputs "serial and the ko" if the serial can occur a ko.
- `legalState`: merges the `compressedBoard` and `koStates`, for every serial in `compressedBoard`, push `pass=0` and `pass=1`. (**`pass=2` is comment out, but total real state number in the powerpoint is `9276006`, which seems impossible without counting in `pass=2`, I will further investigate on this**)


There are some programs in this folder, here are some brief explanations for guidance.

- `assert_legal_board.cpp`: I read through `4x4` data from HC master thesis, and asserted that my judge on all serial numbers match with the thesis.
- `display_hc_ko.cpp`: decode the data `koState` uncompressed from `white.iis.sinica`
- `my_gen_ko.cpp`: since it only takes 7 secondsm it is easy to find all ko states from the reduced legal boards, this file find them and displays the board and its ko position
- `simple_query.cpp`: query a serial number, displays the board and its possible ko positions.
