#pragma once
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <sys/types.h>
#endif

class ProcessLibrary {
public:
    ProcessLibrary() = default;
    ~ProcessLibrary();
    ProcessLibrary(const ProcessLibrary&) = delete;
    ProcessLibrary& operator=(const ProcessLibrary&) = delete;

    bool start(const std::string& executable, const std::vector<std::string>& arguments = {});
    bool wait(int& exitCode);
    const std::string& lastError() const { return error_; }

private:
#ifdef _WIN32
    HANDLE process_ = nullptr;
#else
    pid_t pid_ = -1;
#endif
    std::string error_;
};
