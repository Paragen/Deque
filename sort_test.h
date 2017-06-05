#include <vector>
#include <iostream>
#include "msort.h"
#include <random>
#include <fstream>

namespace sort_test {
    const unsigned long long count = 1000000;
    unsigned long long size;
    string root;
    using std::cout;

    void test_correctness() {
        int tmp;
        string file_name = root + "/correctness";
        std::ofstream fout(file_name);
        std::vector<int> vec1, vec2;

        for (int i = 0; i < count; ++i) {
            tmp = rand();
            vec1.push_back(tmp);
            vec2.push_back(tmp);
            auto buf = to_bytes(tmp);
            fout.write(buf.data(), buf.size());
        }

        fout.close();

        std::sort(vec1.begin(), vec1.end());
        merge_sort(vec2.begin(), vec2.end());
        external_sort<int>(file_name, 1L, 1L);

        auto vec3 = load_block<int>(file_name);


        for (int i = 0; i < count; ++i) {
            assert(vec1[i] == vec2[i]);
            assert(vec3[i] == vec1[i]);
        }

        remove(file_name.c_str());
    }

    void test_external(unsigned long long block_size, unsigned long long cnt, const string &file_name) {
        int tmp;
        std::ofstream fout(file_name);
        for (unsigned long long i = 0; i < size; ++i) {
            tmp = rand();
            auto buf = to_bytes(tmp);
            fout.write(buf.data(), buf.size());
        }
        fout.close();
        ++cnt;
        auto start = clock();
        external_sort<int>(file_name, block_size * cnt, block_size);
        cout << "Done in " << (double) (clock() - start) / CLOCKS_PER_SEC << " seconds." << std::endl;
        remove(file_name.c_str());
    }

    void test_performance() {
        cout << "Data size: " << (float) (size * sizeof(int) / (1024 * 1024)) << " mb\n";
        cout << "------- common sort --------\n";
        {
            std::vector<int> vec;
            for (unsigned long long i = 0; i < size; ++i) {
                vec.push_back(rand());
            }
            auto start = clock();
            merge_sort(vec.begin(), vec.end());
            cout << "Done in " << (double) (clock() - start) / CLOCKS_PER_SEC << " seconds." << std::endl;
        }
        cout << "------------------------\n";

        const unsigned long long block_size = 4 * 1024 * 1024;
        cout << "Sort parameters: Block size = 4mb\n";

        cout << "------- External sort by two blocks --------\n";
        const string file_name = root + "/performance";
        test_external(block_size, 2, file_name);
        cout << "--------------------------\n";

        cout << "-------- External sort by ten blocks --------\n";
        test_external(block_size, 10, file_name);
    }

    void test_diff_blocks_size() {
        const unsigned long long memory_size = 1 * 1024 * 1024 * 1024ULL;
        unsigned long long cnt = 2, block_size = 10 * 1024 * 1024;
        cout << "Sort parameters: Block size = 10mb\n";
        const string file_name = root + "/different";
        while (cnt * block_size < memory_size) {

            cout << "Blocks count: " << cnt << std::endl;
            test_external(block_size, cnt , file_name);
            cout << std::endl;
            cnt *= 2;
        }
    }

    void sort_test(const string &dir_name, unsigned long long sz) {
        root = dir_name;
        size = sz;
        cout << "------- Correctness --------\n";
        test_correctness();
        cout << "------- All correct --------\n";

        cout << "------- Performance --------\n";
        test_performance();
        cout << "-------   Done   --------\n";

        cout << "------- Different blocks count --------\n";
        //test_diff_blocks_size();
        cout << "------- Done --------\n";

    }
}