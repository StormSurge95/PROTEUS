#pragma once

#include "../BackendPCH.h"

/**
 * @interface IDevice IDevice.h "backend/SHARED/IDevice.h"
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
};

/**
 * @interface IDeviceIO IDevice.h "backend/SHARED/IDevice.h"
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