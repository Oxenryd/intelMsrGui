//
// Created by pierre on 2025-06-02.
//

#ifndef PRESETS_H
#define PRESETS_H

#include <utility>
#include <vector>
#include <cstdint>
#include <string>
#include <unordered_map>

struct SetupPackage;
enum class IMT_ErrCode : uint8_t;
namespace MsrOps {

    std::pair<std::vector<int>, std::vector<int>> getCoresCount();
    SetupPackage readCurrentAsPackage();
    IMT_ErrCode readPresets(const std::string& presetFilePath, std::unordered_map<std::string, SetupPackage>* outPresets);
    IMT_ErrCode deletePreset(const std::string& presetFilePath, const std::string& presetName);
    IMT_ErrCode saveCurrentSetting(const std::string& presetFilePath, const std::string& presetName, const SetupPackage& pkg);
    IMT_ErrCode loadSetting(const std::string& presetFilePath, uint8_t presetIndex, SetupPackage** outPackage);
    IMT_ErrCode apply(const SetupPackage& package);
    IMT_ErrCode applyClocks(const SetupPackage& package);
    IMT_ErrCode applyVOffset(const SetupPackage& package);
    IMT_ErrCode applyHWP(const SetupPackage& package);
    IMT_ErrCode applyMisc(const SetupPackage& package);
    IMT_ErrCode applyCoreVF(const SetupPackage& package);

}


#endif //PRESETS_H
