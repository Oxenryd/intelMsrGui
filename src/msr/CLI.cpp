//
// Created by pierre on 2025-06-07.
//

#include "CLI.h"

#include <charconv>
#include <cpuid.h>
#include <filesystem>
#include <format>
#include <vector>
#include <thread>
#include <termios.h>

#include "Msr.hpp"
#include "Description.h"
#include "OcMailbox.h"
#include "Parameters.hpp"
#include "IntelTypes.hpp"
#include "IMT_ErrCode.h"
#include "MsrOps.h"

IMT_ErrCode CLI::validateInputString(const std::string &input, ParamPairs& tokens) {

    // Parse string
    std::vector<std::string> args;
    std::istringstream iss(input);
    std::string word;
    while (iss >> word)
        args.push_back(word);
    size_t i = 0;
    while (i < args.size()) {
        bool found = false;
        uint8_t foundNumArgs = 0;
        auto type = ArgType::Unknown;
        for (const auto& mapping : ARGS_DEF) {
            auto cmpStr = std::string(mapping.name);
            if (cmpStr == args[i]) {
                found = true;
                type = mapping.type;
                foundNumArgs = mapping.num_args;

                // Quick quit
                if (mapping.type == ArgType::Quit)
                    return IMT_ErrCode::OK_ShouldExit;

                if (i + foundNumArgs >= args.size()) {
                    found = false;
                    type = ArgType::Unknown;
                }
                break;
            }
        }
        if (found) {
            auto parameter = foundNumArgs == 0 ? "" : args[i + 1];
            tokens.emplace_back(type, parameter);
            i += foundNumArgs;
        } else
            return IMT_ErrCode::ArgumentError;
    }

    return IMT_ErrCode::OK;
}

IMT_ErrCode CLI::validateCliArgs(const std::vector<std::string> &args, ParamPairs &tokens) {

    size_t i = 1;
    while (i < args.size()) {
        bool found = false;
        uint8_t foundNumArgs = 0;
        auto type = ArgType::Unknown;
        for (const auto& mapping : ARGS_DEF) {
            auto cmpStr = std::string(mapping.name);
            if (cmpStr == args[i]) {
                found = true;
                type = mapping.type;
                foundNumArgs = mapping.num_args;

                if (i + foundNumArgs >= args.size()) {
                    found = false;
                    type = ArgType::Unknown;
                }
                break;
            }
        }
        if (found) {
            auto parameter = foundNumArgs == 0 ? "" : args[i + 1];
            tokens.emplace_back(type, parameter);
            i += 1 + foundNumArgs;
        } else
            return IMT_ErrCode::ArgumentError;
    }



    return IMT_ErrCode::OK;
}

