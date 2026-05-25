```markdown
# GBA-First Multi-System Emulator Plan (Ogb/Cgb/Gba Runtime Selection)

## Summary
Build a **GBA-branded emulator** in C++ that loads a ROM, detects target platform, and boots the correct machine model:
- `Ogb` (Original Game Boy / DMG-class)
- `Cgb` (Game Boy Color)
- `Gba` (Game Boy Advance)

The executable is unified, but hardware is model-specific behind shared interfaces (`iCPU`, `iPPU`, `iAPU`, etc.).  
Default identity is `Gba`, but runtime selection is automatic unless user override is set.

---

## Goals
- Accurate emulation across all Nintendo Game Boy generations (DMG/CGB/GBA era).
- One shared frontend/runtime with pluggable machine backends.
- Deterministic testing and regression safety from the start.
- High compatibility with major test ROMs and commercial games.

---

## Non-Goals (Initial)
- Netplay, shader pipelines, and advanced UX polish before core accuracy.
- Mid-session machine hot-switching.
- Mixing components across machine models.

---

## Hardware Profiles (Baseline Constraints)

| Feature | Game Boy (`Ogb`) | Game Boy Color (`Cgb`) | Game Boy Advance (`Gba`) |
|---|---|---|---|
| CPU | 8-bit SM83 @ 4.19 MHz | 8-bit (dual-speed) @ 4.19/8 MHz | 32-bit ARM7TDMI @ 16.78 MHz |
| Screen | Monochrome, 160x144 | Color, 160x144 | Color, 240x160 |
| VRAM | 8 KB | 16 KB | 128 KB |
| WRAM | 8 KB | 32 KB | 256 KB |

Use these as model profile assertions and config defaults.

---

## Architecture

### Core Interfaces
- `iMachine`
- `iCPU` (`OgbCPU`, `CgbCPU`, `GbaCPU`)
- `iPPU` (`OgbPPU`, `CgbPPU`, `GbaPPU`)
- `iAPU` (`OgbAPU`, `CgbAPU`, `GbaAPU`)
- `iMemoryBus`
- `iCartridgeController`
- `iDMA`, `iTimers`, `iInterruptController`, `iIO`

### Runtime Pipeline
1. `RomInspector` parses ROM header/metadata.
2. `MachineSelector` resolves target model (`Auto|ForceOgb|ForceCgb|ForceGba`).
3. `MachineFactory` builds concrete machine stack.
4. Shared scheduler drives frame/audio/input/debug loop.

### Invariants
- No cross-model component mixing.
- Model changes only at reset/boot boundaries.
- Save states must include `model_id + rom_hash + schema_version`.

---

## Milestones

## Phase 0: Foundation
- [ ] Create repo/module layout for interface-driven architecture.
- [ ] Implement shared scheduler/event loop contract.
- [ ] Implement deterministic headless runner and logging.
- [ ] Add CI (Windows/Linux/macOS) with baseline test jobs.

## Phase 1: Bootstrapping the GBA-First Framework
- [ ] Implement `RomInspector` and `MachineSelector`.
- [ ] Implement `MachineFactory` and model registry.
- [ ] Stand up `GbaMachine` skeleton (stub CPU/PPU/APU/Bus/IO).
- [ ] Wire shared frontend (video/audio/input) to abstract interfaces.
- [ ] Add session metadata (selected model, override mode, ROM hash).

## Phase 2: Ogb Machine to Playable Accuracy
- [ ] Complete `OgbCPU` (instruction correctness + timing model).
- [ ] Complete Ogb memory map, interrupts, timers, joypad, serial basics.
- [ ] Complete `OgbPPU` (modes, LCDC/STAT behavior, sprites/window/bg).
- [ ] Complete `OgbAPU` baseline channels/mixing.
- [ ] Implement key cartridge types: `ROM_ONLY`, `MBC1`, `MBC2`, `MBC3`, `MBC5`.
- [ ] Validate with Ogb-focused test ROM suite + sample commercial titles.

## Phase 3: Cgb Extensions
- [ ] Add double-speed mode and Cgb-specific registers.
- [ ] Add VRAM/WRAM banking and palette/attribute behavior.
- [ ] Add HDMA/GDMA behavior.
- [ ] Add Cgb boot and Ogb-compat nuance handling.
- [ ] Expand mapper/RTC edge-case handling.
- [ ] Validate with Cgb test ROMs and Ogb-on-Cgb compatibility checks.

## Phase 4: Full Gba Implementation
- [ ] Implement `GbaCPU` ARM+Thumb execution, exceptions, IRQ behavior.
- [ ] Implement Gba memory regions, waitstates, and bus timing details.
- [ ] Implement `GbaPPU` (modes 0-5, affine, windows, blending, OBJ rules).
- [ ] Implement DMA channels/triggers and timers.
- [ ] Implement `GbaAPU` (PSG + FIFO DMA audio path).
- [ ] Implement save types: SRAM, EEPROM, Flash variants (+ RTC where required).
- [ ] Validate with Gba test ROMs and broad game matrix.

## Phase 5: Cross-Model Hardening
- [ ] Improve detection heuristics and fallback behavior.
- [ ] Add model-safe save-state loading rules and migration checks.
- [ ] Add trace-diff tooling for timing/order regressions.
- [ ] Run compatibility sweeps and triage per-title issues.
- [ ] Close high-impact timing/IRQ/DMA/PPU race defects.

## Phase 6: Tooling, Performance, Release Readiness
- [ ] Add debugger features (breakpoints, disassembly, memory/VRAM/OAM viewers).
- [ ] Profile/optimize hotspots without violating timing correctness.
- [ ] Add user-facing features: fast-forward, rewind (optional), controller remap.
- [ ] Stabilize packaging and release profile presets (accuracy/performance).

---

## Testing Strategy
- [ ] Unit tests per subsystem (CPU ops, timers, bus reads/writes, IRQ edges).
- [ ] Integration tests (frame hash/audio hash/trace hash).
- [ ] Compatibility matrix by model (`Ogb`, `Cgb`, `Gba`) with pass/fail history.
- [ ] Regression gate in CI: no merge on deterministic output drift unless approved.

---

## Risks & Mitigations
- **Timing accuracy complexity**  
  Mitigation: event scheduler invariants + trace comparison.
- **Audio correctness across models**  
  Mitigation: staged fidelity and reference capture tests.
- **Mapper/save edge cases**  
  Mitigation: cartridge DB + explicit override + telemetry/logging.
- **Scope creep in frontend**  
  Mitigation: keep emulator-core milestones as release gates.

---

## Acceptance Criteria (Project-Level)
- [ ] ROM auto-detection reliably chooses `Ogb/Cgb/Gba` for supported carts.
- [ ] Stable gameplay across representative titles in each model class.
- [ ] Major public test ROM suites substantially pass for each model.
- [ ] Save-state and battery-save behavior is deterministic and model-safe.
- [ ] CI regression suite protects against behavior drift.

---

## Nice-to-Have Follow-Ups
- [ ] Optional JIT experiments for Gba CPU (behind strict correctness checks).
- [ ] Achievement/cheat integration layer.
- [ ] Platform-native frontends beyond SDL baseline.
```