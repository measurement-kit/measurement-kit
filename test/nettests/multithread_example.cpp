// We want to run the unmodified example through Valgrind
#ifdef ENABLE_INTEGRATION_TESTS
#include "example/nettests/multithread.cpp"
#else
int main(){}
#endif
