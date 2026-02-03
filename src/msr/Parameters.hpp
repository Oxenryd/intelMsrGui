//
// Created by pierre on 2025-06-05.
//

#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <cstdint>
#include <cstring>
#include <string>
#include <iostream>
#include <nlohmann/json.hpp>
#include "ArgType.h"

union ValueUnion {
    double asDouble;
    uint64_t asUInt;
    bool asBool;

    constexpr explicit ValueUnion() = delete;
    constexpr explicit ValueUnion(const double d) : asDouble(d) {}
    constexpr explicit ValueUnion(const uint64_t u) : asUInt(u) {}
    constexpr explicit ValueUnion(const bool b) : asBool(b) {}

};

template <typename T>
struct ArgValue {
    static_assert(std::is_same_v<T, double> ||
                std::is_same_v<T, uint64_t> ||
                std::is_same_v<T, bool>,
                  "ArgValue<T> only supports double, bool, uint64_t");

    ValueUnion value;
    constexpr ArgValue(const ArgValue& other) : value{other.get()} {}
    constexpr explicit ArgValue(T v) : value{v} {}

    constexpr void set(T v) {
        if constexpr (std::is_same_v<T, double>) {
            value.asDouble = v;
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            value.asUInt = v;
        } else if constexpr (std::is_same_v<T, bool>) {
            value.asBool = v;
        } else {
            static_assert(std::is_same_v<T, void>, "Unsupported type in ArgValue::set(T)");
        }
    }

    [[nodiscard]] constexpr T get() const {
        if constexpr (std::is_same_v<T, double>) {
            return value.asDouble;
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            return value.asUInt;
        } else if constexpr (std::is_same_v<T, bool>) {
            return value.asBool;
        } else {
            static_assert(std::is_same_v<T, void>, "Unsupported type in ArgValue::get()");
            return nullptr;
        }
    }
};

template <ArgType ArgT, typename ValT>
struct ImtParam {
    static_assert(
        std::is_same_v<ValT, double> ||
        std::is_same_v<ValT, uint64_t> ||
        std::is_same_v<ValT, bool>,
            "Only double, bool, uint64_t are allowed");

    explicit ImtParam(ValT val) : value{val} {}
    static constexpr ArgType type = ArgT;
    [[nodiscard]] ValT getValue() const { return value.get(); }
    void set(ValT val) { value.set(val); }

    ArgValue<ValT> value;

    std::string to_string() {
        //if constexpr (std::is_same_v<ValT, double>) {}
        return std::to_string(getValue());
    }
};


struct ArgMapping {
    const char* name;
    const ArgType type;
    const uint8_t num_args;
};

