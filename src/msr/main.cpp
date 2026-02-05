//
// Created by oxenryd on 2026-02-03.
//

#include <format>
#include <thread>
#include <nlohmann/json.hpp>

#include "CLI.h"
#include "IMT_ErrCode.h"
#include "IntelTypes.hpp"
#include "Msr.hpp"
#include "MsrOps.h"
#include "Parameters.hpp"

using namespace CLI;

int main(const int argc, char *argv[]) {

    if (argc == 1)
        return EXIT_SUCCESS; // TODO: write basic info

    const std::vector<std::string> args(argv, argv + argc);
#ifdef DEBUG
    // if (argc > 1 && args[1] == "-json") {
    //     std::cout << "Caught JSON arg..." << std::endl;
    //     return EXIT_SUCCESS;
    // }
#endif

    if (args[1] == "-json") {
        SetupPackage sPack{};
        std::string rawStr{args[2]};
        JSon j = JSon::parse(rawStr);
        sPack.assign_from_json(rawStr);
        return static_cast<int>(MsrOps::apply(sPack));
    }

    ParamPairs params;
    bool shouldLoop = false;
    auto EC = IMT_ErrCode::OK;
    PendingSave saveData{};
    IMT_CHECK_AND_RETURN_INT(EC, validateCliArgs(args, params));

    if (params[0].first == ArgType::ReadAll) {
        const SetupPackage sPack = MsrOps::readCurrentAsPackage();
        const auto json = sPack.to_json();
        std::cout << json << std::endl;
        return EXIT_SUCCESS;
    }

    if (params[0].first == ArgType::Status) {
        auto [pCores, eCores] = MsrOps::getCoresCount();
        StatusPackage pkg{};

        pkg.pCoresCount = static_cast<uint16_t>(eCores.size());
        pkg.eCoresCount = static_cast<uint16_t>(eCores.size());
        pkg.powerUnits = {MSR::readAndReturn<MSR_RAPL_POWER_UNIT>(0, MSR_RAPL_POWER_UNIT_ADDR)};
        pkg.energyStatus = {MSR::readAndReturn<MSR_PKG_ENERGY_STATUS>(pCores[0], MSR_PKG_ENERGY_STATUS_ADDR)};
        pkg.pcorePerfStats = {MSR::readAndReturn<IA32_PERF_STATUS>(pCores[0], IA32_PERF_STATUS_ADDR)};
        pkg.ecorePerfStats = {MSR::readAndReturn<IA32_PERF_STATUS>(eCores[0], IA32_PERF_STATUS_ADDR)};
        pkg.tempTarget = {MSR::readAndReturn<MSR_TEMPERATURE_TARGET>(pCores[0], MSR_TEMPERATURE_TARGET_ADDR)};
        pkg.packageTherm = {MSR::readAndReturn<IA32_PACKAGE_THERM_STATUS>(pCores[0], IA32_PACKAGE_THERM_STATUS_ADDR)};

        const auto json = pkg.to_json();
        std::cout << json << std::endl;
        return EXIT_SUCCESS;

    }

    IMT_CHECK_AND_RETURN_INT(EC, execute(params, shouldLoop, saveData));
    // if (saveData.pending) {
    //     IMT_CHECK_AND_RETURN_INT(EC, MsrOps::saveCurrentSetting(saveData.path, saveData.name, saveData.presetIndex));
    // } else
    //     return print(shouldLoop);

    return EXIT_SUCCESS;
}