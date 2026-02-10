//
// Created by pierre on 2025-06-06.
//
#include <format>
#include <thread>
#include <cpuid.h>

#include "Parameters.hpp"
#include "MsrOps.h"

#include <filesystem>

#include "IntelTypes.hpp"
#include "Msr.hpp"
#include "OcMailbox.h"
#include "IMT_ErrCode.h"



std::pair<std::vector<int>, std::vector<int>> MsrOps::getCoresCount() {
    // Find physical cpus
    unsigned int logicalCores = std::thread::hardware_concurrency();
    if (logicalCores == 0)
        throw std::logic_error{"Zero logical cores."};
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

    return {pCores, eCores};
}

SetupPackage MsrOps::readCurrentAsPackage() {
    SetupPackage pkg{};

    const auto pm = MSR::readAndReturn<IA32_PM_ENABLE>(0, IA32_PM_ENABLE_ADDR);
    pkg.HWP_IsSet = static_cast<bool>(pm.HWP_Enabled);

    auto cores = getCoresCount();

    pkg.NumPCores = static_cast<uint16_t>(cores.first.size());
    pkg.NumECores = static_cast<uint16_t>(cores.second.size());

    // Prealloc buffers for HWP states
    std::vector<IA32_HWP_REQUEST> pCoreHWPs;
    std::vector<IA32_HWP_REQUEST> eCoreHWPs;
    std::vector<IA32_HWP_REQUEST_PKG> pCoreHWPs_PKG;
    pCoreHWPs.resize(cores.first.size());
    eCoreHWPs.resize(cores.second.size());
    pCoreHWPs_PKG.resize(cores.first.size());
    // for (auto i : cores.first) {
    //     IA32_HWP_REQUEST hwp{};
    //     pCoreHWPs.push_back(hwp);
    //     IA32_HWP_REQUEST_PKG hpwPkg{};
    //     pCoreHWPs_PKG.push_back(hpwPkg);
    // }
    // for (auto i :cores.second) {
    //     IA32_HWP_REQUEST hwp{};
    //     eCoreHWPs.push_back(hwp);
    // }

    // Ratios and misc
    const auto ringRatios{MSR::readAndReturn<MSR_RING_RATIO>(cores.first[0], MSR_RING_RATIO_ADDR)};
    const auto pCoreRatios{MSR::readAndReturn<MSR_TURBO_RATIO_LIMIT>(
        cores.first[0], MSR_TURBO_RATIO_LIMIT_ADDR )};
    const auto eCoreRatios{MSR::readAndReturn<MSR_SECONDARY_TURBO_RATIO_LIMIT>(
        cores.second[0], MSR_SECONDARY_TURBO_RATIO_LIMIT_ADDR )};
    const auto powerCtl{MSR::readAndReturn<MSR_POWER_CTL>(cores.first[0], MSR_POWER_CTL_ADDR)};

    pkg.RingMin.set(ringRatios.RingRatioMin);
    pkg.RingMax.set(ringRatios.RingRatioMax);

    pkg.PCore_RatioGroup_1.value.set(pCoreRatios.Maximum_Ratio_Limit_1_Core);
    pkg.PCore_RatioGroup_2.value.set(pCoreRatios.Maximum_Ratio_Limit_2_Core);
    pkg.PCore_RatioGroup_3.value.set(pCoreRatios.Maximum_Ratio_Limit_3_Core);
    pkg.PCore_RatioGroup_4.value.set(pCoreRatios.Maximum_Ratio_Limit_4_Core);
    pkg.PCore_RatioGroup_5.value.set(pCoreRatios.Maximum_Ratio_Limit_5_Core);
    pkg.PCore_RatioGroup_6.value.set(pCoreRatios.Maximum_Ratio_Limit_6_Core);
    pkg.PCore_RatioGroup_7.value.set(pCoreRatios.Maximum_Ratio_Limit_7_Core);
    pkg.PCore_RatioGroup_8.value.set(pCoreRatios.Maximum_Ratio_Limit_8_Core);

    pkg.ECore_RatioGroup_1.value.set(eCoreRatios.Maximum_Ratio_Limit_1_Core);
    pkg.ECore_RatioGroup_2.value.set(eCoreRatios.Maximum_Ratio_Limit_2_Core);
    pkg.ECore_RatioGroup_3.value.set(eCoreRatios.Maximum_Ratio_Limit_3_Core);
    pkg.ECore_RatioGroup_4.value.set(eCoreRatios.Maximum_Ratio_Limit_4_Core);
    pkg.ECore_RatioGroup_5.value.set(eCoreRatios.Maximum_Ratio_Limit_5_Core);
    pkg.ECore_RatioGroup_6.value.set(eCoreRatios.Maximum_Ratio_Limit_6_Core);
    pkg.ECore_RatioGroup_7.value.set(eCoreRatios.Maximum_Ratio_Limit_7_Core);
    pkg.ECore_RatioGroup_8.value.set(eCoreRatios.Maximum_Ratio_Limit_8_Core);

    pkg.EnhancedHalt_C1E.value.set(powerCtl.C1E_Enable);
    pkg.Disable_EE_Optimization.value.set(powerCtl.Disable_Energy_Eff_Optimization);

    pkg.V_Offset_VCore.value.set(OcMailbox::readCoreVidOffset());
    pkg.V_Offset_IGpu.value.set(OcMailbox::readIGpuVidOffset());
    pkg.V_Offset_Cache.value.set(OcMailbox::readCacheVidOffset());
    pkg.V_Offset_SystemAgent.value.set(OcMailbox::readSAVidOffset());
    pkg.V_Offset_AnalogIO.value.set(OcMailbox::readAnalogVidOffset());
    pkg.V_Offset_DigitalIO.value.set(OcMailbox::readDigitalVidOffset());
    pkg.V_Offset_ECache.value.set(OcMailbox::readECacheVidOffset());

    pkg.VF_CoreOffsetPoint1.value.set(OcMailbox::readVFOffsetRaw(1));
    pkg.VF_CoreOffsetPoint2.value.set(OcMailbox::readVFOffsetRaw(2));
    pkg.VF_CoreOffsetPoint3.value.set(OcMailbox::readVFOffsetRaw(3));
    pkg.VF_CoreOffsetPoint4.value.set(OcMailbox::readVFOffsetRaw(4));
    pkg.VF_CoreOffsetPoint5.value.set(OcMailbox::readVFOffsetRaw(5));
    pkg.VF_CoreOffsetPoint6.value.set(OcMailbox::readVFOffsetRaw(6));
    pkg.VF_CoreOffsetPoint7.value.set(OcMailbox::readVFOffsetRaw(7));
    pkg.VF_CoreOffsetPoint8.value.set(OcMailbox::readVFOffsetRaw(8));
    pkg.VF_CoreOffsetPoint9.value.set(OcMailbox::readVFOffsetRaw(9));
    pkg.VF_CoreOffsetPoint10.value.set(OcMailbox::readVFOffsetRaw(10));
    pkg.VF_CoreOffsetPoint11.value.set(OcMailbox::readVFOffsetRaw(11));

    for (size_t i = 0; i < 11; ++i) {
        const auto result =
            std::bit_cast<OcMailbox::OC_MAILBOX_MSR, uint64_t>(OcMailbox::readVFOffsetRaw(i + 1));
        const double offset = OcMailbox::convertOffsetFromRaw(result.VF.Offset);
        pkg.setVfCoreOffsetPoint(i + 1, offset);
        pkg.VF_CoreFreqs[i] = static_cast<uint8_t>(result.VF.PointFrequency);
    }



    MSR::readAndAssignMany(cores.first, IA32_HWP_REQUEST_ADDR, pCoreHWPs);
    MSR::readAndAssignMany(cores.second, IA32_HWP_REQUEST_ADDR, eCoreHWPs);
    bool completePerCore = !(anyUsesPkgControl(pCoreHWPs) || anyUsesPkgControl(eCoreHWPs));
    if (!completePerCore)
        pkg.HWP_PackageControl.value.set(true);

    MSR::readAndAssignMany(cores.first, IA32_HWP_REQUEST_PKG_ADDR, pCoreHWPs_PKG);

    pkg.HWP_Desired.value.set(pCoreHWPs_PKG[0].Desired_Performance);
    pkg.HWP_EPP.value.set(pCoreHWPs_PKG[0].Energy_Performance_Preference);
    pkg.HWP_Minimum.value.set(pCoreHWPs_PKG[0].Minimum_Performance);
    pkg.HWP_Maximum.value.set(pCoreHWPs_PKG[0].Maximum_Performance);

    return pkg;
}

