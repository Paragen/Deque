#include <string>
#include <fstream>
#include <array>
#include <type_traits>
#include <assert.h>
#include <vector>
#include <gmpxx.h>
#include "deque"
#include "util.h"

#ifndef DEQUE_DUMB_EXTERNAL_DEQUE_H
#define DEQUE_DUMB_EXTERNAL_DEQUE_H
using std::string;

template <class T>
class dumb_external_deque {
    static constexpr unsigned block_size = 8 * 1024 * 1024 / sizeof(T);
    const string prefix;
    static const string delimiter;
    unsigned left_size = 0, right_size = 0, left_edge = 0, right_edge;
    mpz_class data_size;


public:

    class iterator;

    dumb_external_deque(const string& root);
    dumb_external_deque(const dumb_external_deque& ) = delete;

    void push_back(const T& object);
    void push_front(const T& object);

    void pop_back();
    void pop_front();

    mpz_class size() const;

    dumb_external_deque<T>::iterator begin();
    dumb_external_deque<T>::iterator end();

    ~dumb_external_deque();
};

template <class T>
class dumb_external_deque<T>::iterator {
    unsigned shift, block_num;
    const string prefix;
public:

    iterator(unsigned block_num, unsigned shift, const string& prefix):block_num(block_num),shift(shift),prefix(prefix){}

    iterator& operator++() {
        if (block_size <= shift + 1) {
            shift = 0;
            ++block_num;
        } else {
            ++shift;
        }
        return *this;
    }

    iterator& operator--() {
        if (shift <= 0) {
            shift = block_size - 1;
            --block_num;
        } else {
            --shift;
        }
        return *this;
    }

    T operator*() {
        return get_from_file<T>(prefix + std::to_string(block_num) , shift);
    }

    bool operator==(const iterator& another) {
        return block_num == another.block_num && shift == another.shift;
    }

    bool operator!=(const iterator& another) {
        return !(*this == another);
    }
};

template <class T>
const string dumb_external_deque<T>::delimiter = "data";

template <class T>
dumb_external_deque<T>::dumb_external_deque(const string &root):prefix(root + separator() + std::to_string(
        reinterpret_cast<intptr_t>(this))){
}

template <class T>
void dumb_external_deque<T>::push_front(const T &object) {
    if (left_size + 1 == block_size) {
        --left_edge;
        left_size = 0;
    } else {
        ++left_size;
        if (left_edge == right_edge) {
            ++right_size;
        }
    }
    add_to_file_begin(prefix + delimiter + std::to_string(left_edge), object);

    ++data_size;
}

template <class T>
void dumb_external_deque<T>::push_back(const T &object) {
    if (right_size + 1 == block_size) {
        ++right_edge;
        right_size = 0;
    } else {
        ++right_size;
        if (left_edge == right_edge) {
            ++left_size;
        }
    }
    add_to_file_end(prefix + delimiter + std::to_string(right_edge), object);

    ++data_size;
}
template <class T>
void dumb_external_deque<T>::pop_back() {
    remove_from_file<T>(prefix + delimiter + std::to_string(right_edge), true);
    if (right_size == 0) {
        std::remove((prefix + delimiter + std::to_string(right_edge)).c_str());
        --right_edge;
        right_size = block_size - 1;
    } else {
        --right_size;
        if (left_edge == right_edge) {
            --left_size;
        }
    }

    --data_size;
}

template <class T>
void dumb_external_deque<T>::pop_front() {
    remove_from_file<T>(prefix + delimiter + std::to_string(left_edge), false);
    if (left_size == 0) {
        std::remove((prefix + delimiter + std::to_string(left_edge)).c_str());
        ++left_edge;
        left_size= block_size - 1;
    } else {
        --left_size;
        if (left_edge == right_edge) {
            --right_size;
        }
    }

    --data_size;
}

template <class T>
mpz_class dumb_external_deque<T>::size() const {
    return data_size;
}

template <class T>
typename dumb_external_deque<T>::iterator dumb_external_deque<T>::begin() {
    return dumb_external_deque<T>::iterator(left_edge, 0, prefix + delimiter);
}

template <class T>
typename dumb_external_deque<T>::iterator dumb_external_deque<T>::end() {
    return dumb_external_deque<T>::iterator(right_edge, right_size, prefix + delimiter);
}

template <class T>
dumb_external_deque<T>::~dumb_external_deque() {
    for (auto i  = left_edge; i != right_edge + 1; ++i) {
        std::remove((prefix + delimiter + std::to_string(i)).c_str());
    }
}


#endif //DEQUE_DUMB_EXTERNAL_DEQUE_H
