#pragma once
#include <vector>
#include <cassert>

namespace arena {

const int SIZE = 1024 * 1024 * 15;
int data_begin = 0;
char data[SIZE];

template <typename T, typename ...Args>
inline T* create(Args ...args) {
	assert(data_begin + sizeof(T) < SIZE);
	T* ptr = new(data + data_begin) T(args...);
	data_begin += sizeof(T);
	return ptr;
}

inline bool is_from_arena(void* ptr) {
	return ptr >= data && ptr < data + data_begin;
}

}