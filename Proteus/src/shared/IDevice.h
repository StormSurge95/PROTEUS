#pragma once

#include "./SharedPCH.h"
#include "./Types.h"

/**
 * @interface IDevice IDevice.h
 * @brief Interface class for the various devices that are capable of receiving AND submitting read/write requests.
 * @note Unlike `IDeviceIO`, this interface allows for the possibility of writing to multiple addresses on a single device.
 * @tparam DataSize The data type used to represent the size of the data being read/written
 * @tparam AddressSize The data type used to represent the size of the addresses being read from/written to.
 */
template<typename DataSize, typename AddressSize>
class IDevice {
    public:
        /// @brief default virtual destructor
        ~IDevice() = default;
        /// @brief required for the data read operations
        virtual DataSize read(AddressSize, bool = false) = 0;
        /// @brief required for the data write operations
        virtual void write(AddressSize, DataSize) = 0;

        virtual void powerup(u32) {};
        virtual void reset() {};
        virtual void powerdown() {};
    protected:
        u32 seed = 0;
        u32 bufferedWord = 0;
        u8 bytesAvail = 0;
        void initPRNG(u32 s) {
            seed = s;
            bufferedWord = bytesAvail = 0;
        }
        u32 nextWord() {
            seed ^= seed << 13;
            seed ^= seed >> 17;
            seed ^= seed << 5;
            return seed;
        }
        u8 nextByte() {
            if (bytesAvail == 0) {
                bufferedWord = nextWord();
                bytesAvail = 4;
            }

            u8 out = bufferedWord & 0xFF;
            bufferedWord >>= 8;
            bytesAvail--;
            return out;
        }
};

/**
 * @interface IDeviceIO IDevice.h
 * @brief Interface class for the various devices that are capable only of RECEIVING read/write requests.
 * @note Unlike `IDevice`, this interface is meant for devices that only respond to signals at a singular address.
 * @tparam DataSize The data type used to represent the size of the data being written/read
 */
template<typename DataSize>
class IDeviceIO {
    public:
        /// @brief default virtual destructor
        virtual ~IDeviceIO() = default;
        /// @brief required for data read operations
        virtual DataSize onRead() = 0;
        /// @brief required for data write operations
        virtual void onWrite(DataSize) = 0;
};