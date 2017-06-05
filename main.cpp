#include "sort_test.h"
#include "deque_test.h"

int main(int argc, char *argv[]) {
    if (argc < 4) {
        std::cout << "Usage: [deque|sort] [path_to_root] [size]\n";
        return 1;
    }
    string root(argv[2]);
    string mod(argv[1]);
    unsigned long long size = std::stoull(string(argv[3]));
    if (mod == "deque") {
        deque_test::deque_test(root, size);
    } else if (mod == "sort") {
        sort_test::sort_test(root, size);
    } else {
        return 1;
    }
}