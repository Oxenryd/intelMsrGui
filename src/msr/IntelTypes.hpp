//
// Created by pierre on 2025-06-01.
//
#pragma once


#ifndef INTELTYPES_H
#define INTELTYPES_H
#pragma pack(push, 1)

#include <vector>
#include <cstdint>
#include <cmath>

// Address: 770H Power management Enable
union alignas (8) IA32_PM_ENABLE {
    uint64_t value;
    struct {
        uint64_t HWP_Enabled                       : 1;
        uint64_t Reserved                          : 63;
    };
};
#define IA32_PM_ENABLE_ADDR 0x770

// Address: 774H Logical Processor Scope
/*
Typically, the operating system controls HWP operation for each logical processor via the writing of control hints /
constraints to the IA32_HWP_REQUEST MSR. The layout of the IA32_HWP_REQUEST MSR is shown in Figure 16-6.
The bit fields are described below Figure 16-6.
Operating systems can control HWP by writing both IA32_HWP_REQUEST and IA32_HWP_REQUEST_PKG MSRs
(see Section 16.4.4.2). Five valid bits within the IA32_HWP_REQUEST MSR let the operating system flexibly select
which of its five hint / constraint fields should be derived by the processor from the IA32_HWP_REQUEST MSR and
which should be derived from the IA32_HWP_REQUEST_PKG MSR. These five valid bits are supported if
CPUID.06H:EAX[bit17] is set.
When the IA32_HWP_REQUEST MSR Package Control bit is set, any valid bit that is NOT set indicates to the
processor to use the respective field value from the IA32_HWP_REQUEST_PKG MSR. Otherwise, the values are
derived from the IA32_HWP_REQUEST MSR. The valid bits are ignored when the IA32_HWP_REQUEST MSR
Package Control bit is zero.
 */
union alignas (8) IA32_HWP_REQUEST {
    uint64_t value;
    struct {
        uint64_t Minimum_Performance              : 8;
        uint64_t Maximum_Performance              : 8;
        uint64_t Desired_Performance              : 8;
        uint64_t Energy_Performance_Preference    : 8;
        uint64_t Activity_Window                  : 10;
        uint64_t Package_Control                  : 1;
        uint64_t Reserved                         : 16;
        uint64_t Activity_Window_Valid            : 1;
        uint64_t EPP_Valid                        : 1;
        uint64_t Desired_Valid                    : 1;
        uint64_t Maximum_Valid                    : 1;
        uint64_t Minimum_Valid                    : 1;
    };
};
#define IA32_HWP_REQUEST_ADDR 0x774


// Address: 772H Package Scope
/*
The structure of the IA32_HWP_REQUEST_PKG MSR (package-level) is identical to the IA32_HWP_REQUEST MSR
with the exception of the the Package Control bit field and the five valid bit fields, which do not exist in the
IA32_HWP_REQUEST_PKG MSR. Field values written to this MSR apply to all logical processors within the physical
package with the exception of logical processors whose IA32_HWP_REQUEST.Package Control field is clear (zero).
Single P-state Control mode is only supported when IA32_HWP_REQUEST_PKG is not supported.
 */
union alignas (8) IA32_HWP_REQUEST_PKG {
    uint64_t value;
    struct {
        uint64_t Minimum_Performance              : 8;
        uint64_t Maximum_Performance              : 8;
        uint64_t Desired_Performance              : 8;
        uint64_t Energy_Performance_Preference    : 8;
        uint64_t Activity_Window                  : 10;
        uint64_t Reserved                         : 22;
    };
};
#define IA32_HWP_REQUEST_PKG_ADDR 0x772

// Address: DB0H Package Enable/Disable HDC.
union alignas (8) IA32_PKG_HDC_CTL {
    uint64_t value;
    struct {
        uint64_t HDC_PKG_Enable                   : 1;
        uint64_t Reserved                         : 63;
    };
};
#define IA32_PKG_HDC_CTL_ADDR 0xdb0

