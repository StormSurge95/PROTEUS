#pragma once

#include "./NesMapper.h"

namespace NS_NES {
    class M007 : public Mapper {
        public:
            M007(u16 pBnk, vector<u8>* pMem, u16 cBnk, vector<u8>* cMem, vector<u8>*pRam = nullptr, u8 id2 = 0) :
                Mapper(pBnk, pMem, cBnk, cMem, pRam, id2) {
                prg32kBanks = static_cast<u16>(pMem->size() / 0x8000);
                if (prg32kBanks == 0) prg32kBanks = 1;
            }

            void powerup() override {
                prgBank = 0;
                mirrorMode = MIRROR::ONE_SCREEN_LO;
                emulateBusConflicts = subMapperID == 2;
            }

            void reset() override { powerup(); }
            void powerdown() override { powerup(); }

            bool cpuRead(u16 addr, u8& data, bool readonly = false) override {
                if (addr < 0x8000) return false;

                u32 mapped = (addr & 0x7FFF) + (u32(prgBank) << 15);
                data = prgRom->at(mapped);

                if (!readonly && eventSink) eventSink->OnMapperRegisterRead(string("PRG-ROM: ") + hex(mapped, 4), addr, data);
                return true;
            }

            void cpuWrite(u16 addr, u8 data) override {
                if (addr < 0x8000) return;

                u8 tmp;
                u8 latched = emulateBusConflicts ? (data & cpuRead(addr, tmp, true)) : data;
                prgBank = normalizeBank(latched & 0x07);
                mirrorMode = (latched & 0x10) ? MIRROR::ONE_SCREEN_HI : MIRROR::ONE_SCREEN_LO;

                if (eventSink) eventSink->OnMapperRegisterWrite("AxROM - bank/mirror select", addr, latched);
            }

            bool ppuRead(u16 addr, u8& data, bool readonly = false) override {
                if (addr >= 0x2000) return false;

                u16 mapped = addr & 0x1FFF;
                data = chrMem->at(mapped);
                if (!readonly && eventSink) eventSink->OnMapperRegisterRead(string("CHR-") + (hasChrRam ? "RAM: " : "ROM: ") + hex(mapped, 4), addr, data);
                return true;
            }

            void ppuWrite(u16 addr, u8 data) override {
                if (addr >= 0x2000 || !hasChrRam) return;

                u16 mapped = addr & 0x1FFF;
                chrMem->at(mapped) = data;
                if (eventSink) eventSink->OnMapperRegisterWrite(string("CHR-RAM: ") + hex(mapped, 4), addr, data);
            }

            MIRROR getMirrorMode() const override { return mirrorMode; }

            vector<array<string, 2>> getDebugData() override {
                return {
                    { "Mapper ID", "7 (AxROM)" },
                    { "Submapper ID", to_string(subMapperID) },
                    { "PRG 32KB Banks", to_string(prg32kBanks) },
                    { "Selected PRG Bank", to_string(prgBank) },
                    { "Nametable Mode", mirrorMode == MIRROR::ONE_SCREEN_HI ? "One-screen high" : "One-screen low" },
                    { "Bus Conflicts", emulateBusConflicts ? "true" : "false" }
                };
            }
        private:
            u8 prgBank = 0;
            u16 prg32kBanks = 1;
            MIRROR mirrorMode = MIRROR::ONE_SCREEN_LO;
            bool emulateBusConflicts = false;

            u8 normalizeBank(u8 bank) const {
                if ((prg32kBanks & (prg32kBanks - 1)) == 0) return bank & (prg32kBanks - 1);
                return bank % prg32kBanks;
            }
    };
}