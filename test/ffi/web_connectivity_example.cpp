// We want to run the unmodified example through Valgrind
#ifdef ENABLE_INTEGRATION_TESTS
#include "example/ffi/web_connectivity.cpp"
#else
int main(){}
#endif
