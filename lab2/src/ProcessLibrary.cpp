#include "ProcessLibrary.h"
#include <cerrno>
#include <cstring>
#include <string>
#include <vector>

#ifdef _WIN32
static std::wstring widen(const std::string& s) {
    if (s.empty()) return std::wstring();
    int size = MultiByteToWideChar(CP_UTF8, 0, s.c_str(),
                                   static_cast<int>(s.size()), nullptr, 0);
    if (size <= 0) return std::wstring();
    std::wstring result(static_cast<std::wstring::size_type>(size), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()),
                        result.data(), size);
    return result;
}

static std::wstring quoteWindows(const std::wstring& s) {
    std::wstring result = L"\"";
    unsigned backslashes = 0;
    for (wchar_t c : s) {
        if (c == L'\\') { ++backslashes; continue; }
        if (c == L'"') {
            result.append(backslashes * 2 + 1, L'\\');
            result += L'"';
        } else {
            result.append(backslashes, L'\\');
            result += c;
        }
        backslashes = 0;
    }
    result.append(backslashes * 2, L'\\');
    return result + L'"';
}
#else
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#endif

ProcessLibrary::~ProcessLibrary() {
#ifdef _WIN32
    if (process_) CloseHandle(process_);
#else
    if (pid_ != -1) {
        (void)kill(pid_, SIGTERM);
        int status;
        while (waitpid(pid_, &status, 0) < 0 && errno == EINTR) {}
        pid_ = -1;
    }
#endif
}

bool ProcessLibrary::start(const std::string& executable,
                           const std::vector<std::string>& arguments) {
    error_.clear();
#ifdef _WIN32
    if (process_) { error_ = "process already started"; return false; }
#else
    if (pid_ != -1) { error_ = "process already started"; return false; }
#endif
    if (executable.empty()) { error_ = "empty executable path"; return false; }
#ifdef _WIN32
    std::wstring wideExecutable = widen(executable);
    std::wstring command = quoteWindows(wideExecutable);
    for (const auto& arg : arguments) command += L" " + quoteWindows(widen(arg));
    std::vector<wchar_t> mutableCommand(command.begin(), command.end());
    mutableCommand.push_back(L'\0');
    STARTUPINFOW si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};
    if (!CreateProcessW(wideExecutable.c_str(), mutableCommand.data(), nullptr,
                        nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
        error_ = "CreateProcess failed: " + std::to_string(GetLastError());
        return false;
    }
    CloseHandle(pi.hThread);
    process_ = pi.hProcess;
#else
    int pipefd[2];
    if (pipe(pipefd) != 0) { error_ = std::strerror(errno); return false; }
    if (fcntl(pipefd[1], F_SETFD, FD_CLOEXEC) == -1) {
        error_ = std::strerror(errno);
        close(pipefd[0]); close(pipefd[1]); return false;
    }
    pid_t child = fork();
    if (child == -1) {
        error_ = std::strerror(errno);
        close(pipefd[0]); close(pipefd[1]); return false;
    }
    if (child == 0) {
        close(pipefd[0]);
        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(executable.c_str()));
        for (const auto& arg : arguments) argv.push_back(const_cast<char*>(arg.c_str()));
        argv.push_back(nullptr);
        execvp(executable.c_str(), argv.data());
        int saved = errno;
        (void)write(pipefd[1], &saved, sizeof(saved));
        _exit(127);
    }
    close(pipefd[1]);
    int childError = 0;
    ssize_t bytes;
    do { bytes = read(pipefd[0], &childError, sizeof(childError)); }
    while (bytes < 0 && errno == EINTR);
    int readError = errno;
    close(pipefd[0]);
    if (bytes != 0) {
        int status;
        while (waitpid(child, &status, 0) < 0 && errno == EINTR) {}
        error_ = bytes > 0 ? std::strerror(childError) : std::strerror(readError);
        return false;
    }
    pid_ = child;
#endif
    return true;
}

bool ProcessLibrary::wait(int& exitCode) {
    error_.clear();
#ifdef _WIN32
    if (!process_) { error_ = "no active process"; return false; }
    if (WaitForSingleObject(process_, INFINITE) != WAIT_OBJECT_0) {
        error_ = "WaitForSingleObject failed"; return false;
    }
    DWORD code;
    if (!GetExitCodeProcess(process_, &code)) {
        error_ = "GetExitCodeProcess failed"; return false;
    }
    exitCode = static_cast<int>(code);
    CloseHandle(process_);
    process_ = nullptr;
#else
    if (pid_ == -1) { error_ = "no active process"; return false; }
    int status;
    pid_t result;
    do { result = waitpid(pid_, &status, 0); }
    while (result == -1 && errno == EINTR);
    if (result == -1) { error_ = std::strerror(errno); return false; }
    pid_ = -1;
    if (WIFEXITED(status)) {
        exitCode = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        exitCode = 128 + WTERMSIG(status);
    } else {
        exitCode = 128;
    }
#endif
    return true;
}
