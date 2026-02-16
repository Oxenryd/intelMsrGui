//
// Created by pierre on 2025-06-01.
//

#ifndef CLI_H
#define CLI_H
#include <iostream>
#include <cstdint>
#include <vector>

#include "ArgType.h"

struct PendingSave;
struct termios;
enum class IMT_ErrCode : uint8_t;
struct SetupPackage;

using ParamPair = std::pair<ArgType, std::string>;
using ParamPairs = std::vector<ParamPair>;

namespace CLI
{
    void printTop();

    inline std::string MV(const int row, const int col) {
        return "\033[" + std::to_string(row) + ";" + std::to_string(col) + "H";
    }

    inline void moveCursor(int row, int col) {
        std::cout << "\033[" << row << ";" << col << "H" << std::flush;
    }

    inline void clearScreen() {
        std::cout << "\033[2J" << MV(0,0);;// << std::flush;

        //moveCursor(0, 0);
    }

    inline void setCursorVisible(const bool visible) {
        if (visible)
            std::cout << "\033[?25h" << std::flush;
        else
            std::cout << "\033[?25l" << std::flush;
    }

    int print(bool loop);
    IMT_ErrCode validateInputString(const std::string& input, ParamPairs& tokens);
    IMT_ErrCode validateCliArgs(const std::vector<std::string>& args, ParamPairs& tokens);
    IMT_ErrCode execute(const ParamPairs& tokens, bool& shouldLoop, PendingSave& saveData);
}

#endif //CLI_H