IMT_ErrCode CLI::execute(const ParamPairs& tokens, bool& shouldLoop, PendingSave& saveData) {

    constexpr auto tryParseInt( [] (const std::string& str, uint64_t& outValue) -> bool {
        if (std::from_chars(str.data(), str.data() + str.size(), outValue).ec == std::errc{})
            return true;
        return false;
    });

    constexpr auto tryParseDouble( [] (const std::string& str, double& outValue) -> bool {
        if (std::from_chars(str.data(), str.data() + str.size(), outValue).ec == std::errc{})
            return true;
        return false;
    });

    constexpr auto tryParseBool( [] (const std::string& str, bool& outValue) -> bool {
        if (str == "true") {
            outValue = true;
            return true;
        }
        if (str == "false") {
            outValue = false;
            return true;
        }
        uint8_t val;
        if (std::from_chars(str.data(), str.data() + str.size(), val).ec == std::errc{}) {
            if (val > 0)
                outValue = true;
            else
                outValue = false;
            return true;
        }
        return false;
    });

    SetupPackage pkg = MsrOps::readCurrentAsPackage();

    // Check if HWP is set, try to set if not
    if (!pkg.HWP_IsSet) {
        IA32_PM_ENABLE pm{};
        pm.HWP_Enabled = 1;
        if (!MSR::write_msr(0, IA32_PM_ENABLE_ADDR, &pm.value)) {
            return IMT_ErrCode::HWP_NotAvailable;
        }
    }

    bool hasPendingApplyCmd = false;
    for (const auto& cmd : tokens) {

        switch (static_cast<ArgType>(cmd.first)) {

            default: return IMT_ErrCode::UnknownArgument;
            case ArgType::Quit: return IMT_ErrCode::OK_ShouldExit;
            case ArgType::Unknown: return IMT_ErrCode::UnknownArgument;

            // Interface
            case ArgType::Continuous: {
                shouldLoop = true;
            } break;
            case ArgType::LoadPreset: {
                hasPendingApplyCmd = true;
                uint64_t value = 0;
                if (tryParseInt(cmd.second, value)) {
                    if (value > 255)
                        return IMT_ErrCode::ArgumentError;
                    auto path = std::string{PRESET_FILE_DEFAULT};

                    // Look for set target preset file
                    for (const auto& tokCmd : tokens) {
                        if (tokCmd.first == ArgType::PresetFile) {
                            path = tokCmd.second;
                            break;
                        }
                    }

                    auto EC = IMT_ErrCode::OK;
                    SetupPackage* loadPkg;
                    IMT_CHECK_AND_RETURN(EC, MsrOps::loadSetting(path, static_cast<uint8_t>(value), &loadPkg));
                    IMT_CHECK_AND_RETURN(EC, MsrOps::apply(*loadPkg));
                    delete loadPkg;
                    return IMT_ErrCode::OK;
                }
                return IMT_ErrCode::ParsingError;
            }
            case ArgType::SavePreset: {
                auto path = std::string{PRESET_FILE_DEFAULT};
                for (const auto& tokCmd : tokens) {
                    if (tokCmd.first == ArgType::PresetFile) {
                        path = tokCmd.second;
                        break;
                    }
                }
                uint64_t value = 0;
                if (!tryParseInt(cmd.second, value))
                    return IMT_ErrCode::ArgumentError;

                saveData.name = std::string{"New Preset no " + cmd.second};
                saveData.presetIndex = value;
                saveData.path = path;
                saveData.pending = true;
            } break;

            // Voltage Offsets
            case ArgType::V_Offset_Core: {
                hasPendingApplyCmd = true;
                double value = 0.0;
                if (tryParseDouble(cmd.second, value))
                    pkg.V_Offset_VCore.set(value);
                else return IMT_ErrCode::ParsingError;
            } break;
            case ArgType::V_Offset_IGpu: {
                hasPendingApplyCmd = true;
                double value = 0.0;
                if (tryParseDouble(cmd.second, value))
                    pkg.V_Offset_IGpu.set(value);
                else return IMT_ErrCode::ParsingError;
            } break;
            case ArgType::V_Offset_Cache: {
                hasPendingApplyCmd = true;
                double value = 0.0;
                if (tryParseDouble(cmd.second, value))
                    pkg.V_Offset_Cache.set(value);
                else return IMT_ErrCode::ParsingError;
            } break;
            case ArgType::V_Offset_SA: {
                hasPendingApplyCmd = true;
                double value = 0.0;
                if (tryParseDouble(cmd.second, value))
                    pkg.V_Offset_SystemAgent.set(value);
                else return IMT_ErrCode::ParsingError;
            } break;
            case ArgType::V_Offset_Analog: {
                hasPendingApplyCmd = true;
                double value = 0.0;
                if (tryParseDouble(cmd.second, value))
                    pkg.V_Offset_AnalogIO.set(value);
                else return IMT_ErrCode::ParsingError;
            } break;
            case ArgType::V_Offset_Digital: {
                hasPendingApplyCmd = true;
                double value = 0.0;
                if (tryParseDouble(cmd.second, value))
                    pkg.V_Offset_DigitalIO.set(value);
                else return IMT_ErrCode::ParsingError;
            } break;

            // Clock ratios, P CORE
            case ArgType::Ratio_P_Group1: {
                hasPendingApplyCmd = true;
                uint64_t value = 0;
                if (tryParseInt(cmd.second, value))
                    pkg.PCore_RatioGroup_1.set(value);
                else return IMT_ErrCode::ParsingError;
            } break;
            case ArgType::Ratio_P_Group2: {
                hasPendingApplyCmd = true;
                uint64_t value = 0;
                if (tryParseInt(cmd.second, value))
                    pkg.PCore_RatioGroup_2.set(value);
                else return IMT_ErrCode::ParsingError;
            } break;
            case ArgType::Ratio_P_Group3: {
                hasPendingApplyCmd = true;
                uint64_t value = 0;
                if (tryParseInt(cmd.second, value))
                    pkg.PCore_RatioGroup_3.set(value);
                else return IMT_ErrCode::ParsingError;
            } break;
            case ArgType::Ratio_P_Group4: {
                hasPendingApplyCmd = true;
                uint64_t value = 0;
                if (tryParseInt(cmd.second, value))
                    pkg.PCore_RatioGroup_4.set(value);
                else return IMT_ErrCode::ParsingError;
            } break;
            case ArgType::Ratio_P_Group5: {
                hasPendingApplyCmd = true;
                uint64_t value = 0;
                if (tryParseInt(cmd.second, value))
                    pkg.PCore_RatioGroup_5.set(value);
                else return IMT_ErrCode::ParsingError;
            } break;
            case ArgType::Ratio_P_Group6: {
                hasPendingApplyCmd = true;
                uint64_t value = 0;
                if (tryParseInt(cmd.second, value))
                    pkg.PCore_RatioGroup_6.set(value);
                else return IMT_ErrCode::ParsingError;
            } break;
            case ArgType::Ratio_P_Group7: {
                hasPendingApplyCmd = true;
                uint64_t value = 0;
                if (tryParseInt(cmd.second, value))
                    pkg.PCore_RatioGroup_7.set(value);
                else return IMT_ErrCode::ParsingError;
            } break;
            case ArgType::Ratio_P_Group8: {
                hasPendingApplyCmd = true;
                uint64_t value = 0;
                if (tryParseInt(cmd.second, value))
                    pkg.PCore_RatioGroup_8.set(value);
                else return IMT_ErrCode::ParsingError;
            } break;
            case ArgType::Ratio_P_All: {
                hasPendingApplyCmd = true;
                uint64_t value = 0;
                if (tryParseInt(cmd.second, value)) {
                    pkg.PCore_RatioGroup_1.set(value);
                    pkg.PCore_RatioGroup_2.set(value);
                    pkg.PCore_RatioGroup_3.set(value);
                    pkg.PCore_RatioGroup_4.set(value);
                    pkg.PCore_RatioGroup_5.set(value);
                    pkg.PCore_RatioGroup_6.set(value);
                    pkg.PCore_RatioGroup_7.set(value);
                    pkg.PCore_RatioGroup_8.set(value);
                }
                else return IMT_ErrCode::ParsingError;
            } break;

           // Clock ratios, E CORE
            case ArgType::Ratio_E_Group1: {
                hasPendingApplyCmd = true;
                uint64_t value = 0;
                if (tryParseInt(cmd.second, value))
                    pkg.ECore_RatioGroup_1.set(value);
                else return IMT_ErrCode::ParsingError;
            } break;
            case ArgType::Ratio_E_Group2: {
                hasPendingApplyCmd = true;
                uint64_t value = 0;
                if (tryParseInt(cmd.second, value))
                    pkg.ECore_RatioGroup_2.set(value);
                else return IMT_ErrCode::ParsingError;
            } break;
            case ArgType::Ratio_E_Group3: {
                hasPendingApplyCmd = true;
                uint64_t value = 0;
                if (tryParseInt(cmd.second, value))
                    pkg.ECore_RatioGroup_3.set(value);
                else return IMT_ErrCode::ParsingError;
            } break;
            case ArgType::Ratio_E_Group4: {
                hasPendingApplyCmd = true;
                uint64_t value = 0;
                if (tryParseInt(cmd.second, value))
                    pkg.ECore_RatioGroup_4.set(value);
                else return IMT_ErrCode::ParsingError;
            } break;
            case ArgType::Ratio_E_Group5: {
                hasPendingApplyCmd = true;
                uint64_t value = 0;
                if (tryParseInt(cmd.second, value))
                    pkg.ECore_RatioGroup_5.set(value);
                else return IMT_ErrCode::ParsingError;
            } break;
            case ArgType::Ratio_E_Group6: {
                hasPendingApplyCmd = true;
                uint64_t value = 0;
                if (tryParseInt(cmd.second, value))
                    pkg.ECore_RatioGroup_6.set(value);
                else return IMT_ErrCode::ParsingError;
            } break;
            case ArgType::Ratio_E_Group7: {
                hasPendingApplyCmd = true;
                uint64_t value = 0;
                if (tryParseInt(cmd.second, value))
                    pkg.ECore_RatioGroup_7.set(value);
                else return IMT_ErrCode::ParsingError;
            } break;
            case ArgType::Ratio_E_Group8: {
                hasPendingApplyCmd = true;
                uint64_t value = 0;
                if (tryParseInt(cmd.second, value))
                    pkg.ECore_RatioGroup_8.set(value);
                else return IMT_ErrCode::ParsingError;
            } break;
            case ArgType::Ratio_E_All: {
                hasPendingApplyCmd = true;
                uint64_t value = 0;
                if (tryParseInt(cmd.second, value)) {
                    pkg.ECore_RatioGroup_1.set(value);
                    pkg.ECore_RatioGroup_2.set(value);
                    pkg.ECore_RatioGroup_3.set(value);
                    pkg.ECore_RatioGroup_4.set(value);
                    pkg.ECore_RatioGroup_5.set(value);
                    pkg.ECore_RatioGroup_6.set(value);
                    pkg.ECore_RatioGroup_7.set(value);
                    pkg.ECore_RatioGroup_8.set(value);
                }
                else return IMT_ErrCode::ParsingError;
            } break;

            // Misc
            case ArgType::C1E: {
                hasPendingApplyCmd = true;
                bool value = true;
                if (tryParseBool(cmd.second, value))
                    pkg.EnhancedHalt_C1E.set(value);
                else return IMT_ErrCode::ParsingError;
            } break;
            case ArgType::EE_Optimization: {
                hasPendingApplyCmd = true;
                bool value = false;
                if (tryParseBool(cmd.second, value))
                    pkg.Disable_EE_Optimization.set(value);
                else return IMT_ErrCode::ParsingError;
            } break;
            case ArgType::ApplyClocksFirst: {
                hasPendingApplyCmd = true;
                bool value = true;
                if (tryParseBool(cmd.second, value))
                    pkg.ApplyClocksFirst = value;
                else return IMT_ErrCode::ParsingError;
            } break;

                // HWP
            case ArgType::HWP_USE_PACKAGE_CTRL: {
                hasPendingApplyCmd = true;
                bool value = true;
                if (tryParseBool(cmd.second, value))
                    pkg.HWP_PackageControl.set(value);
                else return IMT_ErrCode::ParsingError;
            } break;
            case ArgType::HWP_PKG_Desired: {
                hasPendingApplyCmd = true;
                uint64_t value = 0;
                if (tryParseInt(cmd.second, value))
                    pkg.HWP_Desired.set(value);
                else return IMT_ErrCode::ParsingError;
            } break;
            case ArgType::HWP_PKG_Minimum: {
                hasPendingApplyCmd = true;
                uint64_t value = 0;
                if (tryParseInt(cmd.second, value))
                    pkg.HWP_Minimum.set(value);
                else return IMT_ErrCode::ParsingError;
            } break;
            case ArgType::HWP_PKG_Maximum: {
                hasPendingApplyCmd = true;
                uint64_t value = 0;
                if (tryParseInt(cmd.second, value))
                    pkg.HWP_Maximum.set(value);
                else return IMT_ErrCode::ParsingError;
            } break;
            case ArgType::HWP_PKG_EPP: {
                hasPendingApplyCmd = true;
                uint64_t value = 0;
                if (tryParseInt(cmd.second, value))
                    pkg.HWP_EPP.set(value);
                else return IMT_ErrCode::ParsingError;
            } break;
        }
    }

    IMT_ErrCode EC = IMT_ErrCode::OK;
    if (hasPendingApplyCmd) {
        IMT_CHECK_AND_RETURN(EC, MsrOps::apply(pkg));
    }

    return IMT_ErrCode::OK;
}

