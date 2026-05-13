#pragma once

#include "../BackendPCH.h"

template<typename DataSize, typename AddressSize>
class IDevice {
    public:
        ~IDevice() = default;

        virtual DataSize read(AddressSize, bool = false) = 0;
        virtual void write(AddressSize, DataSize) = 0;
};

template<typename DataSize>
class IDeviceIO {
    public:
        virtual ~IDeviceIO() = default;

        virtual DataSize onRead() = 0;
        virtual void onWrite(DataSize) = 0;
};