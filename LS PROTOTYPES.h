#pragma once
#ifndef LS_PROTOTYPES
#define LS_PROTOTYPES

#include <algorithm>
#include <random>
#include <vector>
#include "Comparators.h"

std::multimap<int, std::wstring, ComparatorMapKey> SortMapByValueDescending(const std::map<std::wstring, int>& inputMap);

template <typename T>
std::vector<T> SelectRandomElements(const std::vector<T>& input, int N) {
	N = (std::max)(N, 0);														//(std::max) to avoid evaluating macro
	int size = static_cast<int>(input.size());
	N = (std::min)(N, size);
	if (N == 0 || input.empty()) return std::vector<T>();
	std::vector<T> shuffledInput = input;                                       // Create a copy of the input vector
	std::random_device rd;                                                      // Obtain a random seed from the random device
	std::default_random_engine rng(rd());                                       // Use the random seed to initialize the random number generator
	std::shuffle(shuffledInput.begin(), shuffledInput.end(), rng);              // Shuffle the elements in the copied vector 
	std::vector<T> output(shuffledInput.begin(), shuffledInput.begin() + N);    // Create the output vector containing the first N elements from the shuffled vector
	return output;
}

template <typename T>
std::vector<T> RemoveRandomElements(std::vector<T>& input, int N) {
	N = (std::max)(N, 0);														//(std::max) to avoid evaluating macro
	int size = static_cast<int>(input.size());
	N = (std::min)(N, size);
	if (N == 0 || input.empty()) return std::vector<T>();
	std::random_device rd;                                                      // Obtain a random seed from the random device
	std::default_random_engine rng(rd());                                       // Use the random seed to initialize the random number generator
	std::shuffle(input.begin(), input.end(), rng);								// Shuffle the elements in the input vector randomly
	std::vector<T> output(input.begin(), input.begin() + N);					// Create the output vector containing the first N elements from the shuffled input vector
	input.erase(input.begin(), input.begin() + N);								// Erase the first N elements from the input vector
	return output;
}

#endif // !LS_PROTOTYPES
