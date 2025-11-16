//
// Created by pierre on 2025-06-01.
//

#ifndef MSR_0X150_H
#define MSR_0X150_H

#include <cstdint>
#include <type_traits>

#define OC_MAILBOX_ADDR 0x150

#define CORE_READ_CMD       0x8000001000000000
#define CORE_WRITE_CMD      0x8000001100000000

#define IGPU_READ_CMD       0x8000011000000000
#define IGPU_WRITE_CMD      0x8000011100000000

#define CACHE_READ_CMD      0x8000021000000000
#define CACHE_WRITE_CMD     0x8000021100000000

#define SA_READ_CMD         0x8000031000000000
#define SA_WRITE_CMD        0x8000031100000000

#define ANALOG_READ_CMD     0x8000041000000000
#define ANALOG_WRITE_CMD    0x8000041100000000

#define DIGITAL_READ_CMD    0x8000051000000000
#define DIGITAL_WRITE_CMD   0x8000051100000000

#define E_CACHE_READ_CMD    0x8000121000000000
#define E_CACHE_WRITE_CMD   0x8000121100000000

#define CORE_VF_READ_CMD    0x8000001200000000
#define CORE_VF_WRITE_CMD   0x8000001300000000

#define VF_READ_CMD         0x12
#define VF_WRITE_CMD        0x13

#define DOMAIN_CORE         0
#define DOMAIN_IGPU         1
#define DOMAIN_CACHE        2
#define DOMAIN_SA           3
#define DOMAIN_ANALOG       4
#define DOMAIN_DIGITAL      5
#define DOMAIN_E_CACHE      12

namespace OcMailbox {
    union alignas (8) OC_MAILBOX_MSR {
        uint64_t value;
        struct {
            uint64_t Reserved1          : 21;
            uint64_t Offset             : 11;
            uint64_t Cmd                : 8;
            uint64_t Domain             : 4;
            uint64_t Reserved3          : 19;
            uint64_t RunBusy_Write      : 1;
        };
        struct {
            uint64_t PointFrequency     : 8;
            uint64_t Reserved1          : 13;
            uint64_t Offset             : 11;
            uint64_t Cmd                : 8;
            uint64_t Domain             : 4;
            uint64_t Reserved2          : 19;
            uint64_t RunBusy_Write      : 1;
        } VF;
    };


    double convertOffsetFromRaw(uint64_t value);
    double getVidOffset(const OC_MAILBOX_MSR& msr);
    double convertToVidOffset(uint64_t fromOffset);
    uint64_t buildVidOffsetMsrValue(uint64_t domainCommand, double offset);
    double readCoreVidOffset();
    double readIGpuVidOffset();
    double readCacheVidOffset();
    double readAnalogVidOffset();
    double readDigitalVidOffset();
    double readECacheVidOffset();
    double readSAVidOffset();
    double readVidOffsetRaw(uint64_t byWriteCmd);

    uint64_t readVFOffsetRaw(uint16_t point, uint16_t domain = 0);
    void writeVFOffsetRaw(uint16_t point, double offset, uint16_t domain = 0);

    union OC_MAILBOX_CTRL {
        uint32_t value;
        struct {
            uint32_t Cmd        : 8;
            uint32_t Domain     : 8;
            uint32_t Reserved0  : 15;
            uint32_t RunBusy    : 1;
        };
    };

    union OC_MAILBOX_DATA {
        uint32_t value;
        struct {
            int16_t  VOffset_raw;
            uint16_t Ratio;
        };
    };

    union OC_MAILBOX_VF {
        uint64_t value = 0;
        struct {
            OC_MAILBOX_DATA data;
            OC_MAILBOX_CTRL ctrl;
        };
    };

}


#endif //MSR_0X150_H
