#pragma once
/* version 1.0 */
#include <map>

struct ComparatorPair {
    bool operator()(const std::pair<int, std::wstring>& a, const std::pair<int, std::wstring>& b) const {
        if (a.first != b.first) {
            return a.first > b.first;               // Sort in descending order based on values
        }
        return a.second < b.second;                 // Maintain order for equal values based on keys
    }
};

struct ComparatorMapKey {
    bool operator()(const int& a, const int& b) const {
        return a > b;                               // Sort in descending order based on values
    }
};