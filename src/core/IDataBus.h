#pragma once

template<typename T1, typename T2>
class IDataBus {
    public:
        IDataBus() = default;
        ~IDataBus() = default;

        virtual T1 read(T2 addr, bool readonly = false) = 0;
        virtual void write(T2 addr, T1 data) = 0;
};