#include "utils.hpp"
#include <sys/time.h>
#include <netinet/in.h>
#include <cstdlib>

uint64_t microtime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((uint64_t) tv.tv_sec) * 1000000 + tv.tv_usec;
}

uint64_t htonll(uint64_t value)
{
     int num = 42;
     if(*((const char*) &num) == num)
     {
         uint32_t high = htonl((uint32_t) (value >> 32));
         uint32_t low = htonl((uint32_t) (value & 0xFFFFFFFFLL));
         return (((uint64_t) low) << 32) | high;
     }
     else
         return value;
}
