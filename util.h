
#include <array>
#include <string>
#include <fstream>
#include <vector>
#include <assert.h>

#ifndef DEQUE_UTIL_H
#define DEQUE_UTIL_H
using std::string;
using std::array;


inline string separator() {
#ifdef _WIN32
    return "\\";
#else
    return "/";
#endif
}


using byte = char;

template<class T>
array<byte, sizeof(T)> to_bytes(const T &object) {
    array<byte, sizeof(T)> bytes;

    const auto b = reinterpret_cast<const byte *>(std::addressof(object));
    std::copy(b, b + sizeof(T), bytes.data());

    return bytes;
}

template<class T>
T &from_bytes(const array<byte, sizeof(T)> &data, T &object) {
    static_assert(std::is_trivially_copyable<T>::value, "unacceptable type");

    std::copy(data.begin(), data.end(), reinterpret_cast<byte *>(std::addressof(object)));

    return object;
}

template<class T>
T get_from_file(const string &file_name, unsigned long shift) {
    std::ifstream fin(file_name, std::ios::binary);
    fin.seekg(shift * sizeof(T), std::ios::beg);

    std::array<byte, sizeof(T)> data;
    fin.read(data.data(), sizeof(T));

    //default constructor
    T tmp;

    return from_bytes(data, tmp);
}

template<class T>
void add_to_file_end(const string &file_name, const T &object) {
    auto data = to_bytes(object);
    std::ofstream fout(file_name, std::ios::out | std::ios::app);
    fout.write(data.data(), data.size());
    fout.close();
}

std::vector<byte> load_raw_block(const string &file_name) {
    std::ifstream fin(file_name, std::ios::binary | std::ios::ate);
    std::streamsize size = fin.tellg();
    fin.seekg(0, std::ios::beg);
    std::vector<byte> vec;
    if (size > 0) {
        vec.resize(size);
        assert(fin.read((char *) vec.data(), size));
    }
    fin.close();

    return vec;
}

template <class T>
std::vector<T> load_block(const string &file_name) {
    auto&& vec = load_raw_block(file_name);
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

    return answer;
}

template <class T>
void save_block(const string& file_name, const std::vector<T>& data) {
    std::vector<byte> raw_data;

    std::for_each(data.begin(), data.end(),
                  [&raw_data](const T& it){
                      auto&& buffer = to_bytes(it);
                      for (auto iter = buffer.begin(); iter != buffer.end(); ++iter) {
                          raw_data.push_back(*iter);
                      }
                  });

    std::ofstream fout(file_name, std::ios::out);
    fout.write(raw_data.data(), raw_data.size());

}

template<class T>
void add_to_file_begin(const string &file_name, const T &object) {

    auto&& vec(load_raw_block(file_name));
    auto data = to_bytes(object);
    std::ofstream fout(file_name);

    fout.write(data.data(), data.size());
    std::copy(vec.begin(), vec.end(), std::ostreambuf_iterator<char>(fout));

    fout.close();
}

template<class T>
void remove_from_file(const string &file_name, bool from_end) {
    auto&& vec (load_raw_block(file_name));

    std::ofstream fout(file_name);
    if (from_end) {
        std::copy(vec.begin(), vec.end() - sizeof(T), std::ostreambuf_iterator<char>(fout));
    } else {
        std::copy(vec.begin() + sizeof(T), vec.end(), std::ostreambuf_iterator<char>(fout));
    }
    fout.close();
}




#endif //DEQUE_UTIL_H