// Address: 19CH Thermal Status
union alignas (8) IA32_THERM_STATUS {
    uint64_t value;
    struct {
        uint64_t Thermal_Status                   : 1;
        uint64_t Thermal_Status_Log               : 1;
        uint64_t PROCHOT_Event                    : 1;
        uint64_t PROCHOT_Log                      : 1;
        uint64_t Critical_Temp_Status             : 1;
        uint64_t Critical_Temp_Log                : 1;
        uint64_t Therm_Threshold_1_Status         : 1;
        uint64_t Therm_Threshold_1_Log            : 1;
        uint64_t Therm_Threshold_2_Status         : 1;
        uint64_t Therm_Threshold_2_Log            : 1;
        uint64_t Power_Limit_Notific_Status       : 1;
        uint64_t Power_Limit_Notific_Log          : 1;
        uint64_t Current_Limit_Status             : 1;
        uint64_t Current_Limit_Log                : 1;
        uint64_t Cross_Domain_Limit_Status        : 1;
        uint64_t Cross_Domain_Limit_Log           : 1;
        uint64_t Digital_Readout                  : 7;
        uint64_t Reserved1                        : 4;
        uint64_t Resolution_Deg_Celsius           : 4;
        uint64_t Reading_Valid                    : 1;
        uint64_t Reserved2                        : 32;
    };
};
#define IA32_THERM_STATUS_ADDR 0x19c

// Address: 1b1H PKG Thermal Status
union alignas (8) IA32_PACKAGE_THERM_STATUS {
    uint64_t value;
    struct {
        uint64_t Thermal_Status                   : 1;
        uint64_t Thermal_Status_Log               : 1;
        uint64_t PROCHOT_Event                    : 1;
        uint64_t PROCHOT_Log                      : 1;
        uint64_t Critical_Temp_Status             : 1;
        uint64_t Critical_Temp_Log                : 1;
        uint64_t Therm_Threshold_1_Status         : 1;
        uint64_t Therm_Threshold_1_Log            : 1;
        uint64_t Therm_Threshold_2_Status         : 1;
        uint64_t Therm_Threshold_2_Log            : 1;
        uint64_t Power_Limit_Notific_Status       : 1;
        uint64_t Power_Limit_Notific_Log          : 1;
        uint64_t Reserved1                        : 4;
        uint64_t Digital_Readout                  : 7;
        uint64_t Reserved2                        : 3;
        uint64_t HW_Fb_IFace_Struct_Change_status : 1;
        uint64_t Reserved3                        : 37;
    };
};
#define IA32_PACKAGE_THERM_STATUS_ADDR 0x1b1


// Address: 1a2H Temperature Targets
union alignas (8) MSR_TEMPERATURE_TARGET {
    uint64_t value;
    struct {
        uint64_t Reserved1                        : 16;
        uint64_t Temperature_Target               : 8;
        uint64_t TCC_Activation_Offset            : 8;
        uint64_t Reserved2                        : 32;

    };
};
#define MSR_TEMPERATURE_TARGET_ADDR 0x1a2




// Address: 1FCH Power Control Register
union alignas (8) MSR_POWER_CTL {
    uint64_t value;
    struct {
        uint64_t Reserved1                         : 1;
        uint64_t C1E_Enable                        : 1;
        uint64_t Reserved2                         : 17;
        uint64_t Disable_Energy_Eff_Optimization   : 1;
        uint64_t Disable_RaceToHalt                : 1;
        uint64_t Reserved3                         : 43;
    };
};
#define MSR_POWER_CTL_ADDR 0x1fc

// Address: 1ADH Primary Maximum Turbo Ratio Limit
union alignas (8) MSR_TURBO_RATIO_LIMIT {
    uint64_t value;
    struct {
        uint64_t Maximum_Ratio_Limit_1_Core       : 8;
        uint64_t Maximum_Ratio_Limit_2_Core       : 8;
        uint64_t Maximum_Ratio_Limit_3_Core       : 8;
        uint64_t Maximum_Ratio_Limit_4_Core       : 8;
        uint64_t Maximum_Ratio_Limit_5_Core       : 8;
        uint64_t Maximum_Ratio_Limit_6_Core       : 8;
        uint64_t Maximum_Ratio_Limit_7_Core       : 8;
        uint64_t Maximum_Ratio_Limit_8_Core       : 8;
    };
};
#define MSR_TURBO_RATIO_LIMIT_ADDR 0x1ad