constexpr ArgMapping ARGS_DEF[] {
    {"-h", ArgType::Help, 0},
    {"--help", ArgType::Help, 0},
    {"-v", ArgType::Version, 0},
    {"--version", ArgType::Version, 0},
    {"-c", ArgType::ApplyClocksFirst, 1},
    {"-l", ArgType::Continuous, 0},
    {"--loop", ArgType::Continuous, 0},
    {"-s", ArgType::SavePreset, 1},
    {"-o", ArgType::LoadPreset, 1},
    {"--save", ArgType::SavePreset, 1},
    {"--open", ArgType::LoadPreset, 1},
    {"-q", ArgType::Quit, 0},
    {"--quit", ArgType::Quit, 0},
    {"--presetfile", ArgType::PresetFile, 1},
    {"-pf", ArgType::PresetFile, 1},

    {"-vcore", ArgType::V_Offset_Core, 1},
    {"-vigpu", ArgType::V_Offset_IGpu, 1},
    {"-vcache", ArgType::V_Offset_Cache, 1},
    {"-vsa", ArgType::V_Offset_SA, 1},
    {"-vanalog", ArgType::V_Offset_Analog, 1},
    {"-vdigital", ArgType::V_Offset_Digital, 1},
  //  {"-vecache", ArgType::V_Offset_ECache, 1},

    {"-rp1", ArgType::Ratio_P_Group1, 1},
    {"-rp2", ArgType::Ratio_P_Group2, 1},
    {"-rp3", ArgType::Ratio_P_Group3, 1},
    {"-rp4", ArgType::Ratio_P_Group4, 1},
    {"-rp5", ArgType::Ratio_P_Group5, 1},
    {"-rp6", ArgType::Ratio_P_Group6, 1},
    {"-rp7", ArgType::Ratio_P_Group7, 1},
    {"-rp8", ArgType::Ratio_P_Group8, 1},
    {"-rp", ArgType::Ratio_P_All, 1},

    {"-re1", ArgType::Ratio_E_Group1, 1},
    {"-re2", ArgType::Ratio_E_Group2, 1},
    {"-re3", ArgType::Ratio_E_Group3, 1},
    {"-re4", ArgType::Ratio_E_Group4, 1},
    {"-re5", ArgType::Ratio_E_Group5, 1},
    {"-re6", ArgType::Ratio_E_Group6, 1},
    {"-re7", ArgType::Ratio_E_Group7, 1},
    {"-re8", ArgType::Ratio_E_Group8, 1},
    {"-re", ArgType::Ratio_E_All, 1},

    {"-c1e", ArgType::C1E, 1},
    {"-eeo", ArgType::EE_Optimization, 1},

    {"-hwppkg", ArgType::HWP_USE_PACKAGE_CTRL, 1},
    {"-hwpdes", ArgType::HWP_PKG_Desired, 1},
    {"-hwpmin", ArgType::HWP_PKG_Minimum, 1},
    {"-hwpmax", ArgType::HWP_PKG_Maximum, 1},
    {"-hwpepp", ArgType::HWP_PKG_EPP, 1},

    {"-rMin", ArgType::RingMin, 1},
    {"-rMax", ArgType::RingMax, 1},

    {"-vf1", ArgType::VF_CoreOffsetPoint1, 1},
    {"-vf2", ArgType::VF_CoreOffsetPoint1, 2},
    {"-vf3", ArgType::VF_CoreOffsetPoint1, 3},
    {"-vf4", ArgType::VF_CoreOffsetPoint1, 4},
    {"-vf5", ArgType::VF_CoreOffsetPoint1, 5},
    {"-vf6", ArgType::VF_CoreOffsetPoint1, 6},
    {"-vf7", ArgType::VF_CoreOffsetPoint1, 7},
    {"-vf8", ArgType::VF_CoreOffsetPoint1, 8},
    {"-vf9", ArgType::VF_CoreOffsetPoint1, 9},
    {"-vf10", ArgType::VF_CoreOffsetPoint1, 10},
    {"-vf11", ArgType::VF_CoreOffsetPoint1, 11},

    {"-json", ArgType::Json, 0}
};
constexpr size_t NUM_ARGS_DEF = std::size(ARGS_DEF);

static ArgType getArgType(const char* arg) {
    for (auto mapping : ARGS_DEF) {
        if (std::strcmp(arg, mapping.name) == 0) {
            return mapping.type;
        }
    }
    return ArgType::Unknown;
}

constexpr ArgType getArgType(const std::string& str) {
    return getArgType(str.c_str());
}

constexpr uint8_t getArgNum(const ArgType type) {
    for (const auto mapping : ARGS_DEF) {
        if (mapping.type == type) {
            return mapping.num_args;
        }
    }
    return static_cast<uint8_t>(-1);
}

constexpr uint8_t getArgNum(const std::string &str) {
    return getArgNum(getArgType(str));
}

constexpr const char* getArgString(const ArgType type) {
    for (auto mapping : ARGS_DEF) {
        if (type == mapping.type)
            return mapping.name;
    }
    return "Unknown";
}

// Serialization
using JSon = nlohmann::json;
#define TYPE "type"
#define VALUE "value"
#define P_CORES "pCores"
#define E_CORES "eCores"
#define CLOCKS_FIRST "clocksFirst"

template <ArgType ArgT, typename ValT>
JSon param_to_json(const ImtParam<ArgT, ValT>& param) {
    JSon j;
    try {
        j[TYPE] = static_cast<uint8_t>(ArgT);
        j[VALUE] = param.getValue();
    } catch (const std::exception& e) {
        std::cerr << "ERROR in param_to_json(): " << e.what() << std::endl;
    }

    return j;
}

struct SetupPackage {

    explicit SetupPackage() = default;
    explicit SetupPackage(const JSon& j) {
        assign_from_json(j);
    }