void CLI::printTop() {
        std::cout << NAME << " version " << VERSION << H_LINE << "\n*** " << MSR::getCpuName() << " ***\n";
}

int CLI::print(const bool loop) {
    // Set decimal places to fixed in stream
    std::cout << std::fixed;

    // Find physical cpus
    unsigned int logicalCores = std::thread::hardware_concurrency();
    if (logicalCores == 0)
        return static_cast<int>(IMT_ErrCode::NoCpusFound);
    std::vector<int> physicalCores;
    for (int i = 0; i < logicalCores; i++) {
        std::filesystem::path dir = std::format("/dev/cpu/{}", i);
        if (std::filesystem::exists(dir) && std::filesystem::is_directory(dir)) {
            physicalCores.push_back(i);
        }
    }

    // Find which are P-Cores / E-Cores
    std::vector<int> pCores;
    std::vector<int> eCores;
    for (auto i : physicalCores) {
        cpu_set_t set{};
        CPU_ZERO(&set); CPU_SET(i, &set);
        if (sched_setaffinity(0, sizeof(set), &set) != 0) { perror("affinity"); std::exit(1); }

        unsigned eax, ebx, ecx, edx;
        if (!__get_cpuid_count(0x1A, 0, &eax, &ebx, &ecx, &edx)) {
            pCores.push_back(i);
            continue;
        }

        auto core_type = static_cast<uint8_t>(eax >> 24);
        if (core_type == 0x20)
            eCores.push_back(i);
        else
            pCores.push_back(i);
    }

    // Pre-alloc buffers for HWP states
    std::vector<IA32_HWP_REQUEST> pCoreHWPs;
    std::vector<IA32_HWP_REQUEST> eCoreHWPs;
    std::vector<IA32_HWP_REQUEST_PKG> pCoreHWPs_PKG;
    std::vector<IA32_HWP_REQUEST_PKG> eCoreHWPs_PKG;
    for (auto i : pCores) {
        IA32_HWP_REQUEST hwp{};
        pCoreHWPs.push_back(hwp);
        IA32_HWP_REQUEST_PKG hpwPkg{};
        pCoreHWPs_PKG.push_back(hpwPkg);
    }
    for (auto i : eCores) {
        IA32_HWP_REQUEST hwp{};
        eCoreHWPs.push_back(hwp);
        IA32_HWP_REQUEST_PKG hpwPkg{};
        eCoreHWPs_PKG.push_back(hpwPkg);
    }

    // Read the units in use
    auto pUnits{MSR::readAndReturn<MSR_RAPL_POWER_UNIT>(
        0, MSR_RAPL_POWER_UNIT_ADDR)};

    // Loop
    // if (loop)
    //     enableRawMode();
    // setup line input and error message
    // std::string inputLine;
    // std::string lastError = "";
    // int errorLineStartNumber = 0;
    // int msCounter = 0;
    bool firstRun{true};
    bool shouldExit{!loop};
    do {

        //if (msCounter <= 0) {
            int line = 5;
            double deltaTime = 0.5;

            // Clear and prepare for output
            setCursorVisible(false);
            clearScreen();
            printTop();
            std::cout << std::flush;

            // POWER ---------------------------------------------------------------------------------------------------
            auto powerStats{MSR::readAndReturn<MSR_PKG_ENERGY_STATUS>(pCores[0], MSR_PKG_ENERGY_STATUS_ADDR)};
            if (firstRun)
                s_lastTotalConsumed = powerStats.Total_Energy_Consumed;
            auto perfStatsP0{MSR::readAndReturn<IA32_PERF_STATUS>(pCores[0], IA32_PERF_STATUS_ADDR)};
            auto perfStatsE0{MSR::readAndReturn<IA32_PERF_STATUS>(eCores[0], IA32_PERF_STATUS_ADDR)};
            auto tempTarget{MSR::readAndReturn<MSR_TEMPERATURE_TARGET>(pCores[0], MSR_TEMPERATURE_TARGET_ADDR)};
            auto pkgTherm{MSR::readAndReturn<IA32_PACKAGE_THERM_STATUS>(pCores[0], IA32_PACKAGE_THERM_STATUS_ADDR)};


            std::cout << MV(line, 0) << "Power --------------------------------------------------------------";
            std::cout << std::setprecision(3) << MV(++line, 8)
                << "CPU VID" << MV(line, 56) << readVID(perfStatsP0) << "V";
            std::cout << std::setprecision(2) << MV(++line, 8)
                << "Power Consumption" << MV(line, 56) << getPkgPowerW(pUnits, powerStats, deltaTime) << "W";
            std::cout << std::setprecision(0) << MV(++line, 8)
                << "Package Temperature" << MV(line, 56) << getCurPkgTemp(tempTarget, pkgTherm) << "°";
            std::cout << MV(++line, 8) << "Voltage Offsets";
            std::cout << std::setprecision(2) << MV(++line, 16)
                << " - Core" << MV(line, 56) << OcMailbox::readCoreVidOffset() << "V";
            std::cout << std::setprecision(2) << MV(++line, 16)
                << " - IGPU" << MV(line, 56) << OcMailbox::readIGpuVidOffset() << "V";
            std::cout << std::setprecision(2) << MV(++line, 16)
                << " - CPU Cache" << MV(line, 56) << OcMailbox::readCacheVidOffset() << "V";
            std::cout << std::setprecision(2) << MV(++line, 16)
                << " - System Agent" << MV(line, 56) << OcMailbox::readSAVidOffset() << "V";
            std::cout << std::setprecision(2) << MV(++line, 16)
                << " - Analog I/O" << MV(line, 56) << OcMailbox::readAnalogVidOffset() << "V";
            std::cout << std::setprecision(2) << MV(++line, 16)
                << " - Digital I/O" << MV(line, 56) << OcMailbox::readDigitalVidOffset() << "V";
            std::cout << std::setprecision(2) << MV(++line, 16)
                << " - E-Core Cache" << MV(line, 56) << OcMailbox::readECacheVidOffset() << "V";


            // Performance ----------------------------------------------------------------------------------------------
            auto pCoreRatios{MSR::readAndReturn<MSR_TURBO_RATIO_LIMIT>(
                pCores[0], MSR_TURBO_RATIO_LIMIT_ADDR )};
            auto eCoreRatios{MSR::readAndReturn<MSR_SECONDARY_TURBO_RATIO_LIMIT>(
                eCores[0], MSR_SECONDARY_TURBO_RATIO_LIMIT_ADDR )};
            auto powerCtl{MSR::readAndReturn<MSR_POWER_CTL>(pCores[0], MSR_POWER_CTL_ADDR)};
            auto hwpEnabled = MSR::readAndReturn<IA32_PM_ENABLE>(pCores[0], IA32_PM_ENABLE_ADDR).HWP_Enabled == 1;
            line++;
            std::cout << MV(++line, 0) << "Performance --------------------------------------------------------";
            std::cout << MV(++line, 8)
                << "P-Core 0 Current Multiplier" << MV(line, 56) << perfStatsP0.CurrentFID << "x";
            std::cout << MV(++line, 8)
                << "E-Core 0 Current Multiplier" << MV(line, 56) << perfStatsE0.CurrentFID << "x";

            std::cout << MV(++line, 8) << "P Core Ratios";
            std::cout << std::setprecision(0) << MV(++line, 16)
                << " - 1 Core Active" << MV(line, 56) << pCoreRatios.Maximum_Ratio_Limit_1_Core << "x";
            std::cout << std::setprecision(0) << MV(++line, 16)
                << " - 2 Core Active" << MV(line, 56) << pCoreRatios.Maximum_Ratio_Limit_2_Core << "x";
            std::cout << std::setprecision(0) << MV(++line, 16)
                << " - 3 Core Active" << MV(line, 56) << pCoreRatios.Maximum_Ratio_Limit_3_Core << "x";
            std::cout << std::setprecision(0) << MV(++line, 16)
                << " - 4 Core Active" << MV(line, 56) << pCoreRatios.Maximum_Ratio_Limit_4_Core << "x";
            std::cout << std::setprecision(0) << MV(++line, 16)
                << " - 5 Core Active" << MV(line, 56) << pCoreRatios.Maximum_Ratio_Limit_5_Core << "x";
            std::cout << std::setprecision(0) << MV(++line, 16)
                << " - 6 Core Active" << MV(line, 56) << pCoreRatios.Maximum_Ratio_Limit_6_Core << "x";
            std::cout << std::setprecision(0) << MV(++line, 16)
                << " - 7 Core Active" << MV(line, 56) << pCoreRatios.Maximum_Ratio_Limit_7_Core << "x";
            std::cout << std::setprecision(0) << MV(++line, 16)
                << " - 8 Core Active" << MV(line++, 56) << pCoreRatios.Maximum_Ratio_Limit_8_Core << "x";

            std::cout << MV(++line, 8) << "E Core Ratios";
            std::cout << std::setprecision(0) << MV(++line, 16)
                << " - 1 Core Active" << MV(line, 56) << eCoreRatios.Maximum_Ratio_Limit_1_Core << "x";
            std::cout << std::setprecision(0) << MV(++line, 16)
                << " - 2 Core Active" << MV(line, 56) << eCoreRatios.Maximum_Ratio_Limit_2_Core << "x";
            std::cout << std::setprecision(0) << MV(++line, 16)
                << " - 3 Core Active" << MV(line, 56) << eCoreRatios.Maximum_Ratio_Limit_3_Core << "x";
            std::cout << std::setprecision(0) << MV(++line, 16)
                << " - 4 Core Active" << MV(line, 56) << eCoreRatios.Maximum_Ratio_Limit_4_Core << "x";
            std::cout << std::setprecision(0) << MV(++line, 16)
                << " - 5 Core Active" << MV(line, 56) << eCoreRatios.Maximum_Ratio_Limit_5_Core << "x";
            std::cout << std::setprecision(0) << MV(++line, 16)
                << " - 6 Core Active" << MV(line, 56) << eCoreRatios.Maximum_Ratio_Limit_6_Core << "x";
            std::cout << std::setprecision(0) << MV(++line, 16)
                << " - 7 Core Active" << MV(line, 56) << eCoreRatios.Maximum_Ratio_Limit_7_Core << "x";
            std::cout << std::setprecision(0) << MV(++line, 16)
                << " - 8 Core Active" << MV(line++, 56) << eCoreRatios.Maximum_Ratio_Limit_8_Core << "x";

            std::cout << MV(++line, 8)
                << "C1E Enhanced Halt" << MV(line, 56) << (powerCtl.C1E_Enable == 1 ? "Yes" : "No");
            std::cout << MV(++line, 8)
                << "Energy Efficiency Optimization" << MV(line, 56) <<
                    (powerCtl.Disable_Energy_Eff_Optimization == 1 ? "No" : "Yes");
            std::cout << MV(++line, 8)
                << "HWP Enabled" << MV(line, 56) << (hwpEnabled ? "" : "No");
            if (hwpEnabled) {
                MSR::readAndAssignMany(pCores, IA32_HWP_REQUEST_ADDR, pCoreHWPs);
                bool completePerCore = !(anyUsesPkgControl(pCoreHWPs) || anyUsesPkgControl(eCoreHWPs));
                std::cout << std::setprecision(0) << MV(++line, 16)
                    << "Complete Per Core Control" << MV(line, 56) << (completePerCore ? "Yes" : "No");
                if (completePerCore) {
                    std::cout << MV(++line, 16) << "P-Core 0 Request";
                    std::cout << MV(++line, 24)
                        << "- Desired Performance" << MV(line, 56) << pCoreHWPs[0].Desired_Performance << "  ";
                    std::cout << MV(++line, 24)
                        << "- Minimum Performance" << MV(line, 56) << pCoreHWPs[0].Minimum_Performance << "  ";;
                    std::cout << MV(++line, 24)
                        << "- Maximum Performance" << MV(line, 56) << pCoreHWPs[0].Maximum_Performance << "  ";;
                    std::cout << MV(++line, 24)
                        << "- EPP" << MV(line, 56) << pCoreHWPs[0].Energy_Performance_Preference << "  ";
                } else {
                    MSR::readAndAssignMany(pCores, IA32_HWP_REQUEST_PKG_ADDR, pCoreHWPs_PKG);
                    std::cout << MV(++line, 16) << "Package Request";
                    std::cout << MV(++line, 24)
                        << "- Desired Performance" << MV(line, 56) << pCoreHWPs_PKG[0].Desired_Performance << "  ";
                    std::cout << MV(++line, 24)
                        << "- Minimum Performance" << MV(line, 56) << pCoreHWPs_PKG[0].Minimum_Performance << "  ";;
                    std::cout << MV(++line, 24)
                        << "- Maximum Performance" << MV(line, 56) << pCoreHWPs_PKG[0].Maximum_Performance << "  ";;
                    std::cout << MV(++line, 24)
                        << "- EPP" << MV(line, 56) << pCoreHWPs_PKG[0].Energy_Performance_Preference << "  ";
                }
            }

            // Wait for next refresh ------------------------------------------------------------------------------------
            firstRun = false;
            setCursorVisible(true);
            if (!loop)
                return 0;

        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<uint32_t>(deltaTime * 1000)));
    } while (!shouldExit);
    return 0;
}
