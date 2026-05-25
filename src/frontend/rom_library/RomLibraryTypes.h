#pragma once

#include "../FrontendPCH.h"

namespace NS_Proteus {
    struct ROM_DATA {
        string gameName;
        string path;
        string hash;
        size_t fileSize;
        u64 lastWrite;
    };
}