    [[nodiscard]] JSon to_json() const {
        JSon j = JSon::array();

        j.push_back(param_to_json(V_Offset_VCore));
        j.push_back(param_to_json(V_Offset_IGpu));
        j.push_back(param_to_json(V_Offset_Cache));
        j.push_back(param_to_json(V_Offset_SystemAgent));
        j.push_back(param_to_json(V_Offset_AnalogIO));
        j.push_back(param_to_json(V_Offset_DigitalIO));
        j.push_back(param_to_json(V_Offset_ECache));

        j.push_back(param_to_json(EnhancedHalt_C1E));
        j.push_back(param_to_json(Disable_EE_Optimization));

        j.push_back(param_to_json(PCore_RatioGroup_1));
        j.push_back(param_to_json(PCore_RatioGroup_2));
        j.push_back(param_to_json(PCore_RatioGroup_3));
        j.push_back(param_to_json(PCore_RatioGroup_4));
        j.push_back(param_to_json(PCore_RatioGroup_5));
        j.push_back(param_to_json(PCore_RatioGroup_6));
        j.push_back(param_to_json(PCore_RatioGroup_7));
        j.push_back(param_to_json(PCore_RatioGroup_8));

        j.push_back(param_to_json(ECore_RatioGroup_1));
        j.push_back(param_to_json(ECore_RatioGroup_2));
        j.push_back(param_to_json(ECore_RatioGroup_3));
        j.push_back(param_to_json(ECore_RatioGroup_4));
        j.push_back(param_to_json(ECore_RatioGroup_5));
        j.push_back(param_to_json(ECore_RatioGroup_6));
        j.push_back(param_to_json(ECore_RatioGroup_7));
        j.push_back(param_to_json(ECore_RatioGroup_8));

        j.push_back(param_to_json(HWP_PackageControl));
        j.push_back(param_to_json(HWP_Desired));
        j.push_back(param_to_json(HWP_Minimum));
        j.push_back(param_to_json(HWP_Maximum));
        j.push_back(param_to_json(HWP_EPP));

        j.push_back(param_to_json(VF_CoreOffsetPoint1));
        j.push_back(param_to_json(VF_CoreOffsetPoint2));
        j.push_back(param_to_json(VF_CoreOffsetPoint3));
        j.push_back(param_to_json(VF_CoreOffsetPoint4));
        j.push_back(param_to_json(VF_CoreOffsetPoint5));
        j.push_back(param_to_json(VF_CoreOffsetPoint6));
        j.push_back(param_to_json(VF_CoreOffsetPoint7));
        j.push_back(param_to_json(VF_CoreOffsetPoint8));
        j.push_back(param_to_json(VF_CoreOffsetPoint9));
        j.push_back(param_to_json(VF_CoreOffsetPoint10));
        j.push_back(param_to_json(VF_CoreOffsetPoint11));
        j.push_back(param_to_json(RingMin));
        j.push_back(param_to_json(RingMax));

        auto p = nlohmann::json::object();
        auto e = nlohmann::json::object();
        auto cf = nlohmann::json::object();
        p[P_CORES] = NumPCores;
        e[E_CORES] = NumECores;
        cf[CLOCKS_FIRST] = ApplyClocksFirst;
        j.push_back(p);
        j.push_back(e);
        j.push_back(cf);

        return j;
    }

    uint16_t NumPCores = static_cast<uint16_t>(-1);
    uint16_t NumECores = static_cast<uint16_t>(-1);
    bool ApplyClocksFirst = true;
    bool HWP_IsSet = false;

    ImtParam<ArgType::V_Offset_Core, double> V_Offset_VCore{0.0};
    ImtParam<ArgType::V_Offset_IGpu, double> V_Offset_IGpu{0.0};
    ImtParam<ArgType::V_Offset_Cache, double> V_Offset_Cache{0.0};
    ImtParam<ArgType::V_Offset_SA, double> V_Offset_SystemAgent{0.0};
    ImtParam<ArgType::V_Offset_Analog, double> V_Offset_AnalogIO{0.0};
    ImtParam<ArgType::V_Offset_Digital, double> V_Offset_DigitalIO{0.0};
    ImtParam<ArgType::V_Offset_ECache, double> V_Offset_ECache{0.0};

    ImtParam<ArgType::C1E, bool> EnhancedHalt_C1E{true};
    ImtParam<ArgType::EE_Optimization, bool> Disable_EE_Optimization{false};

