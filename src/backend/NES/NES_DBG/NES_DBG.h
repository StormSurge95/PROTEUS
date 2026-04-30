#pragma once

#include "../../../core/IDebugger.h"
#include "../NES_CORE.h"

class NES_DBG : public IDebugger {
    private:
        bool enabled = false;
        std::shared_ptr<NES_BUS> bus = nullptr;
        std::shared_ptr<NES_CPU> cpu = nullptr;
        std::shared_ptr<NES_PPU> ppu = nullptr;
        std::vector<uint16_t> instAddrs;

        uint32_t debugPalette[4] = { 0xFF000000, 0xFF323232, 0xFF646464, 0xFFFFFFFF };
    public:
        NES_DBG() = default;
        NES_DBG(std::shared_ptr<NES_CORE>);
        ~NES_DBG() { Clear(); }

        void Toggle(bool enable) override { enabled = enabled; }
        void StepInstruction() override;
        void StepCycle() override;
        bool IsEnabled() const override { return enabled; }
        void Clear();

        std::string GetStateCPU() override;
        std::string GetDisassembly() override;
        std::string GetStateRAM() override;

        void ScanInstructions(uint16_t, bool) override;

        std::vector<uint32_t> GetPaletteColors() override;
        std::vector<uint32_t> GetPatternTable(int) override;

        // TODO: figure out APU debug stuff
};