StatusPackage MsrOps::readStatusAsPackage(const size_t eCoreOffset) {
    StatusPackage sPkg{};
    sPkg.powerUnits = MSR::readAndReturn<MSR_RAPL_POWER_UNIT>(0, MSR_RAPL_POWER_UNIT_ADDR);
    sPkg.energyStatus = MSR::readAndReturn<MSR_PKG_ENERGY_STATUS>(0, MSR_PKG_ENERGY_STATUS_ADDR);
    sPkg.ecorePerfStats = MSR::readAndReturn<IA32_PERF_STATUS>(0, IA32_PERF_STATUS_ADDR);
    sPkg.ecorePerfStats = MSR::readAndReturn<IA32_PERF_STATUS>(eCoreOffset, IA32_PERF_STATUS_ADDR);
    sPkg.tempTarget = MSR::readAndReturn<MSR_TEMPERATURE_TARGET>(0, MSR_TEMPERATURE_TARGET_ADDR);
    sPkg.packageTherm = MSR::readAndReturn<IA32_PACKAGE_THERM_STATUS>(0, IA32_PACKAGE_THERM_STATUS_ADDR);

    return sPkg;
}

IMT_ErrCode MsrOps::readPresets(
    const std::string &presetFilePath,
    std::unordered_map<std::string, SetupPackage>* outPresets)
{
    std::filesystem::path presetPath = presetFilePath;
    std::ifstream inFile{presetPath};
    if (!inFile) {
        std::cerr << "ERROR: Could not locate preset file." << std::endl;
        return IMT_ErrCode::NotFound;
    }

    JSon j;
    try {
        inFile >> j;
    } catch (const std::exception& e) {
        std::cerr << "ERROR IN READING JSON: " << e.what() << std::endl;
        // todo warning?
        return IMT_ErrCode::NoPresetsFound;
    }

    outPresets->clear();
    for (auto& [key, value] : j.items()) {
        outPresets->insert(std::make_pair(key, SetupPackage{value}));
    }

    return IMT_ErrCode::OK;
}