    ImtParam<ArgType::Ratio_P_Group1, uint64_t> PCore_RatioGroup_1{static_cast<uint64_t>(-1)};
    ImtParam<ArgType::Ratio_P_Group2, uint64_t> PCore_RatioGroup_2{static_cast<uint64_t>(-1)};
    ImtParam<ArgType::Ratio_P_Group3, uint64_t> PCore_RatioGroup_3{static_cast<uint64_t>(-1)};
    ImtParam<ArgType::Ratio_P_Group4, uint64_t> PCore_RatioGroup_4{static_cast<uint64_t>(-1)};
    ImtParam<ArgType::Ratio_P_Group5, uint64_t> PCore_RatioGroup_5{static_cast<uint64_t>(-1)};
    ImtParam<ArgType::Ratio_P_Group6, uint64_t> PCore_RatioGroup_6{static_cast<uint64_t>(-1)};
    ImtParam<ArgType::Ratio_P_Group7, uint64_t> PCore_RatioGroup_7{static_cast<uint64_t>(-1)};
    ImtParam<ArgType::Ratio_P_Group8, uint64_t> PCore_RatioGroup_8{static_cast<uint64_t>(-1)};

    ImtParam<ArgType::Ratio_E_Group1, uint64_t> ECore_RatioGroup_1{static_cast<uint64_t>(-1)};
    ImtParam<ArgType::Ratio_E_Group2, uint64_t> ECore_RatioGroup_2{static_cast<uint64_t>(-1)};
    ImtParam<ArgType::Ratio_E_Group3, uint64_t> ECore_RatioGroup_3{static_cast<uint64_t>(-1)};
    ImtParam<ArgType::Ratio_E_Group4, uint64_t> ECore_RatioGroup_4{static_cast<uint64_t>(-1)};
    ImtParam<ArgType::Ratio_E_Group5, uint64_t> ECore_RatioGroup_5{static_cast<uint64_t>(-1)};
    ImtParam<ArgType::Ratio_E_Group6, uint64_t> ECore_RatioGroup_6{static_cast<uint64_t>(-1)};
    ImtParam<ArgType::Ratio_E_Group7, uint64_t> ECore_RatioGroup_7{static_cast<uint64_t>(-1)};
    ImtParam<ArgType::Ratio_E_Group8, uint64_t> ECore_RatioGroup_8{static_cast<uint64_t>(-1)};

    ImtParam<ArgType::HWP_USE_PACKAGE_CTRL, bool> HWP_PackageControl{false};
    ImtParam<ArgType::HWP_PKG_Desired, uint64_t> HWP_Desired{static_cast<uint64_t>(-1)};
    ImtParam<ArgType::HWP_PKG_Minimum, uint64_t> HWP_Minimum{static_cast<uint64_t>(-1)};
    ImtParam<ArgType::HWP_PKG_Maximum, uint64_t> HWP_Maximum{static_cast<uint64_t>(-1)};
    ImtParam<ArgType::HWP_PKG_EPP, uint64_t> HWP_EPP{static_cast<uint64_t>(-1)};

    ImtParam<ArgType::VF_CoreOffsetPoint1, double> VF_CoreOffsetPoint1{0.0};
    ImtParam<ArgType::VF_CoreOffsetPoint2, double> VF_CoreOffsetPoint2{0.0};
    ImtParam<ArgType::VF_CoreOffsetPoint3, double> VF_CoreOffsetPoint3{0.0};
    ImtParam<ArgType::VF_CoreOffsetPoint4, double> VF_CoreOffsetPoint4{0.0};
    ImtParam<ArgType::VF_CoreOffsetPoint5, double> VF_CoreOffsetPoint5{0.0};
    ImtParam<ArgType::VF_CoreOffsetPoint6, double> VF_CoreOffsetPoint6{0.0};
    ImtParam<ArgType::VF_CoreOffsetPoint7, double> VF_CoreOffsetPoint7{0.0};
    ImtParam<ArgType::VF_CoreOffsetPoint8, double> VF_CoreOffsetPoint8{0.0};
    ImtParam<ArgType::VF_CoreOffsetPoint9, double> VF_CoreOffsetPoint9{0.0};
    ImtParam<ArgType::VF_CoreOffsetPoint10, double> VF_CoreOffsetPoint10{0.0};
    ImtParam<ArgType::VF_CoreOffsetPoint11, double> VF_CoreOffsetPoint11{0.0};

    ImtParam<ArgType::RingMin, uint64_t> RingMin{static_cast<uint64_t>(-1)};
    ImtParam<ArgType::RingMax, uint64_t> RingMax{static_cast<uint64_t>(-1)};

