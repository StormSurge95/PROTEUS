#pragma once

#include "../BackendPCH.h"

template<typename T1, typename T2>
class IDevice {
    public:
        ~IDevice() = default;

        virtual T1 read(T2, bool = false) = 0;
        virtual void write(T2, T1) = 0;
};

template<typename T>
class IDeviceIO {
    public:
        virtual ~IDeviceIO() = default;

        virtual T onRead() = 0;
        virtual void onWrite(T) = 0;
};