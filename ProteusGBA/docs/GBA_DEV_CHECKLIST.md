# ProteusGBA Checklist (GBA-first multi-model core as Proteus plugin)

## Objective
Deliver `ProteusGBA` as a Windows-first plugin that fully conforms to Proteus’s existing runtime contracts:
- `IConsole` + plugin exports/manifest
- `ConsoleSession` lifecycle
- `VideoManager` framebuffer pull
- `AudioManager` sample pull
- `InputManager` button-state push

Internal architecture remains multi-model (`Ogb/Cgb/Gba`) with runtime ROM detection.

## Current Host Contract (must satisfy)
- [ ] Implement all required `IConsole` methods:
  - [ ] `loadROM(const string&)`
  - [ ] `reset()`
  - [ ] `clock()`
  - [ ] `getFrameBuffer() const`
  - [ ] `SCREEN_WIDTH() const`
  - [ ] `SCREEN_HEIGHT() const`
  - [ ] `collectAudio(vector<float>&)`
  - [ ] `buttonCount() const`
  - [ ] `update(u8, const bool*)`
  - [ ] `initSST(...)`, `runSST()`, `checkSST(...)`
- [ ] Export required plugin symbols via `PluginExports.h` macros:
  - [ ] `CreateCore`
  - [ ] `DestroyCore`
  - [ ] `GetPluginManifest`
- [ ] Export debugger symbols (or explicitly provide safe null behavior consistent with host expectations):
  - [ ] `CreateDebugger`
  - [ ] `DestroyDebugger`
- [ ] Match contract versions:
  - [ ] `ICONSOLE_CONTRACT_VERSION`
  - [ ] `IDEBUGGER_CONTRACT_VERSION` (if debugger provided)

---

## Phase 0: Plugin Skeleton + Discovery
- [ ] Build `ProteusGBA` as a loadable plugin artifact (`.dll` on Windows).
- [ ] Ensure plugin naming/pathing matches `PluginRegistry` expectations (`gba` -> `ProteusGBA` base name).
- [ ] Add `ConsoleID::GBA` mapping in host `ConsolePluginMap` if not already present.
- [ ] Verify `PluginManager::IsConsoleAvailable(ConsoleID::GBA)` succeeds.

## Phase 1: Internal Core Architecture (behind IConsole)
- [ ] Implement internal machine abstraction:
  - [ ] `IMachine`
  - [ ] `OgbMachine`, `CgbMachine`, `GbaMachine`
- [ ] Implement internal component interfaces:
  - [ ] `iCPU`, `iPPU`, `iAPU`, `iMemoryBus`, timing/scheduler primitives
- [ ] Add `RomInspector + MachineSelector + MachineFactory`:
  - [ ] auto-detect `Ogb/Cgb/Gba`
  - [ ] support optional forced mode (config/debug path)

## Phase 2: Host Dataflow Compliance
- [ ] `clock()` advances emulation safely under Proteus per-frame loop.
- [ ] `getFrameBuffer()` returns stable non-owning pointer valid until next `clock()`.
- [ ] `collectAudio(...)` provides host-consumable float samples in expected cadence.
- [ ] `buttonCount()/update(...)` align with `InputManager::TranslateInputs(...)` behavior.
- [ ] `SCREEN_WIDTH()/SCREEN_HEIGHT()` reflect active machine mode and support texture init flow.

## Phase 3: Ogb Completion (first full playable path)
- [ ] Complete Ogb CPU/PPU/APU/timers/interrupts/memory behavior.
- [ ] Implement required Ogb mappers (`ROM_ONLY`, `MBC1`, `MBC2`, `MBC3`, `MBC5`).
- [ ] Validate ROM load/start/reset/shutdown through real `ConsoleSession` flow.
- [ ] Confirm stable rendering/audio/input in Proteus `GAME_VIEW`.

## Phase 4: Cgb Completion
- [ ] Implement Cgb speed-switch, VRAM/WRAM banking, palettes, DMA variants.
- [ ] Validate Cgb and Ogb-on-Cgb behavior in same plugin runtime.
- [ ] Ensure mode transitions occur only on ROM load/reset boundaries.

## Phase 5: Gba Completion
- [ ] Implement ARM7TDMI ARM/Thumb execution and IRQ behavior.
- [ ] Implement Gba PPU modes, DMA, timers, memory timing essentials.
- [ ] Implement Gba audio path (PSG + FIFO/DMA behavior needed for compatibility).
- [ ] Implement save-type handling required by commercial carts.

## Phase 6: Debugger + Test Contract
- [ ] Provide `IDebugger` implementation compatible with Proteus debug toggles and stepping.
- [ ] Ensure `StepInstruction`/`StepCycle` work when session is paused (F7 path).
- [ ] Implement SST hooks to support existing test harness entry points.
- [ ] Add plugin-level tests for:
  - [ ] manifest validity
  - [ ] symbol export validity
  - [ ] lifecycle (`CreateCore -> loadROM -> clock -> DestroyCore`)

## Phase 7: Windows-First Release Gate
- [ ] Build/release with MSVC + CMake in Visual Studio workflow.
- [ ] Verify plugin discovery/load in packaged Windows Proteus build output.
- [ ] Pass deterministic regression suite on Windows CI.
- [ ] Meet compatibility threshold targets for Ogb/Cgb/Gba milestone ROM sets.

## Phase 8: Multi-Platform Follow-through
- [ ] Keep plugin ABI and core code portable.
- [ ] Add Linux/macOS plugin builds once Windows path is stable.
- [ ] Validate cross-platform behavior parity (timing-sensitive regressions tracked).

---

## Hard Guardrails
- [ ] Do not break `IConsole` contract semantics used by `ConsoleSession`.
- [ ] Keep framebuffer/audio ownership rules exactly as host expects.
- [ ] Keep emulator core decoupled from SDL/ImGui/frontend concerns.
- [ ] No mid-session machine swap; resolve model at ROM load/reset only.

## Acceptance Criteria
- [ ] User can select GBA in Proteus UI, launch ROM, and run via existing session loop without host-side emulator changes.
- [ ] Plugin auto-selects Ogb/Cgb/Gba execution model correctly for supported ROMs.
- [ ] Reset/pause/resume/debug paths function through existing `ConsoleSession` and input bindings.
- [ ] Windows release artifact is discoverable, loadable, and stable in Proteus.