// Address: 650H Secondary Maximum Turbo Ratio Limit
union alignas (8) MSR_SECONDARY_TURBO_RATIO_LIMIT {
    uint64_t value;
    struct {
        uint64_t Maximum_Ratio_Limit_1_Core       : 8;
        uint64_t Maximum_Ratio_Limit_2_Core       : 8;
        uint64_t Maximum_Ratio_Limit_3_Core       : 8;
        uint64_t Maximum_Ratio_Limit_4_Core       : 8;
        uint64_t Maximum_Ratio_Limit_5_Core       : 8;
        uint64_t Maximum_Ratio_Limit_6_Core       : 8;
        uint64_t Maximum_Ratio_Limit_7_Core       : 8;
        uint64_t Maximum_Ratio_Limit_8_Core       : 8;
    };
};
#define MSR_SECONDARY_TURBO_RATIO_LIMIT_ADDR 0x650


// Address: 6B1H Indicator of Frequency Clipping in the Ring Interconnect
union alignas (8) MSR_RING_PERF_LIMIT_REASONS {
    uint64_t value;
    struct {
        uint64_t PROCHOT_Status                   : 1;
        uint64_t Thermal_Status                   : 1;
        uint64_t Reserved1                        : 3;
        uint64_t Running_Avg_Therm_Limit_Status   : 1;
        uint64_t VR_Therm_Alert_Status            : 1;
        uint64_t VR_Therm_Design_Current_Status   : 1;
        uint64_t Other_Status                     : 1;
        uint64_t Reserved2                        : 1;
        uint64_t Pack_PlatLevel_Pow_Lim_PL1_Stat  : 1;
        uint64_t Pack_PlatLevel_Pow_Lim_PL2_Stat  : 1;
        uint64_t Reserved3                        : 4;

        uint64_t PROCHOT_Log                      : 1;
        uint64_t Thermal_Log                      : 1;
        uint64_t Reserved4                        : 3;
        uint64_t Running_Avg_Therm_Limit_Log      : 1;
        uint64_t VR_Therm_Alert_Log               : 1;
        uint64_t VR_Therm_Design_Current_Log      : 1;
        uint64_t Other_Log                        : 1;
        uint64_t Reserved5                        : 1;
        uint64_t Pack_PlatLevel_Pow_Lim_PL1_Log   : 1;
        uint64_t Pack_PlatLevel_Pow_Lim_PL2_Log   : 1;
        uint64_t Reserved6                        : 4;

    };
};
#define MSR_RING_PERF_LIMIT_REASONS_ADDR 0x6b1

// Address: 64FH Indicator of Frequency Clipping in the Ring Interconnect
union alignas (8) MSR_CORE_PERF_LIMIT_REASONS {
    uint64_t value;
    struct {
        uint64_t PROCHOT_Status                   : 1;
        uint64_t Thermal_Status                   : 1;
        uint64_t Reserved1                        : 2;
        uint64_t Residency_State_Regu_Status      : 1;
        uint64_t Running_Avg_Therm_Limit_Status   : 1;
        uint64_t VR_Therm_Alert_Status            : 1;
        uint64_t VR_Therm_Design_Current_Status   : 1;
        uint64_t Other_Status                     : 1;
        uint64_t Reserved2                        : 1;
        uint64_t Pack_PlatLevel_Pow_Lim_PL1_Stat  : 1;
        uint64_t Pack_PlatLevel_Pow_Lim_PL2_Stat  : 1;
        uint64_t Max_Turbo_Limit_Status           : 1;
        uint64_t Turbo_Transition_Atten_Status    : 1;
        uint64_t Reserved3                        : 2;

        uint64_t PROCHOT_Log                      : 1;
        uint64_t Thermal_Log                      : 1;
        uint64_t Reserved4                        : 2;
        uint64_t Residency_State_Regu_Log         : 1;
        uint64_t Running_Avg_Therm_Limit_Log      : 1;
        uint64_t VR_Therm_Alert_Log               : 1;
        uint64_t VR_Therm_Design_Current_Log      : 1;
        uint64_t Other_Log                        : 1;
        uint64_t Reserved5                        : 1;
        uint64_t Pack_PlatLevel_Pow_Lim_PL1_Log   : 1;
        uint64_t Pack_PlatLevel_Pow_Lim_PL2_Log   : 1;
        uint64_t Max_Turbo_Limit_Log              : 1;
        uint64_t Turbo_Transition_Atten_Log       : 1;
        uint64_t Reserved6                        : 2;

    };
};
#define MSR_CORE_PERF_LIMIT_REASONS_ADDR 0x64f


