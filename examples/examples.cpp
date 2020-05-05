#include "utils.h"
#include <chrono>
#include <iostream>

int main()
{
    auto start = std::chrono::system_clock::now();
    download_synchronous();
    auto step1_end = std::chrono::system_clock::now();
    download_asynchronous();
    auto step2_end = std::chrono::system_clock::now();
    download_multiplexing();
    auto end = std::chrono::system_clock::now();
    auto ratio = std::chrono::duration_cast<std::chrono::system_clock::duration>(std::chrono::seconds(1)).count();
    auto get_sec = [&ratio](auto start, auto end) {return static_cast<double>((end - start).count()) / ratio; };
    std::cout << "Synchronous: " << get_sec(start, step1_end) << " s." << std::endl
        << "Asynchronous: " << get_sec(step1_end, step2_end) << " s." << std::endl
        << "Multiplexing: " << get_sec(step2_end, end) << " s." << std::endl;
}