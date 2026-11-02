#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>
int main(int argc, char* argv[]) {
    if (argc != 3) return 99;
    int milliseconds = std::atoi(argv[1]);
    int code = std::atoi(argv[2]);
    if (milliseconds < 0 || milliseconds > 10000 || code < 0 || code > 255) return 98;
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    std::cout << "Child completed, exit code: " << code << '\n';
    return code;
}
