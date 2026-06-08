# ProteusNES Checklist (commercial-compatibility core as Proteus plugin)

## Objective
Deliver `ProteusNES` as a Windows-first plugin that integrates cleanly with the current Proteus runtime contracts:
- `IConsole` core lifecycle + frame/audio/input API
- plugin discovery/loading via `PluginManager/PluginRegistry`
- optional `IDebugger` integration for paused stepping and debug UI paths
- compatibility target: all commercially released NES cartridges

## Host Contract Compliance (non-negotiable)
- [x] Implement all required `IConsole` methods with host-compatible semantics:
  - [x] `loadROM(const string&)`
  - [x] `reset()`
  - [x] `clock()`
  - [x] `getFrameBuffer() const`
  - [x] `SCREEN_WIDTH() const`
  - [x] `SCREEN_HEIGHT() const`
  - [x] `collectAudio(vector<float>&)`
  - [x] `buttonCount() const`
  - [x] `update(u8, const bool*)`
  - [x] `initSST(...)`, `runSST()`, `checkSST(...)`
- [x] Export required plugin symbols:
  - [x] `CreateCore`
  - [x] `DestroyCore`
  - [x] `GetPluginManifest`
- [x] Export debugger symbols (real or safe null path consistent with host handling):
  - [x] `CreateDebugger`
  - [x] `DestroyDebugger`
- [x] Manifest/version correctness:
  - [x] `iConsoleContractVersion == ICONSOLE_CONTRACT_VERSION`
  - [x] `iDebuggerContractVersion` aligned when debugger is implemented
  - [x] `consoleName` / metadata set for NES plugin identity

---

## Phase 0: Plugin Packaging + Runtime Wiring
- [x] Build `ProteusNES` as plugin DLL compatible with current loader expectations.
- [x] Verify plugin filename/base-name mapping resolves through registry.
- [x] Confirm discovery and creation path in host:
  - [x] `PluginManager::IsConsoleAvailable(ConsoleID::NES)`
  - [x] `ConsoleFactory::Create(ConsoleID::NES)`
- [x] Validate `ConsoleSession` lifecycle:
  - [x] create
  - [x] ROM load
  - [x] start
  - [x] pause/resume
  - [x] reset
  - [x] shutdown

## Phase 1: Core Emulation Baseline (host-visible functional)
- [x] CPU (2A03/6502-derived) execution baseline with required opcodes used by commercial carts.
- [x] PPU baseline rendering and vblank/NMI flow.
- [x] APU baseline channel output and frame sequencing.
- [x] Controller input path aligned with `buttonCount/update`.
- [x] Stable framebuffer pointer behavior for `VideoManager`.
- [x] Stable sample extraction cadence for `AudioManager`.

## Phase 2: Deterministic Runtime + SST/Test Hooks
- [x] Ensure `clock()` behavior is deterministic under Proteus frame loop.
- [x] Complete SST hooks used by existing test flow (`RunSST` path in app).
- [ ] Add plugin-level deterministic checks:
  - [x] repeatable CPU state progression
  - [x] repeatable frame hash
  - [x] repeatable audio hash/window
- [x] Ensure no frontend/UI dependencies inside plugin core.