// Address: 611H Package Energy Status
union alignas (8) MSR_PKG_ENERGY_STATUS  {
    uint64_t value;
    struct {
        uint64_t Total_Energy_Consumed             : 32;
        uint64_t Reserved                          : 32;
    };
};
#define MSR_PKG_ENERGY_STATUS_ADDR 0x611


// Address: 614H Package Power info
union alignas (8) MSR_PKG_POWER_INFO  {
    uint64_t value;
    struct {
        uint64_t Thermal_Spec_Power                : 15;
        uint64_t Reserved1                         : 1;
        uint64_t Minimum_Power                     : 15;
        uint64_t Reserved2                         : 1;
        uint64_t Maximum_Power                     : 15;
        uint64_t Reserved3                         : 1;
        uint64_t Maximum_time_Window               : 6;
        uint64_t Reserved4                         : 10;
    };
};
#define MSR_PKG_POWER_INFO_ADDR 0x614

// Address: 198H Performance Status
union alignas (8) IA32_PERF_STATUS  {
    uint64_t value;
    struct {
        uint64_t Current_Performance_State_Value   : 8;
        uint64_t CurrentFID                        : 8;
        uint64_t Reserved1                         : 16;
        // P-State core voltage can be computed by
        // this * (float) 1/(2^13)
        uint64_t Core_Voltage_ID                   : 16;
        uint64_t Reserved2                         : 16;
    };
};
#define IA32_PERF_STATUS_ADDR 0x198


// Address: 606H RAPL Power Units
union alignas (8) MSR_RAPL_POWER_UNIT  {
    uint64_t value;
    struct {
        /* Power related information (in Watts) is based on the multiplier,
         * 1/ 2^PU; where PU is an unsigned integer represented by bits 3:0.
         * Default value is 0011b, indicating power unit is in 1/8 Watts increment.
         */
        uint64_t Power_Units: 4;
        uint64_t Reserved1: 4;
        /*
        *(bits 12:8): Energy related information (in Joules) is based on the multiplier, 1/2^ESU;
where ESU is an unsigned integer represented by bits 12:8. Default value is 10000b, indicating energy status
unit is in 15.3 micro-Joules increment.
         */
        uint64_t Energy_Status_Units: 5;
        uint64_t Reserved2: 3;
        /*
        *(bits 19:16): Time related information (in Seconds) is based on the multiplier, 1/ 2^TU; where TU
is an unsigned integer represented by bits 19:16. Default value is 1010b, indicating time unit is in 976 micro-
seconds increment.
         **/
        uint64_t Time_Units                        : 4;
        uint64_t Reserved3                         : 44;
    };
};
#define MSR_RAPL_POWER_UNIT_ADDR 0x606











// HELPERS
static inline uint32_t s_lastTotalConsumed = 0;
static double getPkgPowerW(
    MSR_RAPL_POWER_UNIT pUnits,
    MSR_PKG_ENERGY_STATUS eStats,
    double deltaTime)
{
    const double energy_unit = std::pow(0.5, static_cast<double>(pUnits.Energy_Status_Units));
    const uint32_t delta_raw = (eStats.Total_Energy_Consumed - s_lastTotalConsumed) & 0xFFFFFFFF;
    const double delta_joules = delta_raw * energy_unit;
    const double power = delta_joules / deltaTime;
    s_lastTotalConsumed = eStats.Total_Energy_Consumed;
    return power;
}

static float readVID(const IA32_PERF_STATUS msr) {
    constexpr float factor = 0.0001220703125f;
    return static_cast<float>(msr.Core_Voltage_ID) * factor;
}

static float getCurPkgTemp(const MSR_TEMPERATURE_TARGET tempTarget, const IA32_PACKAGE_THERM_STATUS pkgStats) {
    return tempTarget.Temperature_Target - pkgStats.Digital_Readout;
}

static bool anyUsesPkgControl(const std::vector<IA32_HWP_REQUEST>& hwps) {
    for (const auto hwp : hwps) {
        if (hwp.Package_Control)
            return true;
    }
    return false;
}


#pragma pack(pop)
#endif //INTELTYPES_H
