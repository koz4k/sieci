#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdint>

uint64_t microtime();
uint64_t htonll(uint64_t value);
inline uint64_t ntohll(uint64_t value) { return htonll(value); }

#endif
