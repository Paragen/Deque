#include <iostream>
#include "deque.h"
#include <deque>
#include <random>
#include <string>
#include <fstream>
#include "dumb_external_deque.h"
#include "external_deque.h"
#include <time.h>
#include <map>
#include "util.h"
using std::rand;
using std::string;
using std::cout;
string root = "/home/ouroboros/tmp";
const size_t size_equals = 1000, size = 10000;
const int fill_by = 0;

void test_correctness() {

    cout << "------- Correctness --------\n";
    cout << "Data size: " << 2 * size_equals << " elements.\n";

    std::deque<int> native_deque;
    deque<int> simple_deque;
    dumb_external_deque<int> dumb_deque(root);
    external_deque<int> ext_deque(root);

    int tmp;
    for (size_t i  = 0; i < size_equals; ++i) {
        tmp = rand();
        native_deque.push_back(tmp);
        simple_deque.push_back(tmp);
        dumb_deque.push_back(tmp);
        ext_deque.push_back(tmp);

        tmp = rand();
        native_deque.push_front(tmp);
        simple_deque.push_front(tmp);
        dumb_deque.push_front(tmp);
        ext_deque.push_front(tmp);
    }



    auto it_simple = simple_deque.begin();
    auto it_dumb = dumb_deque.begin();
    auto it_ext = ext_deque.begin();
	
    for (auto it_native = native_deque.begin(); it_native != native_deque.end(); ++it_native, ++it_simple, ++it_dumb, ++it_ext) {
	
        assert(*it_native == *it_simple);
        assert(*it_native == *it_dumb);
        assert(*it_native == *it_ext);
    }

    while (native_deque.size() != 0) {
        assert(native_deque.front() == simple_deque.front());
        assert(native_deque.front() == *dumb_deque.begin());
        assert(native_deque.front() == *ext_deque.begin());

        native_deque.pop_front();
        simple_deque.pop_front();
        dumb_deque.pop_front();
        ext_deque.pop_front();
    }

    assert(native_deque.size() == simple_deque.size());
    assert(native_deque.size() == dumb_deque.size());
    assert(native_deque.size() == ext_deque.size());

    cout << "------ All correct -------\n";
}

template<class T>
void test_one(T& deq) {
    clock_t start = clock();
    clock_t prev = start;

    cout << "Filling by push_back\n";

    for (size_t i = 0; i < size; ++i) {
        deq.push_back(fill_by);
    }
    cout << "Done in " << ((float)(clock() - prev)) / CLOCKS_PER_SEC << " seconds.\n\n";
    prev = clock();
    cout << "Filling by push_front\n";

    for (size_t i = 0; i < size; ++i) {
        deq.push_front(fill_by);
    }

    cout << "Done in " << ((float)(clock() - prev)) / CLOCKS_PER_SEC << " seconds.\n\n";
    prev = clock();
    cout << "Iterating\n";
    int tmp;
	auto end = deq.end();
    for (auto it = deq.begin(); it != end; ++it) {
        tmp = *it;
    }


    cout << "Done in " << ((float)(clock() - prev)) / CLOCKS_PER_SEC << " seconds.\n\n";
    prev = clock();

    cout << "Erasing by pop_back\n";

    for (size_t i = 0; i < size; ++i) {
        deq.pop_back();
    }
    cout << "Done in " << ((float)(clock() - prev)) / CLOCKS_PER_SEC << " seconds.\n\n";
    prev = clock();
    cout << "Erasing by pop_front\n";

    for (size_t i = 0; i < size; ++i) {
        deq.pop_front();
    }
    cout << "Done in " << ((float)(clock() - prev)) / CLOCKS_PER_SEC << " seconds.\n\n";
    assert(deq.size() == 0);


    cout << "All done in " << ((float)(clock() - start)) / CLOCKS_PER_SEC << " seconds.\n";
}

void test_performance() {
    cout << "------- Performance --------\n";
    cout << "Data size: " << (float) (2 * size * sizeof(int) / (1024 * 1024)) << " mb\n";

    deque<int> simple_deque;
    dumb_external_deque<int> dumb_deque(root);
    external_deque<int> ext_deque(root);

    cout << "Testing usual deque\n";
    test_one(simple_deque);

    cout << "------------------------\n";

    cout << "Testing naive realisation of external deque deque\n";
    test_one(dumb_deque);

    cout << "------------------------\n";

    cout << "Testing external deque\n";
    test_one(ext_deque);

    cout << "--------- Done ----------\n";
}



int main() {
    test_correctness();
    test_performance();
}
