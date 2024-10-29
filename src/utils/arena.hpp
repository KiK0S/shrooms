#pragma once
#include <vector>

namespace arena {

const int SIZE = 1024 * 1024 * 15;
int data_begin = 0;
char data[SIZE];

template <typename T, typename ...Args>
T* create(Args ...args) {
	assert(data_begin + sizeof(T) < SIZE);
	T* ptr = new(data + data_begin) T(args...);
	data_begin += sizeof(T);
	return ptr;
}


}