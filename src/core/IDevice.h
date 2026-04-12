#pragma once

#include <memory>


template<typename T1, typename T2>
class IDevice {
    public:
        ~IDevice() = default;

        virtual T1 read(T2 addr, bool readonly = false) = 0;
        virtual void write(T2 addr, T1 data) = 0;
};

template<typename T>
class IODevice {
    public:
        virtual ~IODevice() = default;

        virtual T onRead() = 0;
        virtual void onWrite(T data) = 0;
};