

#ifndef MERGESORT_MSORT_H
#define MERGESORT_MSORT_H


#include <algorithm>
#include "util.h"

namespace {

    template<class T, class Comp>
    void merge_inplace(T left, T left_end, T right, T right_end, T dest, Comp &comp) {

        while (left != left_end && right != right_end) {
            std::swap(*(dest++), comp(*left, *right) ? *(left++) : *(right++));
        }

        while (left != left_end) {
            std::swap(*(dest++), *(left++));
        }

        while (right != right_end) {
            std::swap(*(dest++), *(right++));
        }
    };


    template<class T, class Comp>
    void merge(T it_begin, T it_end, T ws, Comp &comp) {
        if (it_end - it_begin <= 1) {
            return;
        }
        auto middle = (it_end - it_begin) >> 1;

        merge(it_begin, it_begin + middle, ws, comp);
        merge(it_begin + middle, it_end, ws, comp);
        T m = it_begin + middle;

        merge_inplace(it_begin, m, m, it_end, ws, comp);

        while (it_begin != it_end) {
            std::swap(*(ws++), *(it_begin++));
        }
    }
    const string prefix = "externalsortblock#";
    template<class T, class Comp>
    std::vector<string> split_and_sort(const string& file_name, unsigned long block_size, Comp comp) {


        std::ifstream fin(file_name, std::ios::binary | std::ios::ate);
        std::streamsize size = fin.tellg();
        fin.seekg(0, std::ios::beg);
        std::vector<byte> vec;
        std::vector<string> names;
        unsigned long position = 0;
        block_size -= block_size % sizeof(T);
        vec.resize(block_size);
        while (position < size) {
            if (size - position < block_size) {
                block_size = size - position;
                vec.resize(block_size);
            }
            //todo : make default class for loading parts of file, add possibility to pass custom loader
            if (!fin.read( vec.data(), block_size)) {
                throw std::runtime_error("Can't read from file " + file_name );
            }
            size_t counter = 0;
            std::array<byte, sizeof(T)> buffer;
            std::vector<T> answer;
            T example;

            while (counter < vec.size()) {
                for (auto iter = buffer.begin(); iter != buffer.end(); ++iter) {
                    (*iter) = vec[counter++];
                }
                answer.push_back(from_bytes(buffer, example));
            }
            std::sort(answer.begin(), answer.end(), comp);
            names.push_back(prefix + std::to_string(names.size()));
            save_block(names.back(), answer);

            position += block_size;
        }

        return names;
    }

    void merge_blocks(const string& file_name, const std::vector<string>& names) {
        std::ofstream fout(file_name, std::ios::out);
        for (auto it = names.begin(); it != names.end(); ++it) {
            auto raw_data = load_raw_block(*it);
            std::remove((*it).c_str());
            fout.write(raw_data.data(), raw_data.size());
        }
        fout.close();
    }
    const string temporary_prefix = "externalsorttemporary#";
    template<class T>
    void save_temporary(const std::vector<T>& data, size_t suffix) {
        save_block(temporary_prefix + std::to_string(suffix), data);
    }


    void rename_temporary(size_t limit) {
        for (size_t i = 0; i < limit; ++i) {
            rename((temporary_prefix + std::to_string(i)).c_str(), (prefix + std::to_string(i)).c_str());
        }
    }
}

template<class T, class Comp>
void merge_sort(T it_begin, T it_end, Comp comp) {
    auto middle = ((it_end - it_begin) + 1) >> 1;
    T m = it_begin + middle;
    merge(m, it_end, it_begin, comp);
    T l = it_begin, r = m, tmp;
    while (r - l > 2) {
        m = l + ((r - l) >> 1);
        merge(l, m, m, comp);
        tmp = l + (((r - l) + 1) >> 1);
        merge_inplace(l, m, r, it_end, tmp, comp);
        r = tmp;
    }

    while (r > l) {
        for (T iter = r; iter < it_end && comp(*iter, *(iter - 1)); ++iter) {
            std::swap(*iter, *(iter - 1));
        }
        --r;
    }
}

template<class T>
void merge_sort(T it_begin, T it_end) {
    merge_sort(it_begin, it_end,
               [](const decltype(*it_begin) f, const decltype(*it_begin) s) -> bool { return f < s; });
}



template<class T, class Comp>
void
external_sort(const std::string &file_name, unsigned long  memory_size, unsigned long block_size, Comp comp) {
    if (block_size < 2 * 1024 * 1024) {
        block_size = 2 * 1024 * 1024;
    }
    std::vector<std::string> names = split_and_sort<int>(file_name, block_size, comp);

    auto iter = names.begin();
    size_t count = 1;
    const size_t blocks_count = (memory_size < 20 * 1024 * 1024 ? 20 * 1024 * 1024 : memory_size) / block_size - 1;
    assert(blocks_count > 1);
    block_size /= sizeof(T);
    std::vector<T> buffer;
    std::vector<std::vector<T>> loaded_data;
    std::vector<decltype(names.begin())> current_iters, end_iters;
    std::vector<decltype(buffer.begin())> current_positions;

    while (count != names.size()) {
        size_t tmp_block_counter = 0;
        iter = names.begin();
        while (iter < names.end()) {
            size_t tmp_counter = 0;
            while (tmp_counter < blocks_count && iter < names.end()) {
                loaded_data.push_back(load_block<T>(*iter));
                current_iters.push_back(iter);
                current_positions.push_back(loaded_data.back().begin());
                iter = names.end() - iter < count ? names.end() : iter + count;
                end_iters.push_back(iter);
                ++tmp_counter;
            }


            decltype(buffer.begin()) *max;
            for (;;) {
                max = nullptr;
                for (size_t i = 0; i < loaded_data.size(); ++i) {
                    if (current_iters[i] == end_iters[i]) {
                        continue;
                    }

                    if (current_positions[i] == loaded_data[i].end()) {
                        ++current_iters[i];
                        if (current_iters[i] == end_iters[i]) {
                            continue;
                        }

                        loaded_data[i] = load_block<T>(*current_iters[i]);
                        current_positions[i] = loaded_data[i].begin();
                    }

                    if (max == nullptr || comp(*current_positions[i], **max)) {
                        max = &current_positions[i];
                    }
                }

                if (max == nullptr) {
                    if (buffer.size() != 0) {
                        save_temporary(buffer, tmp_block_counter++);
                        buffer.clear();
                    }
                    break;
                }

                buffer.push_back(**max);
                ++(*max);
                if (buffer.size() == block_size) {
                    save_temporary(buffer, tmp_block_counter++);
                    buffer.clear();
                }
            }
            current_iters.clear();
            current_positions.clear();
            end_iters.clear();
            loaded_data.clear();
        }

        rename_temporary(tmp_block_counter);

        count *= blocks_count;
        if (count > names.size()) {
            count = names.size();
        }
    }

    merge_blocks(file_name, names);
}


template<class T>
void external_sort(const std::string &file_name, unsigned long  memory_size, unsigned long block_size) {
    external_sort<T>(file_name, memory_size, block_size,
                     [](const T& f, const T& s) -> bool { return f < s; });
}
#endif //MERGESORT_MSORT_H