## Phase 3: High-Impact Mapper Coverage
- [ ] Implement major commercial mappers first and test against various ROMs for behavior:
  - [x] NROM (0)
    - [x] Boot/Load Smoke ---------- Donkey Kong
    - [x] Core Functionality ------- Excitebike
    - [x] Edge Behavior  ----------- Mach Rider
  - [x] MMC1 (1)
    - [x] Boot/Load Smoke ---------- The Legend of Zelda
    - [x] Core Functionality ------- Metroid
    - [x] Edge Behavior ------------ Mega Man 2
  - [x] UxROM (2)
    - [x] Boot/Load Smoke ---------- Mega Man
    - [x] Core Functionality ------- Castlevania
    - [x] Edge Behavior ------------ Contra
  - [x] CNROM (3)
    - [x] Boot/Load Smoke ---------- Arkanoid
    - [x] Core Functionality ------- Gradius
    - [x] Edge Behavior ------------ Adventure Island
  - [ ] MMC3/MMC6 (4)
    - [ ] Boot/Load Smoke ---------- Batman
    - [ ] Core Functionality ------- Batman Returns
    - [ ] Edge Behavior ------------ Alien3
  - [ ] AOROM (7)
    - [ ] Boot/Load Smoke ---------- Marble Madness
    - [ ] Core Functionality ------- Battletoads
    - [ ] Edge Behavior ------------ Solar Jetman
  - [ ] MMC2 (9)
    - [ ] Boot/Load Smoke ---------- Mike Tyson's Punch-Out!!
    - [ ] Core Functionality ------- Punch Out!!
    - [ ] Edge Behavior ------------ use previous
  - [ ] MMC4 (10)
    - [ ] Boot/Load Smoke ---------- Famicom Wars
    - [ ] Core Functionality ------- Fire Emblem Gaiden
    - [ ] Edge Behavior ------------ use previous
- [ ] Implement mirroring, banking, PRG-RAM semantics per mapper.
- [ ] Implement mapper IRQ behavior used by timing-sensitive commercial titles.

## Phase 4: Commercial Long-Tail Mapper Completion
- [ ] Add remaining licensed commercial mapper/submapper coverage.
- [ ] Add board-specific edge behavior (bus conflicts, RAM quirks, protection patterns where needed).
- [ ] Validate each newly added board against targeted title set before merge.

## Phase 5: Timing/Accuracy Hardening
- [ ] PPU edge cases:
  - [ ] sprite 0 hit behavior
  - [ ] sprite overflow behavior
  - [ ] odd/even frame cadence details
- [ ] APU edge cases:
  - [ ] DMC timing/IRQ interaction
  - [ ] frame counter IRQ behavior
- [ ] CPU/bus edge cases:
  - [ ] RMW timing expectations
  - [ ] open-bus behavior where title-critical

## Phase 6: Debugger Integration for Proteus UX
- [ ] Implement `IDebugger` state/query methods used by debug overlay.
- [ ] Ensure paused stepping flow works with F7 handling:
  - [ ] `StepInstruction()`
  - [ ] `StepCycle()`
- [ ] Populate CPU/PPU/APU state views and disassembly APIs expected by frontend.
- [ ] Confirm debug mode does not corrupt normal runtime determinism.

## Phase 7: Compatibility Push (Commercial Cartridge Goal)
- [ ] Build licensed commercial test matrix and track status by mapper/region.
- [ ] Triage all failures by subsystem (`CPU/PPU/APU/Mapper/Timing`).
- [ ] Resolve blockers until commercial compatibility target is reached.
- [ ] Lock regression corpus for release candidate.

## Phase 8: Windows-First Release Gate
- [ ] Validate MSVC/Visual Studio build and runtime integration.
- [ ] Verify plugin discovery/load in packaged Windows Proteus output.
- [ ] Pass Windows CI deterministic and lifecycle suites.
- [ ] Confirm no P0/P1 regressions in crash, load, save, audio/video sync.

## Phase 9: Multi-Platform Expansion
- [ ] Keep plugin ABI/core portable and frontend-independent.
- [ ] Add Linux/macOS plugin builds after Windows baseline is stable.
- [ ] Promote non-Windows CI from advisory to blocking once parity is proven.

---

## Guardrails
- [ ] Do not break `IConsole` semantics consumed by `ConsoleSession`, `VideoManager`, and `AudioManager`.
- [ ] Do not require frontend changes for normal NES runtime operation.
- [ ] Preserve pointer ownership/lifetime rules for framebuffer and plugin objects.
- [ ] Keep all plugin create/destroy ownership routed through registry/manager path.

## Acceptance Criteria
- [ ] NES plugin runs through current Proteus app loop without architecture changes in host.
- [ ] Commercial cartridge compatibility target achieved with documented coverage.
- [ ] Debug, pause/resume, reset, and SST paths all function through existing frontend/session flow.
- [ ] Windows release artifact is discoverable, loadable, and stable.