#include "comm.h"
#include "GoState.h"

#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;
using namespace GoBitState;

#define L first
#define R second

int main ()
{
	CreateEncoding();
	for ( int i=0; i<5; ++i ) {
		cout << (uint32_t)encode_size[i] << "\n";
		cout << (uint32_t)encode_lr[i].L << " " << (uint32_t)encode_lr[i].R << "\n";
	}
	
	GoState x;
	x.set_serial(127);
	cout << x.bitstring() << "\n";

	x.set_serial(5);
	cout << x.bitstring() << "\n";

	return (0);
}

#undef R
#undef L