IMT_ErrCode MsrOps::deletePreset(const std::string &presetFilePath, const std::string &presetName) {

    std::filesystem::path presetPath = presetFilePath;
    JSon j;
    {
        std::ifstream inFile{presetPath};
        if (inFile.is_open() && inFile.peek() != std::ifstream::traits_type::eof()) {
            inFile >> j;
        }
    }
    if (j.contains(presetName)) {
        j.erase(presetName);
        // Write back
        {
            std::ofstream outFile{presetPath};
            if (!outFile)
                return IMT_ErrCode::WriteError;

            outFile << j.dump(2) << '\n';
        }
        return IMT_ErrCode::OK;
    }

    return IMT_ErrCode::NoPresetsFound;
}

IMT_ErrCode MsrOps::saveCurrentSetting(
    const std::string& presetFilePath,
    const std::string& presetName,
    const SetupPackage& pkg)
{

    std::filesystem::path presetPath = presetFilePath;
    JSon j;
    {
        std::ifstream inFile{presetPath};
        if (inFile.is_open() && inFile.peek() != std::ifstream::traits_type::eof()) {
            inFile >> j;
        }
    }
    if (j.contains(presetName))
        return IMT_ErrCode::PresetNameExists;
    j[presetName] = pkg.to_json();

    // Write back
    {
        std::ofstream outFile{presetPath};
        if (!outFile)
            return IMT_ErrCode::WriteError;

        outFile << j.dump(2) << '\n';
    }
    return IMT_ErrCode::OK;
}