    [[nodiscard]] std::vector<double> getVfCoreOffsetPoints() const {
        std::vector<double> vfCoreOffsetPoints;
        vfCoreOffsetPoints.push_back(VF_CoreOffsetPoint1.getValue());
        vfCoreOffsetPoints.push_back(VF_CoreOffsetPoint2.getValue());
        vfCoreOffsetPoints.push_back(VF_CoreOffsetPoint3.getValue());
        vfCoreOffsetPoints.push_back(VF_CoreOffsetPoint4.getValue());
        vfCoreOffsetPoints.push_back(VF_CoreOffsetPoint5.getValue());
        vfCoreOffsetPoints.push_back(VF_CoreOffsetPoint6.getValue());
        vfCoreOffsetPoints.push_back(VF_CoreOffsetPoint7.getValue());
        vfCoreOffsetPoints.push_back(VF_CoreOffsetPoint8.getValue());
        vfCoreOffsetPoints.push_back(VF_CoreOffsetPoint9.getValue());
        vfCoreOffsetPoints.push_back(VF_CoreOffsetPoint10.getValue());
        vfCoreOffsetPoints.push_back(VF_CoreOffsetPoint11.getValue());
        return vfCoreOffsetPoints;
    }

    [[nodiscard]] std::vector<uint64_t> getPCoreRatioGroups() const {
        std::vector<uint64_t> ratioGroups;
        ratioGroups.push_back(PCore_RatioGroup_1.getValue());
        ratioGroups.push_back(PCore_RatioGroup_2.getValue());
        ratioGroups.push_back(PCore_RatioGroup_3.getValue());
        ratioGroups.push_back(PCore_RatioGroup_4.getValue());
        ratioGroups.push_back(PCore_RatioGroup_5.getValue());
        ratioGroups.push_back(PCore_RatioGroup_6.getValue());
        ratioGroups.push_back(PCore_RatioGroup_7.getValue());
        ratioGroups.push_back(PCore_RatioGroup_8.getValue());
        return ratioGroups;
    }
    [[nodiscard]] std::vector<uint64_t> getECoreRatioGroups() const {
        std::vector<uint64_t> ratioGroups;
        ratioGroups.push_back(ECore_RatioGroup_1.getValue());
        ratioGroups.push_back(ECore_RatioGroup_2.getValue());
        ratioGroups.push_back(ECore_RatioGroup_3.getValue());
        ratioGroups.push_back(ECore_RatioGroup_4.getValue());
        ratioGroups.push_back(ECore_RatioGroup_5.getValue());
        ratioGroups.push_back(ECore_RatioGroup_6.getValue());
        ratioGroups.push_back(ECore_RatioGroup_7.getValue());
        ratioGroups.push_back(ECore_RatioGroup_8.getValue());
        return ratioGroups;
    }

    void setVfCoreOffsets(const std::vector<double>& vfOffsets) {
        assert(vfOffsets.size() == 11);
        VF_CoreOffsetPoint1.set(vfOffsets[0]);
        VF_CoreOffsetPoint2.set(vfOffsets[1]);
        VF_CoreOffsetPoint3.set(vfOffsets[2]);
        VF_CoreOffsetPoint4.set(vfOffsets[3]);
        VF_CoreOffsetPoint5.set(vfOffsets[4]);
        VF_CoreOffsetPoint6.set(vfOffsets[5]);
        VF_CoreOffsetPoint7.set(vfOffsets[6]);
        VF_CoreOffsetPoint8.set(vfOffsets[7]);
        VF_CoreOffsetPoint9.set(vfOffsets[8]);
        VF_CoreOffsetPoint10.set(vfOffsets[9]);
        VF_CoreOffsetPoint11.set(vfOffsets[10]);
    }

    void setPCoreRatioGroups(const std::vector<uint64_t>& ratioGroups) {
        assert(ratioGroups.size() == 8);
        PCore_RatioGroup_1.set(ratioGroups[0]);
        PCore_RatioGroup_2.set(ratioGroups[1]);
        PCore_RatioGroup_3.set(ratioGroups[2]);
        PCore_RatioGroup_4.set(ratioGroups[3]);
        PCore_RatioGroup_5.set(ratioGroups[4]);
        PCore_RatioGroup_6.set(ratioGroups[5]);
        PCore_RatioGroup_7.set(ratioGroups[6]);
        PCore_RatioGroup_8.set(ratioGroups[7]);
    }

