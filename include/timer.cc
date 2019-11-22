#include "timer.h"

using namespace std::chrono;


Timer::Timer () : clock_start(clock::now) {
}
void Timer::Reset () {
	timestamp = clock::now;
}

int64_t Timer::sec () const {
	return (duration_cast<seconds>(clock::now-timestamp).count());
}
int64_t Timer::ms () const {
	return (duration_cast<milliseconds>(clock::now-timestamp).count());
}
int64_t Timer::us () const {
	return (duration_cast<microseconds>(clock::now-timestamp).count());
}

int64_t Timer::fsec () const {
	return (duration_cast<float>(clock::now-timestamp).count());
}
int64_t Timer::fms () const {
	return (duration_cast<float, std::milli>(clock::now-timestamp).count());
}
int64_t Timer::fus () const {
	return (duration_cast<float, std::micro>(clock::now-timestamp).count());
}