IMT_ErrCode MsrOps::loadSetting(
    const std::string& presetFilePath,
    uint8_t presetIndex,
    SetupPackage** outPackage)
{
    std::filesystem::path presetPath = presetFilePath;
    std::ifstream inFile{presetPath};
    if (!inFile) {
        delete *outPackage;
        *outPackage = nullptr;
        std::cerr << "ERROR: Could not locate preset file." << std::endl;
        return IMT_ErrCode::ReadError;
    }

    JSon j;
    try {
        inFile >> j;
    } catch (const std::exception& e) {
        delete *outPackage;
        std::cerr << "ERROR IN READING JSON: " << e.what() << std::endl;
        throw;
    }

    const std::string key = std::to_string(presetIndex);
    if (!j.contains(key)) {
        delete *outPackage;
        *outPackage = nullptr;
        std::cerr << "ERROR: JSon does not contain key " << key << "." << std::endl;
        return IMT_ErrCode::NotFound;
    }
    if (!j[key].contains("package")) {
        delete *outPackage;
        *outPackage = nullptr;
        std::cerr << "ERROR: JSon does not contain \"package\"." << std::endl;
        return IMT_ErrCode::NotFound;
    }

    const JSon& pkgNode = j[key]["package"];
    if (!pkgNode.is_array()) {
        delete *outPackage;
        *outPackage = nullptr;
        std::cerr << "ERROR: \"package\" is not an array." << std::endl;
        return IMT_ErrCode::ReadError;
    }

    auto* newPackage = new SetupPackage{pkgNode};
    auto [pCores, eCores] = getCoresCount();
    newPackage->NumPCores = pCores.size();
    newPackage->NumECores = eCores.size();
    *outPackage = newPackage;

    return IMT_ErrCode::OK;
}

IMT_ErrCode MsrOps::apply(const SetupPackage& package) {

    auto EC = IMT_ErrCode::OK;
    if (package.ApplyClocksFirst) {
        IMT_CHECK_AND_RETURN(EC, applyClocks(package));
        IMT_CHECK_AND_RETURN(EC, applyVOffset(package));
        IMT_CHECK_AND_RETURN(EC, applyCoreVF(package));
    } else {
        IMT_CHECK_AND_RETURN(EC, applyVOffset(package));
        IMT_CHECK_AND_RETURN(EC, applyCoreVF(package));
        IMT_CHECK_AND_RETURN(EC, applyClocks(package));
    }

    IMT_CHECK_AND_RETURN(EC, applyHWP(package));
    IMT_CHECK_AND_RETURN(EC, applyMisc(package));

    return IMT_ErrCode::OK;
}



