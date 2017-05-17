#pragma once

#include <glob.h>
#include <initializer_list>
#include <new>
#include <memory>
#include <assert.h>


template<class T>
class deque {

    static void customDeleter(T*);
    const static size_t  standard_capacity = 10;
    size_t init_capacity;
    size_t data_size = 0, capacity;
    size_t head, tail;
    std::unique_ptr<T,  void(&)(T*)> data;

    void set_and_copy(size_t new_size);
    void ensure_capacity();

public:
    class iterator;

    deque();
    deque(size_t init_capacity);
    deque(const std::initializer_list<T>& list);


    deque<T>::iterator begin();
    deque<T>::iterator end();

    T& front();
    T& back();

    void insert(const iterator&) = delete;
    void erase(const iterator&) = delete;

    void push_front(T&& el);
    void push_back(T&& el);

    void push_front(const T& el);
    void push_back(const T& el);

    void pop_front();
    void pop_back();

    size_t size() const;
    bool empty() const;

};

template <class T>
class deque<T>::iterator {
    friend class deque<T>;
    T* ptr;
    size_t size, shift;
    iterator(T* ptr, size_t size, size_t shift):ptr(ptr), size(size), shift(shift){}

public:

    T& operator++() {
        shift = (shift + 1 >= size) ? 0 : shift + 1;
    }

    T& operator--() {
        shift = (shift != 0 ) ? shift - 1 : size - 1;
    }

    bool operator==(const iterator& another) {
        return shift == another.shift;
    }

    bool operator!=(const iterator& another) {
        return !(*this == another);
    }

    T& operator*() {
        return *(ptr + shift);
    }
};

template <class T>
void deque<T>::customDeleter(T* ptr) {
    delete[] ptr;
}
template <class T>
deque<T>::deque():deque(standard_capacity) {}

template <class T>
deque<T>::deque(size_t init_capacity): data(std::unique_ptr<T, void(&)(T*)>(nullptr, customDeleter)), head(0), tail(0), capacity(0),init_capacity(init_capacity),data_size(0) {
    set_and_copy(init_capacity);
    capacity = init_capacity;
}

template <class T>
deque<T>::deque(const std::initializer_list<T>& list):deque(list.size() * 2) {
    auto iter = list.begin();
    for (size_t i = 0; i < list.size(); ++i, ++iter) {
        new (data.get() + i) T(std::move(*iter));
    }
    tail = data_size = list.size();
}


template <class T>
void deque<T>::set_and_copy(size_t new_capacity) {
    std::unique_ptr<T, void(&)(T*)> buf {static_cast<T*>(operator new[] (sizeof(T) * new_capacity)) , customDeleter};
    size_t counter = 0;
    try {
        for (auto iter = begin(); iter != end(); ++iter) {
            new (buf.get() + counter) T(std::move(*iter));
            ++counter;
        }
    } catch (...) {
        size_t re_counter = 0;
        for (auto iter = begin();re_counter != counter; ++iter, ++re_counter) {
            *iter = std::move(buf.get()[re_counter]);
        }
        throw;
    }
    capacity = new_capacity;
    data.reset(buf.release());
    head = 0;
    tail = data_size;
}

template <class T>
void deque<T>::ensure_capacity() {
    if (data_size >= capacity - 1) {
        set_and_copy(capacity * 2);
    } else if (capacity >> 2 > data_size  && capacity > 2 * standard_capacity) {
        set_and_copy(capacity / 2);
    }
}

template <class T>
T& deque<T>::front() {
    return data.get()[head];
}

template <class T>
T& deque<T>::back() {
    auto tmp = (tail  > 0) ? tail - 1 : capacity - 1;
    return data.get()[tmp];
}



template <class T>
void deque<T>::push_front(T &&el) {
    ensure_capacity();
    auto pos = (head > 0) ? head - 1 : capacity - 1;
    new (data.get() + pos) T(std::forward<T>(el));
    ++data_size;
    head = pos;
}

template <class T>
void deque<T>::push_front(const T& el) {
    ensure_capacity();
    auto pos = (head > 0) ? head - 1 : capacity - 1;
    new (data.get() + pos) T(el);
    ++data_size;
    head = pos;
}

template <class T>
void deque<T>::push_back(T &&el) {
    ensure_capacity();
    new (data.get() + tail) T(std::forward<T>(el));
    ++data_size;
    tail = (tail + 1 >= capacity) ? 0 : tail + 1;
}

template <class T>
void deque<T>::push_back(const T& el) {
    ensure_capacity();
    new (data.get() + tail) T(el);
    ++data_size;
    tail = (tail + 1 >= capacity) ? 0 : tail + 1;
}

template <class T>
void deque<T>::pop_front() {
    ensure_capacity();
    data.get()[head].~T();
    --data_size;
    head = (head + 1 >= capacity) ? 0 : head + 1;

}

template <class T>
void deque<T>::pop_back() {
    ensure_capacity();
    auto pos = (tail > 0) ? tail - 1 : capacity - 1;
    data.get()[pos].~T();
    --data_size;
    tail = pos;
}

template <class T>
size_t deque<T>::size() const {
    return data_size;
}

template <class T>
bool deque<T>::empty() const {
    return data_size == 0;
}

template <class T>
typename deque<T>::iterator deque<T>::begin() {
    return deque<T>::iterator(data.get(), capacity, head);
}

template <class T>
typename deque<T>::iterator deque<T>::end() {
    return deque<T>::iterator(data.get(), capacity, tail);
}





