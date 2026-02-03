//
// Created by pierre on 2025-06-07.
//

#ifndef ARGTYPE_H
#define ARGTYPE_H
#include <cstdint>

// Do not alter the order of this enum!!!
enum class ArgType : uint8_t {
    Help = 0,
    Quit,
    Version,
    Continuous,
    SavePreset,
    LoadPreset,
    PresetFile,

    V_Offset_Core = 15,
    V_Offset_IGpu,
    V_Offset_Cache,
    V_Offset_SA,
    V_Offset_Analog,
    V_Offset_Digital,
    V_Offset_ECache,

    Ratio_P_Group1 = 23,
    Ratio_P_Group2,
    Ratio_P_Group3,
    Ratio_P_Group4,
    Ratio_P_Group5,
    Ratio_P_Group6,
    Ratio_P_Group7,
    Ratio_P_Group8,
    Ratio_P_All,

    Ratio_E_Group1 = 48,
    Ratio_E_Group2,
    Ratio_E_Group3,
    Ratio_E_Group4,
    Ratio_E_Group5,
    Ratio_E_Group6,
    Ratio_E_Group7,
    Ratio_E_Group8,
    Ratio_E_All,

    C1E = 64,
    EE_Optimization,
    ApplyClocksFirst,

    HWP_USE_PACKAGE_CTRL = 80,
    HWP_PKG_Desired,
    HWP_PKG_Minimum,
    HWP_PKG_Maximum,
    HWP_PKG_EPP,

    VF_CoreOffsetPoint1 = 101,
    VF_CoreOffsetPoint2,
    VF_CoreOffsetPoint3,
    VF_CoreOffsetPoint4,
    VF_CoreOffsetPoint5,
    VF_CoreOffsetPoint6,
    VF_CoreOffsetPoint7,
    VF_CoreOffsetPoint8,
    VF_CoreOffsetPoint9,
    VF_CoreOffsetPoint10,
    VF_CoreOffsetPoint11,

    RingMin = 120,
    RingMax,

    Json = 192,

    Unknown = 255
};


#endif //ARGTYPE_H