IMT_ErrCode MsrOps::applyClocks(const SetupPackage& package) {

    MSR_TURBO_RATIO_LIMIT pRatios{};
    pRatios.Maximum_Ratio_Limit_1_Core = package.PCore_RatioGroup_1.getValue();
    pRatios.Maximum_Ratio_Limit_2_Core = package.PCore_RatioGroup_2.getValue();
    pRatios.Maximum_Ratio_Limit_3_Core = package.PCore_RatioGroup_3.getValue();
    pRatios.Maximum_Ratio_Limit_4_Core = package.PCore_RatioGroup_4.getValue();
    pRatios.Maximum_Ratio_Limit_5_Core = package.PCore_RatioGroup_5.getValue();
    pRatios.Maximum_Ratio_Limit_6_Core = package.PCore_RatioGroup_6.getValue();
    pRatios.Maximum_Ratio_Limit_7_Core = package.PCore_RatioGroup_7.getValue();
    pRatios.Maximum_Ratio_Limit_8_Core = package.PCore_RatioGroup_8.getValue();
    for (int i = 0; i < package.NumPCores; ++i) {
        auto pResult = MSR::write_msr(i, MSR_TURBO_RATIO_LIMIT_ADDR, &pRatios.value);
        if (!pResult)
            return IMT_ErrCode::MsrWriteError;
    }

    MSR_SECONDARY_TURBO_RATIO_LIMIT eRatios{};
    eRatios.Maximum_Ratio_Limit_1_Core = package.ECore_RatioGroup_1.getValue();
    eRatios.Maximum_Ratio_Limit_2_Core = package.ECore_RatioGroup_2.getValue();
    eRatios.Maximum_Ratio_Limit_3_Core = package.ECore_RatioGroup_3.getValue();
    eRatios.Maximum_Ratio_Limit_4_Core = package.ECore_RatioGroup_4.getValue();
    eRatios.Maximum_Ratio_Limit_5_Core = package.ECore_RatioGroup_5.getValue();
    eRatios.Maximum_Ratio_Limit_6_Core = package.ECore_RatioGroup_6.getValue();
    eRatios.Maximum_Ratio_Limit_7_Core = package.ECore_RatioGroup_7.getValue();
    eRatios.Maximum_Ratio_Limit_8_Core = package.ECore_RatioGroup_8.getValue();
    for (int i = 0; i < package.NumECores; ++i) {
        auto eResult = MSR::write_msr(i, MSR_SECONDARY_TURBO_RATIO_LIMIT_ADDR, &eRatios.value);
        if (!eResult)
            return IMT_ErrCode::MsrWriteError;
    }

    return IMT_ErrCode::OK;
}

IMT_ErrCode MsrOps::applyCoreVF(const SetupPackage &package) {

    const auto offsets = package.getVfCoreOffsetPoints();
    for (int i = 0; i < offsets.size(); ++i) {
        OcMailbox::writeVFOffsetRaw(i + 1, offsets[i]);
    }

    return IMT_ErrCode::OK;
}

IMT_ErrCode MsrOps::applyVOffset(const SetupPackage& package) {

    constexpr auto applyVOffsetCmd ( [](const double offset, const uint64_t domCmdWrite) -> IMT_ErrCode {
        OcMailbox::OC_MAILBOX_MSR writeCmd{};
        bool writeResult = false;
        auto offset_mV = offset;
        writeCmd.value = OcMailbox::buildVidOffsetMsrValue(domCmdWrite, offset_mV);
        auto voltageToRead = OcMailbox::convertToVidOffset(writeCmd.Offset);
        writeResult = MSR::write_msr(0, OC_MAILBOX_ADDR, &writeCmd.value);
        if (!writeResult)
            return IMT_ErrCode::MsrWriteError;

        auto voltageReread = OcMailbox::readVidOffsetRaw(domCmdWrite);

        if (voltageToRead != voltageReread) {
            OcMailbox::OC_MAILBOX_MSR resetCmd{};
            resetCmd.value = OcMailbox::buildVidOffsetMsrValue(domCmdWrite, 0.0);
            MSR::write_msr(0, OC_MAILBOX_ADDR, &resetCmd.value);
            return IMT_ErrCode::MsrVoltageNotAppliedCorrectly;
        }
        return IMT_ErrCode::OK;
    });

    auto EC = IMT_ErrCode::OK;
    IMT_CHECK_AND_RETURN(EC, applyVOffsetCmd(package.V_Offset_VCore.getValue(),         CORE_WRITE_CMD));
    IMT_CHECK_AND_RETURN(EC, applyVOffsetCmd(package.V_Offset_IGpu.getValue(),          IGPU_WRITE_CMD));
    IMT_CHECK_AND_RETURN(EC, applyVOffsetCmd(package.V_Offset_Cache.getValue(),         CACHE_WRITE_CMD));
    IMT_CHECK_AND_RETURN(EC, applyVOffsetCmd(package.V_Offset_SystemAgent.getValue(),   SA_WRITE_CMD));
    IMT_CHECK_AND_RETURN(EC, applyVOffsetCmd(package.V_Offset_AnalogIO.getValue(),      ANALOG_WRITE_CMD));
    IMT_CHECK_AND_RETURN(EC, applyVOffsetCmd(package.V_Offset_DigitalIO.getValue(),     DIGITAL_WRITE_CMD));
    //IMT_CHECK_AND_RETURN(EC, applyVOffsetCmd(package.V_Offset_ECache.getValue(),        E_CACHE_WRITE_CMD));

    return EC;
}

