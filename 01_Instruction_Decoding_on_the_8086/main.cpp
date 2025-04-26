#include <iostream>
#include <random>
// https://www.computerenhance.com/p/instruction-decoding-on-the-8086

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());

    int min = 1, max = 100;
    std::uniform_int_distribution<> dist(min, max);

    std::cout << dist(gen) << std::endl;
    return 0;
}
