#include "ProcessLibrary.h"
#include <chrono>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: process_test <path-to-child>\n";
        return 2;
    }
    int failures = 0;
    auto check = [&](bool ok, const std::string& name) {
        std::cout << (ok ? "[PASS] " : "[FAIL] ") << name << '\n';
        if (!ok) ++failures;
    };

    ProcessLibrary process;
    auto before = std::chrono::steady_clock::now();
    bool launched = process.start(argv[1], {"1500", "42"});
    auto launchTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - before).count();
    check(launched, "launch child");
    if (!launched) {
        std::cerr << process.lastError() << '\n';
        return 1;
    }
    check(launchTime < 1000, "start returns without waiting for child");
    check(!process.start(argv[1]), "cannot launch twice on same active object");
    int code = -1;
    check(process.wait(code), "wait for child");
    check(code == 42, "exit code is 42");
    check(!process.wait(code), "second wait fails");

    ProcessLibrary spaced;
    check(spaced.start(argv[1], {"250 ms \"quoted\"", "7"}),
          "start child with spaced argument");
    check(spaced.wait(code) && code == 7,
          "spaced argument preserved and exit code is 7");

    ProcessLibrary second;
    check(second.start(argv[1], {"0", "0"}), "start second child");
    check(second.wait(code) && code == 0, "exit code is 0");

    ProcessLibrary never;
    check(!never.wait(code), "wait without start fails");

    ProcessLibrary missing;
#ifdef _WIN32
    check(!missing.start("Z:\\nonexistent_folder_9483\\missing.exe"), "missing executable rejected");
#else
    check(!missing.start("/nonexistent_folder_9483/missing"), "missing executable rejected");
#endif
    std::cout << "\n" << (failures == 0 ? "ALL TESTS PASSED" : "TESTS FAILED") << '\n';
    return failures == 0 ? 0 : 1;
}