    void setECoreRatioGroups(const std::vector<uint64_t>& ratioGroups) {
        assert(ratioGroups.size() == 8);
        ECore_RatioGroup_1.set(ratioGroups[0]);
        ECore_RatioGroup_2.set(ratioGroups[1]);
        ECore_RatioGroup_3.set(ratioGroups[2]);
        ECore_RatioGroup_4.set(ratioGroups[3]);
        ECore_RatioGroup_5.set(ratioGroups[4]);
        ECore_RatioGroup_6.set(ratioGroups[5]);
        ECore_RatioGroup_7.set(ratioGroups[6]);
        ECore_RatioGroup_8.set(ratioGroups[7]);
    }

    void setGlobalOffsets(const std::vector<double>& offsets) {
        assert(offsets.size() == 6);
        V_Offset_VCore.set(offsets[0]);
        V_Offset_IGpu.set(offsets[1]);
        V_Offset_Cache.set(offsets[2]);
        V_Offset_SystemAgent.set(offsets[3]);
        V_Offset_AnalogIO.set(offsets[4]);
        V_Offset_DigitalIO.set(offsets[5]);
    }

    void assign_from_json(const JSon& j) {
        for (const auto& obj : j) {

            if (obj.contains(P_CORES)) {
                NumPCores = obj[P_CORES].get<uint16_t>();
                continue;
            }
            if (obj.contains(E_CORES)) {
                NumECores = obj[E_CORES].get<uint16_t>();
                continue;
            }
            if (obj.contains(CLOCKS_FIRST)) {
                ApplyClocksFirst = obj[CLOCKS_FIRST].get<bool>();
                continue;
            }

            if (obj.contains(TYPE)) {

                auto type = obj[TYPE].get<int>();
                switch ( static_cast<ArgType>(type) ) {
                    default: break;

                    case ArgType::V_Offset_Core:
                        try {
                            V_Offset_VCore.set(obj[VALUE].get<double>());
                        } catch (const std::exception& e) {
                            std::cerr << "In assign_from_json:VCore: " << e.what() << std::endl;
                        } break;
                    case ArgType::V_Offset_IGpu:
                        V_Offset_IGpu.set(obj[VALUE].get<double>()); break;
                    case ArgType::V_Offset_Cache:
                        V_Offset_Cache.set(obj[VALUE].get<double>()); break;
                    case ArgType::V_Offset_SA:
                        V_Offset_SystemAgent.set(obj[VALUE].get<double>()); break;
                    case ArgType::V_Offset_Analog:
                        V_Offset_AnalogIO.set(obj[VALUE].get<double>()); break;
                    case ArgType::V_Offset_Digital:
                        V_Offset_DigitalIO.set(obj[VALUE].get<double>()); break;
                    case ArgType::V_Offset_ECache:
                        V_Offset_ECache.set(obj[VALUE].get<double>()); break;

                    case ArgType::C1E:
                        try {
                            EnhancedHalt_C1E.set(obj[VALUE].get<bool>());
                        } catch (const std::exception& e) {
                            std::cerr << "In assign_from_json:C1E: " << e.what() << std::endl;
                        } break;
                    case ArgType::EE_Optimization:
                        try {
                            Disable_EE_Optimization.set(obj[VALUE].get<bool>());
                        } catch (const std::exception& e) {
                            std::cerr << "In assign_from_json:Disable_EE: " << e.what() << std::endl;
                        } break;

                    case ArgType::Ratio_P_Group1:
                        PCore_RatioGroup_1.set(obj[VALUE].get<uint64_t>()); break;
                    case ArgType::Ratio_P_Group2:
                        PCore_RatioGroup_2.set(obj[VALUE].get<uint64_t>()); break;
                    case ArgType::Ratio_P_Group3:
                        PCore_RatioGroup_3.set(obj[VALUE].get<uint64_t>()); break;
                    case ArgType::Ratio_P_Group4:
                        PCore_RatioGroup_4.set(obj[VALUE].get<uint64_t>()); break;
                    case ArgType::Ratio_P_Group5:
                        PCore_RatioGroup_5.set(obj[VALUE].get<uint64_t>()); break;
                    case ArgType::Ratio_P_Group6:
                        PCore_RatioGroup_6.set(obj[VALUE].get<uint64_t>()); break;
                    case ArgType::Ratio_P_Group7:
                        PCore_RatioGroup_7.set(obj[VALUE].get<uint64_t>()); break;
                    case ArgType::Ratio_P_Group8:
                        PCore_RatioGroup_8.set(obj[VALUE].get<uint64_t>()); break;

                    case ArgType::Ratio_E_Group1:
                        ECore_RatioGroup_1.set(obj[VALUE].get<uint64_t>()); break;
                    case ArgType::Ratio_E_Group2:
                        ECore_RatioGroup_2.set(obj[VALUE].get<uint64_t>()); break;
                    case ArgType::Ratio_E_Group3:
                        ECore_RatioGroup_3.set(obj[VALUE].get<uint64_t>()); break;
                    case ArgType::Ratio_E_Group4:
                        ECore_RatioGroup_4.set(obj[VALUE].get<uint64_t>()); break;
                    case ArgType::Ratio_E_Group5:
                        ECore_RatioGroup_5.set(obj[VALUE].get<uint64_t>()); break;
                    case ArgType::Ratio_E_Group6:
                        ECore_RatioGroup_6.set(obj[VALUE].get<uint64_t>()); break;
                    case ArgType::Ratio_E_Group7:
                        ECore_RatioGroup_7.set(obj[VALUE].get<uint64_t>()); break;
                    case ArgType::Ratio_E_Group8:
                        ECore_RatioGroup_8.set(obj[VALUE].get<uint64_t>()); break;

                    case ArgType::HWP_USE_PACKAGE_CTRL:
                        try {
                            HWP_PackageControl.set(obj[VALUE].get<bool>());
                        } catch (const std::exception& e) {
                            std::cerr << "In assign_from_json:HWP_USE_PKG_CTL: " << e.what() << std::endl;
                        } break;

                    case ArgType::HWP_PKG_Desired:
                        HWP_Desired.set(obj[VALUE].get<uint64_t>()); break;
                    case ArgType::HWP_PKG_Minimum:
                        HWP_Minimum.set(obj[VALUE].get<uint64_t>()); break;
                    case ArgType::HWP_PKG_Maximum:
                        HWP_Maximum.set(obj[VALUE].get<uint64_t>()); break;
                    case ArgType::HWP_PKG_EPP:
                        HWP_EPP.set(obj[VALUE].get<uint64_t>()); break;

                    case ArgType::VF_CoreOffsetPoint1:
                        VF_CoreOffsetPoint1.set(obj[VALUE].get<double>()); break;
                    case ArgType::VF_CoreOffsetPoint2:
                        VF_CoreOffsetPoint2.set(obj[VALUE].get<double>()); break;
                    case ArgType::VF_CoreOffsetPoint3:
                        VF_CoreOffsetPoint3.set(obj[VALUE].get<double>()); break;
                    case ArgType::VF_CoreOffsetPoint4:
                        VF_CoreOffsetPoint4.set(obj[VALUE].get<double>()); break;
                    case ArgType::VF_CoreOffsetPoint5:
                        VF_CoreOffsetPoint5.set(obj[VALUE].get<double>()); break;
                    case ArgType::VF_CoreOffsetPoint6:
                        VF_CoreOffsetPoint6.set(obj[VALUE].get<double>()); break;
                    case ArgType::VF_CoreOffsetPoint7:
                        VF_CoreOffsetPoint7.set(obj[VALUE].get<double>()); break;
                    case ArgType::VF_CoreOffsetPoint8:
                        VF_CoreOffsetPoint8.set(obj[VALUE].get<double>()); break;
                    case ArgType::VF_CoreOffsetPoint9:
                        VF_CoreOffsetPoint9.set(obj[VALUE].get<double>()); break;
                    case ArgType::VF_CoreOffsetPoint10:
                        VF_CoreOffsetPoint10.set(obj[VALUE].get<double>()); break;
                    case ArgType::VF_CoreOffsetPoint11:
                        VF_CoreOffsetPoint11.set(obj[VALUE].get<double>()); break;

                    case ArgType::RingMin:
                        RingMin.set(obj[VALUE].get<uint64_t>()); break;
                    case ArgType::RingMax:
                        RingMax.set(obj[VALUE].get<uint64_t>()); break;
                }
            }
        }
    }
};

struct PendingSave {
    bool pending = false;
    uint16_t presetIndex;
    std::string path;
    std::string name;
};



#endif //PARAMETERS_H
