#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <stdexcept>

// Custom dynamic array (replacement for std::vector, which the project forbids).
// Memory is managed manually with new[] / delete[].
template <typename T>
class DynamicArray {
private:
    T* data;
    int count;
    int capacity;

    void grow() {
        int newCapacity = (capacity == 0) ? 4 : capacity * 2;
        T* newData = new T[newCapacity];
        for (int i = 0; i < count; i++) {
            newData[i] = data[i];
        }
        delete[] data;
        data = newData;
        capacity = newCapacity;
    }

    void checkIndex(int index) const {
        if (index < 0 || index >= count) {
            throw std::out_of_range("DynamicArray: index out of range");
        }
    }

public:
    DynamicArray() : data(nullptr), count(0), capacity(0) {}

    // Rule of three: copy constructor, copy assignment, destructor.
    DynamicArray(const DynamicArray& other) : data(nullptr), count(0), capacity(0) {
        if (other.count > 0) {
            data = new T[other.count];
            capacity = other.count;
            for (int i = 0; i < other.count; i++) {
                data[i] = other.data[i];
            }
            count = other.count;
        }
    }

    DynamicArray& operator=(const DynamicArray& other) {
        if (this != &other) {
            DynamicArray copy(other);
            T* tmpData = data;
            data = copy.data;
            copy.data = tmpData;
            int tmp = count; count = copy.count; copy.count = tmp;
            tmp = capacity; capacity = copy.capacity; copy.capacity = tmp;
        }
        return *this;
    }

    ~DynamicArray() {
        delete[] data;
    }

    void add(const T& item) {
        if (count == capacity) {
            grow();
        }
        data[count++] = item;
    }

    void removeAt(int index) {
        checkIndex(index);
        for (int i = index; i < count - 1; i++) {
            data[i] = data[i + 1];
        }
        count--;
    }

    T& get(int index) {
        checkIndex(index);
        return data[index];
    }

    const T& get(int index) const {
        checkIndex(index);
        return data[index];
    }

    T& operator[](int index) { return get(index); }
    const T& operator[](int index) const { return get(index); }

    void set(int index, const T& item) {
        checkIndex(index);
        data[index] = item;
    }

    // Pointer to the first element (nullptr if empty). Only for passing the
    // values to C-style APIs such as the chart library; do not keep it.
    const T* rawData() const { return data; }

    int size() const { return count; }
    bool isEmpty() const { return count == 0; }

    void clear() {
        delete[] data;
        data = nullptr;
        count = 0;
        capacity = 0;
    }
};

#endif
