#pragma once

#include "../FrontendPCH.h"

namespace NS_Proteus {
    static const SDL_GPUShaderFormat ShaderFlags =
        SDL_GPU_SHADERFORMAT_PRIVATE |
        SDL_GPU_SHADERFORMAT_SPIRV |
        SDL_GPU_SHADERFORMAT_DXBC |
        SDL_GPU_SHADERFORMAT_DXIL |
        SDL_GPU_SHADERFORMAT_MSL |
        SDL_GPU_SHADERFORMAT_METALLIB;

    static const SDL_WindowFlags WindowFlags =
        SDL_WINDOW_RESIZABLE |
        SDL_WINDOW_HIDDEN |
        SDL_WINDOW_HIGH_PIXEL_DENSITY;

    struct DisplayInfo {
        float scale = 2.0f;
        u32 screenWidth = 3840;
        u32 screenHeight = 2160;
        u32 dispWidth = 2560;
        u32 dispHeight = 1440;
        u32 gameWidth = 0;
        u32 gameHeight = 0;

        float PopupX() const { return dispWidth * 0.5f; }
        float PopupY() const { return dispHeight * 0.5f; }
        float PopupW() const { return dispWidth * 0.2f; }
        float PopupH() const { return dispHeight * 0.2f; }
    };

    typedef map<ConsoleID, u16> PageCounts;

    enum class DebugView {
        NONE = -1,
        CPU_REGS,
        CPU_DISASM,
        CPU_MEMORY,
        PPU_REGS,
        PPU_PATTERNTABLES,
        PPU_NAMETABLES,
        PPU_SPRITES,
        APU_REGISTERS,
        APU_CHANNELS,
        PAK_HEADER,
        TOTAL_VIEWS
    };

    enum class MenuType {
        MAIN, OVERLAY, DEBUG
    };

    struct MenuSelection {
        int row = 0;
        int col = 0;
        int maxRow = 2;
        int maxCol = 3;

        MenuSelection() = default;
        void Update(u8 r, u8 c) { maxRow = r; maxCol = c; }
        void RowDown() { row = (row == maxRow) ? 0 : row + 1; }
        void RowUp() { row = (row == 0) ? 2 : row - 1; }
        void ColLeft() { col = (col == 0) ? 3 : col - 1; }
        void ColRight() { col = (col == maxCol) ? 0 : col + 1; }
    };
}