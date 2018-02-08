// We want to run the unmodified example through Valgrind
#ifdef ENABLE_INTEGRATION_TESTS
#include "example/ffi/ndt.cpp"
#else
int main(){}
#endif
