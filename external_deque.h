#include <deque>
#include <gmpxx.h>
#include <string>
#include "util.h"
#include <memory>
#include <map>


#ifndef DEQUE_EXTERNAL_DEQUE_H
#define DEQUE_EXTERNAL_DEQUE_H

using std::string;

template<class T>
class external_deque {

    static constexpr unsigned block_size = 8 * 1024 * 1024 / sizeof(T);
    const string prefix;
    static const string delimiter;
    unsigned left_edge = 0, right_edge = 0;
    mpz_class data_size = 0;
    std::deque<T> *left_block, *left_block_next = nullptr, *right_block, *right_block_next = nullptr;

    std::map<unsigned, std::pair<int, std::deque<T>>> loaded_blocks;

    std::deque<T> *upload(unsigned number);

    void drop(unsigned number);

public:

    class iterator;

    external_deque(const string &root);

    external_deque(const external_deque &) = delete;

    void push_back(const T &object);

    void push_front(const T &object);

    void pop_back();

    void pop_front();

    mpz_class size() const;

    external_deque<T>::iterator begin();

    external_deque<T>::iterator end();

    ~external_deque();
};

template<class T>
class external_deque<T>::iterator {
    external_deque<T> *host;
    unsigned shift, block_num, block_num_prev = 0;
    std::deque<T> *block, *block_prev = nullptr;

public:

    iterator(unsigned block_num, unsigned shift, external_deque<T> *host) : block_num(block_num), shift(shift),
                                                                            host(host) {
        block = host->upload(block_num);
    }

    iterator &operator++() {
        if (block->size() <= shift + 1) {
            shift = 0;
            ++block_num;
            if (block_prev == nullptr) {
                block_prev = host->upload(block_num);
            }
            block_num_prev = block_num - 1;
            std::swap(block, block_prev);
        } else {
            ++shift;
        }

        if (block_prev != nullptr && shift == block_size >> 1) {
            host->drop(block_num_prev);
            block_prev = nullptr;
        }

        return *this;
    }

    iterator &operator--() {
        if (shift == 0) {
            --block_num;
            if (block_prev == nullptr) {
                block_prev = host->upload(block_num);
            }
            block_num_prev = block_num + 1;
            std::swap(block, block_prev);
            shift = block->size() - 1;
        } else {
            --shift;
        }

        if (block_prev != nullptr && shift == block_size >> 1) {
            host->drop(block_num_prev);
            block_prev = nullptr;
        }

        return *this;
    }

    T operator*() {
        return block->at(shift);
    }

    bool operator==(const iterator &another) {
        return block_num == another.block_num && shift == another.shift;
    }

    bool operator!=(const iterator &another) {
        return !(*this == another);
    }

    ~iterator() {
        host->drop(block_num);
        if (block_prev != nullptr) {
            host->drop(block_num_prev);
        }
    }
};

template<class T>
const string external_deque<T>::delimiter = "data";

template<class T>
external_deque<T>::external_deque(const string &root):prefix(root + separator() + std::to_string(
        reinterpret_cast<intptr_t>(this)) + delimiter) {
    left_block = upload(0);
    right_block = upload(0);
}

template<class T>
void external_deque<T>::push_front(const T &object) {

    if (left_block->size() + 1 == block_size) {
        --left_edge;
        if (left_block_next == nullptr) {
            left_block_next = upload(left_edge);
        }
        std::swap(left_block, left_block_next);
    }
    left_block->push_front(object);
    ++data_size;

    if (left_block_next != nullptr && left_block->size() == block_size >> 1) {
        drop(left_edge + 1);
        left_block_next = nullptr;
    }
}

template<class T>
void external_deque<T>::push_back(const T &object) {
    if (right_block->size() + 1 == block_size) {
        ++right_edge;
        if (right_block_next == nullptr) {
            right_block_next = upload(right_edge);
        }
        std::swap(right_block, right_block_next);
    }
    right_block->push_back(object);
    ++data_size;

    if (right_block_next != nullptr && right_block->size() == block_size >> 1) {
        drop(right_edge - 1);
        right_block_next = nullptr;

    }
}

template<class T>
void external_deque<T>::pop_front() {
    if (left_block->size() == 0) {
        ++left_edge;
        if (left_block_next == nullptr) {
            left_block_next = upload(left_edge);
        }
        std::swap(left_block, left_block_next);
    }
    left_block->pop_front();
    --data_size;

    if (left_block_next != nullptr && left_block->size() == block_size >> 1) {
        drop(left_edge - 1);
        left_block_next = nullptr;
        std::remove((prefix + std::to_string(left_edge - 1)).c_str());
    }
}

template<class T>
void external_deque<T>::pop_back() {
    if (right_block->size() == 0) {
        --right_edge;
        if (right_block_next == nullptr) {
            right_block_next = upload(right_edge);
        }
        std::swap(right_block, right_block_next);
    }
    right_block->pop_back();
    --data_size;

    if (right_block_next != nullptr && right_block->size() == block_size >> 1) {
        drop(right_edge + 1);
        right_block_next = nullptr;
        std::remove((prefix + std::to_string(right_edge + 1)).c_str());
    }
}

template<class T>
mpz_class external_deque<T>::size() const {
    return data_size;
}

template<class T>
typename external_deque<T>::iterator external_deque<T>::begin() {
    auto tmp = (left_block->size() == 0) ? left_edge + 1 : left_edge;
    return external_deque<T>::iterator(tmp, 0, this);
}

template<class T>
typename external_deque<T>::iterator external_deque<T>::end() {
    return external_deque<T>::iterator(right_edge + 1, 0, this);
}

template<class T>
typename std::deque<T> *external_deque<T>::upload(unsigned number) {
    auto it = loaded_blocks.find(number);
    if (it == loaded_blocks.end()) {
        auto buffer = load_block<T>(prefix + std::to_string(number));
        std::deque<T> deq;
        deq.resize(buffer.size());
        std::copy(buffer.begin(), buffer.end(), deq.begin());
        loaded_blocks[number] = std::pair<int, std::deque<T>>(1, deq);
        //std::cout << "reading from memory\n";
        return &(loaded_blocks[number].second);
    } else {
        ++(it->second.first);
        return &(it->second.second);
    }
}

template<class T>
void external_deque<T>::drop(unsigned number) {
    auto it = loaded_blocks.find(number);
    if (it != loaded_blocks.end()) {
        if (it->second.first == 1) {
            std::vector<T> buffer(it->second.second.begin(), it->second.second.end());
            save_block(prefix + std::to_string(number), buffer);
            loaded_blocks.erase(it);
            //std::cout << "writing into memory\n";
        } else {
            --(it->second.first);
        }
    }
}

template<class T>
external_deque<T>::~external_deque() {
    for (unsigned i = left_edge - 1; i != right_edge + 2; ++i) {
        std::remove((prefix + std::to_string(i)).c_str());
    }
}

#endif //DEQUE_EXTERNAL_DEQUE_H
