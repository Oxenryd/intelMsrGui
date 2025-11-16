//
// Created by pierre on 2025-06-07.
//

#ifndef IMT_ERRCODE_H
#define IMT_ERRCODE_H
#include <cstdint>

enum class IMT_ErrCode : uint8_t {
    OK = 0,
    OK_ShouldExit,
    ArgumentError,
    ValueError,
    ValueWarning,
    ArgumentCount,
    WriteError,
    ReadError,
    PresetNameExists,
    NotFound,
    MsrWriteError,
    MsrReadError,
    MsrVoltageNotAppliedCorrectly,
    NoCpusFound,
    ParsingError,
    UnknownArgument,
    HWP_NotAvailable,
    NoPresetsFound
};

constexpr auto IMT_OK = static_cast<IMT_ErrCode>(0);
#define IMT_SUCCEEDED(ec) ((ec) == IMT_OK)
#define IMT_FAILED(ec)    ((ec) != IMT_OK)
#define IMT_CHECK_AND_RETURN(ecVar, expr) (ecVar) = (expr); if (IMT_FAILED(ecVar)) return (ecVar);
#define IMT_CHECK_AND_RETURN_INT(ecVar, expr) (ecVar) = (expr); if (IMT_FAILED(ecVar)) return static_cast<int>((ecVar));

#endif //IMT_ERRCODE_H
