#pragma once

#include <vector>
#include <assert.h>

namespace shl {
	template<typename T>
	inline bool isSorted(T* arr, size_t size) {
		for (size_t i = 1; i < size; ++i) {
			if (arr[i] < arr[i - 1]) {
				return false;
			}
		}
		return true;
	}
	template<typename T>
	inline bool isSorted(std::vector<T>& arr) {
		return isSorted(arr.data(), arr.size());
	}
	
	template<typename T>
	inline void reverseArray(T* arr, size_t size) {
		size_t half = size / 2;
		for (size_t i = 0; i < half; ++i) {
			std::swap(arr[i], arr[size - i]);
		}
	}
	template<typename T>
	inline void reverseArray(std::vector<T>& arr) {
		reverseArray(arr.data(), arr.size());
	}
	
	template<typename T>
	inline void insertionSort(T* toSort, size_t size) {
		for (size_t i = 1; i < size; ++i) {
			T key = toSort[i];
			size_t j = i;
			for (; j > 0 && toSort[j-1] > key; --j) {
				toSort[j] = toSort[j-1];
			}
			toSort[j] = key;
		}
	}
	template<typename T>
	inline void insertionSort(std::vector<T>& toSort) {
		insertionSort(toSort.data(), toSort.size());
	}

	template<typename T>
	inline size_t pickPivot(T* toSort, size_t start, size_t end) {
		assert(start < end);
		size_t mid = start + (end - start) / 2;
		if (toSort[start] < toSort[end]) {
			if (toSort[end] < toSort[mid]) return end;
			else if (toSort[start] < toSort[mid]) return mid;
			else return start;
		}
		else {
			if (toSort[start] < toSort[mid]) return start;
			else if (toSort[end] < toSort[mid]) return mid;
			else return end;
		}
	}
	template<typename T>
	inline size_t swapAroundPivot(T* toSort, size_t start, size_t end, size_t pivot) {
		assert(start < end);
		assert(pivot <= end);
		assert(start <= pivot);
		std::swap(toSort[pivot], toSort[end]);
		size_t smaller = end - 1;
		size_t greater = start;
		while (true) {
			while (start < smaller && toSort[smaller] > toSort[end]) {
				--smaller;
			}
			while (greater < end && toSort[end] >= toSort[greater]) {
				++greater;
			}
			if (greater >= smaller)
				break;
			std::swap(toSort[greater], toSort[smaller]);
		}
		std::swap(toSort[greater], toSort[end]);
		return greater;
	}
	template<typename T>
	inline void quickSortSlice(T* toSort, size_t start, size_t end) {
		if (start < end) {
			size_t pivot = pickPivot(toSort, start, end);
			pivot = swapAroundPivot(toSort, start, end, pivot);
			quickSortSlice(toSort, start, pivot - 1);
			quickSortSlice(toSort, pivot + 1, end);
		}
	}
	template<typename T>
	inline void quickSort(T* toSort, size_t size) {
		quickSortSlice(toSort, 0, size - 1);
	}
	template<typename T>
	inline void quickSort(std::vector<T>& toSort) {
		quickSortSlice(toSort.data(), 0, toSort.size() - 1);
	}

	template<typename T>
	inline void bubbleSort(T* toSort, size_t size) {
		for (size_t i = 1; i < size; ++i) {
			bool swapped = false;
			for (size_t j = 0; j < size - i; ++j) {
				if (toSort[j] > toSort[j+1]) {
					std::swap(toSort[j], toSort[j + 1]);
					swapped = true;
				}
			}
			if (!swapped)
				return;
		}
	}
	template<typename T>
	inline void bubbleSort(std::vector<T>& toSort) {
		bubbleSort(toSort.data(), toSort.size());
	}
	
}