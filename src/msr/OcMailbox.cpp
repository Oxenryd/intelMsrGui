//
// Created by pierre on 2025-06-06.
//
#include <cmath>
#include "OcMailbox.h"

#include <algorithm>

#include "Msr.hpp"

double OcMailbox::convertToVidOffset(uint64_t fromOffset) {
    auto raw12 = static_cast<int32_t>(fromOffset);
    if (fromOffset & 0x400)
        raw12 |= ~0x7ff;
    const auto final = static_cast<double>(raw12) / 1.024;
    return final;
}


double OcMailbox::convertOffsetFromRaw(const uint64_t value) {
    auto raw12 = static_cast<int32_t>(value);
    if (value & 0x400)
        raw12 |= ~0x7ff;
    const auto final = static_cast<double>(raw12) / 1.024;
    return final;
}

double OcMailbox::getVidOffset(const OC_MAILBOX_MSR& msr) {
    MSR::write_msr(0, OC_MAILBOX_ADDR, &msr.value);
    const auto result = MSR::readAndReturn<OC_MAILBOX_MSR>(0, OC_MAILBOX_ADDR );
    auto raw12 = static_cast<int32_t>(result.Offset);
    if (result.Offset & 0x400)
        raw12 |= ~0x7ff;
    const auto final = static_cast<double>(raw12) / 1.024;
    return final;
}



uint64_t OcMailbox::buildVidOffsetMsrValue(uint64_t domainCommand, double offset)
{
    auto raw = static_cast<int32_t>(std::lround(offset * 1.024));

    constexpr int32_t kMin = -500;
    constexpr int32_t kMax =  500;
    raw = std::clamp(raw, kMin, kMax);

    const uint32_t encoded = static_cast<uint32_t>(raw) & 0x7FF;
    const uint32_t shifted = encoded << 21;

    const uint64_t val = domainCommand | shifted;

    return val ;
}
double OcMailbox::readCoreVidOffset() {
    OC_MAILBOX_MSR command{};
    command.value = CORE_READ_CMD;
    return getVidOffset(command);
}
double OcMailbox::readIGpuVidOffset() {
    OC_MAILBOX_MSR command{};
    command.value = IGPU_READ_CMD;
    return getVidOffset(command);
}
double OcMailbox::readCacheVidOffset() {
    OC_MAILBOX_MSR command{};
    command.value = CACHE_READ_CMD;
    return getVidOffset(command);
}
double OcMailbox::readAnalogVidOffset() {
    OC_MAILBOX_MSR command{};
    command.value = ANALOG_READ_CMD;
    return getVidOffset(command);
}
double OcMailbox::readDigitalVidOffset() {
    OC_MAILBOX_MSR command{};
    command.value = DIGITAL_READ_CMD;
    return getVidOffset(command);
}
double OcMailbox::readECacheVidOffset() {
    OC_MAILBOX_MSR command{};
    command.value = E_CACHE_READ_CMD;
    return getVidOffset(command);
}
double OcMailbox::readSAVidOffset() {
    OC_MAILBOX_MSR command{};
    command.value = SA_READ_CMD;
    return getVidOffset(command);
}

double OcMailbox::readVidOffsetRaw(uint64_t byWriteCmd) {

    switch (byWriteCmd) {
        case CORE_WRITE_CMD: return readCoreVidOffset();
        case IGPU_WRITE_CMD: return readIGpuVidOffset();
        case CACHE_WRITE_CMD: return readCacheVidOffset();
        case SA_WRITE_CMD: return readSAVidOffset();
        case ANALOG_WRITE_CMD: return readAnalogVidOffset();
        case DIGITAL_WRITE_CMD: return readDigitalVidOffset();
        case E_CACHE_WRITE_CMD: return readECacheVidOffset();

        default: throw std::runtime_error("Unknown VID Offset Write CMD");

    }
}

uint64_t OcMailbox::readVFOffsetRaw(const uint16_t point, const uint16_t domain) {

    uint64_t value = 0x8000001000000000;
    value |= static_cast<uint64_t>(domain) << 40;
    value |= static_cast<uint64_t>(point) << 48;

    MSR::write_msr(0, OC_MAILBOX_ADDR, &value);
    OC_MAILBOX_MSR result{};
    result.RunBusy_Write = 1;
    while (result.RunBusy_Write == 1)
        result = MSR::readAndReturn<OC_MAILBOX_MSR>(0, OC_MAILBOX_ADDR );

    return result.value;

}

void OcMailbox::writeVFOffsetRaw(const uint16_t point, const double offset, const uint16_t domain) {

    uint64_t cmd = 0x8000001100000000;
    cmd |= static_cast<uint64_t>(domain) << 40;
    cmd |= static_cast<uint64_t>(point) << 48;

    const auto raw = static_cast<int16_t>(std::lround(offset * 1.024));
    constexpr int16_t kMin = -500;
    constexpr int16_t kMax =  150;
    const auto clamped = std::clamp(raw, kMin, kMax);

    auto value = buildVidOffsetMsrValue(cmd, clamped);
    MSR::write_msr(0, OC_MAILBOX_ADDR, &value);
}
