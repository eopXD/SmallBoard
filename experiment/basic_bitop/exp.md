# Expermiment on Bit Operation

## Code

```
#include <cstdlib>
#include <ctime>
#include <bitset>
using namespace std;
int main ()
{
	srand(112358);
	const unsigned long long N = 1e11;
	const unsigned long long pow2[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};

	for ( unsigned long long i=0; i<N; ++i ) {
#ifdef BITSET_1
		bitset<64> bst(rand());
		unsigned long long fetch = 0;
		for ( int j=9; j>=3; j-- ) {
			fetch += bst[j]*pow2[j-3];
		}
#endif
#ifdef BITSET_2
		bitset<64> bst(rand());
		unsigned long long fetch = 0;
		for ( int j=3; j<10; ++j ) {
			fetch += bst[j]*pow2[j-3];
		}
#endif
#ifdef BITSET_3
		bitset<64> bst(rand());
		unsigned long long fetch = 0;
		for ( int j=9; j>=3; j-- ) {
			fetch = fetch<<1 + bst[j];
		}
#endif
#ifdef BIT_OP
		unsigned long long x = rand();
		unsigned long long fetch = (x>>53)&((1<<7)-1);
#endif
	}
	return (0);
}
```

## Compilation

```
all:
	g++ -std=c++11 -D BIT_OP -O2 test.cpp -o test_bitop
	g++ -std=c++11 -D BITSET_3 -O2 test.cpp -o test_bitset3
	g++ -std=c++11 -D BITSET_2 -O2 test.cpp -o test_bitset2
	g++ -std=c++11 -D BITSET_1 -O2 test.cpp -o test_bitset1
```



## Result

I run the run code 10 times each.

| N=1e9 | BIT_OP | BITSET_1 | BITSET_2 | BITSET_3 |
|---|--------|----------|----------|----------|
|Rank|	1|	2|	4|	3||Average	|6.7111	|6.6941|	6.6588	|6.6603||Standard Deviation	|0.079628931	|0.034468826	|0.084178118	|0.06651157|


| N=1e10 | BIT_OP | BITSET_1 | BITSET_2 | BITSET_3 |
|---|--------|----------|----------|----------|
|Rank|	3|	2|	1|	4||Average|	71.3118|	71.498|	71.8427	|71.2525||Standard Deviation|	0.480812923	|1.209239614	|2.904639624	|1.088619028|				
| N=1e11 | BIT_OP | BITSET_1 | BITSET_2 | BITSET_3 |
|---|--------|----------|----------|----------|
| Rank	|4	|1	|2	|3||Average	|708.5203	|713.3967	|711.4905	|709.8945||Standard Deviation	|8.040931262	|8.485305037|	7.067016127|	10.37166017|


## Conclusion

Bit operation performs the stable and fastest. The performance using bit-op on `std::bitset` or a full `uint64_t` shows not much difference.

More detail can be seen in the spreadsheet.

I will use `std::bitset` to interact with disk.
