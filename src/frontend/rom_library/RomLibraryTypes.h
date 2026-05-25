#pragma once

#include "../FrontendPCH.hpp"

namespace NS_Proteus {
    struct ROM_DATA {
        string gameName;
        string path;
        string hash;
        size_t fileSize;
        u64 lastWrite;
    };
}