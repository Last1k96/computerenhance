#pragma once

#include <string>

#ifdef _WIN32

#include <windows.h>
#include <vector>

#endif

static std::string getShortPathName(const std::string &longPath) {
#ifdef _WIN32
    // Get the required buffer size
    DWORD length = GetShortPathNameA(longPath.c_str(), NULL, 0);
    if (length == 0) return longPath; // Error, return original

    // Allocate buffer
    std::vector<char> buffer(length);

    // Get the short path
    DWORD result = GetShortPathNameA(longPath.c_str(), buffer.data(), length);
    if (result == 0) return longPath; // Error, return original

    return buffer.data();
#else
    return longPath; // Non-Windows, return original
#endif
}

static bool compareAsmLines(std::string_view expected,
                            std::string_view actual) {
    using namespace std::string_view_literals;
    constexpr auto green = "\x1B[32m"sv;   // “good” colour
    constexpr auto red = "\x1B[31m"sv;   // “bad” colour
    constexpr auto reset = "\x1B[0m"sv;    // restore defaults

    auto expLines = std::views::split(expected, '\n');
    auto actLines = std::views::split(actual, '\n');

    auto expIt = expLines.begin();
    auto actIt = actLines.begin();

    std::size_t lineNo = 1;
    bool allOk = true;

    for (; expIt != expLines.end() || actIt != actLines.end(); ++lineNo) {
        std::string_view e = expIt != expLines.end()
                             ? std::string_view{*expIt++}
                             : ""sv;
        std::string_view a = actIt != actLines.end()
                             ? std::string_view{*actIt++}
                             : ""sv;

        if (e != a) {
            allOk = false;
            std::cout << lineNo << ": "
                      << green << e << reset << '\n'
                      << lineNo << ": "
                      << red << a << reset << "\n\n";
        }
    }

    return allOk;
}