IMT_ErrCode MsrOps::applyHWP(const SetupPackage& package) {

    // Turn off per core control
    for (int i = 0; i < package.NumPCores + package.NumPCores; ++i) {
        IA32_HWP_REQUEST hwp{};
        auto readResult = MSR::read_msr(i, IA32_HWP_REQUEST_ADDR, &hwp.value);
        if (!readResult)
            return IMT_ErrCode::MsrReadError;
        hwp.Package_Control = 1;
        hwp.Maximum_Valid = 0;
        hwp.Desired_Valid = 0;
        hwp.Minimum_Valid = 0;
        hwp.EPP_Valid = 0;
        auto writeResult = MSR::write_msr(i, IA32_HWP_REQUEST_ADDR, &hwp.value);
        if (!writeResult)
            return IMT_ErrCode::MsrWriteError;
    }

    // Set target HWP Request
    for (int i = 0; i < package.NumPCores + package.NumPCores; i++) {
        IA32_HWP_REQUEST_PKG hwpPkg{};
        auto readResult = MSR::read_msr(i, IA32_HWP_REQUEST_PKG_ADDR, &hwpPkg.value);
        if (!readResult)
            return IMT_ErrCode::MsrReadError;
        hwpPkg.Maximum_Performance = package.HWP_Maximum.getValue();
        hwpPkg.Minimum_Performance = package.HWP_Minimum.getValue();
        hwpPkg.Desired_Performance = package.HWP_Desired.getValue();
        hwpPkg.Energy_Performance_Preference = package.HWP_EPP.getValue();;
        auto writeResult = MSR::write_msr(i, IA32_HWP_REQUEST_PKG_ADDR, &hwpPkg.value);
        if (!writeResult)
            return IMT_ErrCode::MsrWriteError;
    }

    return IMT_ErrCode::OK;
}

IMT_ErrCode MsrOps::applyMisc(const SetupPackage& package) {

    for (int i = 0; i < package.NumPCores + package.NumPCores; ++i) {
        MSR_POWER_CTL pCtl{};
        auto readResult = MSR::read_msr(i, MSR_POWER_CTL_ADDR, &pCtl.value);
        if (!readResult)
            return IMT_ErrCode::MsrReadError;
        pCtl.C1E_Enable = package.EnhancedHalt_C1E.getValue();
        pCtl.Disable_Energy_Eff_Optimization = package.Disable_EE_Optimization.getValue();
        auto writeResult = MSR::write_msr(i, MSR_POWER_CTL_ADDR, &pCtl.value);
        if (!writeResult)
            return IMT_ErrCode::MsrWriteError;

        MSR_RING_RATIO ring{};
        ring.RingRatioMin = package.RingMin.getValue();
        ring.RingRatioMax = package.RingMax.getValue();
        writeResult = MSR::write_msr(i, MSR_RING_RATIO_ADDR, &ring.value);
        if (!writeResult)
            return IMT_ErrCode::MsrWriteError;
    }

    return IMT_ErrCode::OK;
}


