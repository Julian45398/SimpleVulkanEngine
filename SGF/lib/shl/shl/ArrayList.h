#pragma once


#ifndef SHLIB_USE_CUSTOM
#include <vector>
template <typename T>
using ArrayList = std::vector<T>;
#else
#error Custom Array list currently not in functional state
template<typename T>
class ArrayList
{
private:
	T* m_buffer;
	size_t m_size;
public:
	ArrayList() {
		m_size = 0;
		m_buffer = nullptr;
	}

	ArrayList(size_t size) {
		m_size = size;
		m_buffer = arrayAlloc(T, size);
	}

	ArrayList(size_t array_size, const T* static_array) {
		m_size = array_size;
		m_buffer = arrayAlloc(T, array_size);
		memcpy(m_buffer, staticArray, array_size);
	}

	ArrayList(const ArrayList<T>& toCopy) {
		m_size = toCopy.size();
		m_buffer = arrayAlloc(T, m_size);
		for (size_t i = 0; i < m_size; i++) {
			m_buffer[i] = toCopy[i];
		}
	}
	
	// Gives direct access to Value
	// No check if index is out of bounds in release mode
	T& operator[](const size_t index) {
		asser
		return m_Buffer[index];
	}

	void operator=(const ArrayList<T>& toCopy) {
		setSize(toCopy.m_size);
		m_size = toCopy.m_size;
		m_buffer = arrayAlloc(T, m_size);
		for (size_t i = 0; i < m_Size; i++) {
			m_buffer[i] = toCopy[i];
		}
	}

	const T* data() {
		return m_buffer;
	}

	// Returns the Value of the Array without giving direct access
	// Checks if index is inside Bounds
	// Requires Testing in Debug-Mode to make sure the index is always inside bounds
	T get(const size_t index) {
		if (index < m_size) {
			return m_buffer[index];
		}
		else {
			logWarn("Index: ", index, " is out of bounds for size: ", m_size - 1);
		}
	}

	// Returns true if 
	bool push_back(const T& toAdd) {
		T* buffer = arrayResize(m_buffer, T, m_size + 1);
		if (buffer == nullptr) {
			logError("Failed to add element to Array-List");
			return false;
		}
		m_buffer = buffer;
		m_buffer[m_size] = toAdd;
		m_size++;
		return true;
	}

	// Safe version to set a value
	bool set(const size_t index, const T& value) {
		if (index < m_size) {
			m_buffer[index] = value;
			return true;
		}
		else {
			logWarn("Index: ", index, " is out of bounds for size: ", m_size - 1, " of Array-List");
		}
		return false;
	}

	void clear() {
		m_size = 0;
		arrayFree(m_buffer);
	}

	bool isEmpty() {
		return m_size == 0;
	}

	// Changes the size of the Array without changing the values
	bool resize(size_t size) {
		T* buffer = arrayResize(m_buffer, size);
		if (buffer == nullptr) {
			Log_Error("Failed to set Size of Array-List to: ", size);
			return false;
		}
		m_buffer = buffer;
		m_size = size;
		return true;
	}

	bool grow(size_t amount = 1) {
		amount = amount + m_size;
		T* buffer = arrayResize(m_buffer, amount);
		ASSERT(buffer != nullptr);
		m_size = amount;
		m_buffer = buffer;
	}

	void shrink(size_t amount = 1) {
		if (m_size < amount+1) {
			clear();
		}
		else {
			m_size -= amount;
			T* buffer = arrayResize(m_buffer, m_size);
			ASSERT(buffer != nullptr);
			m_buffer = buffer;
		}
	}

	size_t size() { 
		return m_size;
	}
};
#endif