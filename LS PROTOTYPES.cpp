/* version 1.0 */

#include <algorithm>
#include <vector>
#include <functional>
#include <map>
#include <string>
#include "Comparators.h"

std::multimap<int, std::wstring, ComparatorMapKey> SortMapByValueDescending(const std::map<std::wstring, int>& inputMap) {
	// Create a vector of pairs from the input map
	std::vector<std::pair<int, std::wstring>> sortedVector;
	for (const auto& entry : inputMap) {
		sortedVector.emplace_back(entry.second, entry.first);
	}

	// Sort the vector of pairs in descending order based on the count (number)
	/*std::sort(sortedVector.begin(), sortedVector.end(), [](const auto& a, const auto& b) {
		return a.first > b.first;
		});*/
	std::sort(sortedVector.begin(), sortedVector.end(), ComparatorPair());

	// Create a new map with the sorted elements
	std::multimap<int, std::wstring, ComparatorMapKey> sortedMap;
	for (const auto& entry : sortedVector) {
		sortedMap.insert(entry);
	}

	return sortedMap;
}