//
// Created by pierre on 2025-06-01.
//

#ifndef MSR_H
#define MSR_H
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <cstdint>
#include <fstream>
#include <vector>
#include <iostream>
#include <bitset>

#include "HasUint64ValueConcept.hpp"

namespace MSR
{
    // Read MSR
    static bool read_msr(const int cpu, const uint32_t msr, uint64_t* value) {
        char path[64];
        snprintf(path, sizeof(path), "/dev/cpu/%d/msr", cpu);

        const int fd = open(path, O_RDONLY);
        if (fd < 0) {
            fprintf(stderr, "Failed to open %s: %s\n", path, strerror(errno));
            return false;
        }

        if (lseek(fd, msr, SEEK_SET) == -1) {
            fprintf(stderr, "Failed to seek to MSR 0x%X: %s\n", msr, strerror(errno));
            close(fd);
            return false;
        }

        ssize_t rd = read(fd, value, sizeof(uint64_t));
        if (rd != sizeof(uint64_t)) {
            fprintf(stderr, "Failed to read MSR 0x%X: %s\n", msr, strerror(errno));
            close(fd);
            return false;
        }

        close(fd);
        return true;
    }

    template <HasUint64Value T>
    static T readAndReturn(const int cpu, const uint32_t msr) {
        T newMsr = T{};
        auto result = read_msr(cpu, msr, &newMsr.value);
        if (!result)
            throw std::runtime_error("MSR read failed");
        return newMsr;
    }

    template <HasUint64Value T>
    static bool readAndAssignMany(const std::vector<int>& cpus, const uint32_t msr, std::vector<T>& regBuffer) {
        size_t j = 0;
        for (auto& i : cpus) {
            auto result = read_msr(i, msr, &regBuffer[j++].value);
            if (!result)
                return false;
        }
        return true;
    }


    static std::string asBitsetString(const uint64_t value) {
        return std::bitset<64>(value).to_string();
    }

    // Write MSR
    static bool write_msr(const int cpu, const uint32_t msr, const uint64_t* value) {
        char path[64];
        snprintf(path, sizeof(path), "/dev/cpu/%d/msr", cpu);

        const int fd = open(path, O_WRONLY);
        if (fd < 0) {
            fprintf(stderr, "Failed to open %s: %s\n", path, strerror(errno));
            return false;
        }

        if (lseek(fd, msr, SEEK_SET) == -1) {
            fprintf(stderr, "Failed to seek to MSR 0x%X: %s\n", msr, strerror(errno));
            close(fd);
            return false;
        }
        auto size = sizeof(uint64_t);
        ssize_t wr = write(fd, value, size );
        if (wr != sizeof(uint64_t)) {
            fprintf(stderr, "Failed to write MSR 0x%X: %s\n", msr, strerror(errno));
            close(fd);
            return false;
        }

        close(fd);
        return true;
    }

    static std::string getCpuName() {
        std::ifstream cpuInfo("/proc/cpuinfo");
        std::string line;

        while (std::getline(cpuInfo, line)) {
            if (line.rfind("model name", 0) == 0) {
                size_t colon = line.find(':');
                if (colon != std::string::npos)
                    return line.substr(colon + 2);
            }
        }

        return "Unknown CPU";
    }

}


#endif //MSR_H
