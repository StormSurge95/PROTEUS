# GBA Emulator Core Development Plan

## 1. Timing Foundation
- [x] Correct constants and data types.
- [x] Define the global scheduler and deterministic reset state.
- [x] Separate `InstructionSet { Arm, Thumb }` from hardware compatibility mode.
- [ ] Establish trace records for every bus access and hardware event.

## 2. Bus and Memory Map
- [ ] BIOS, EWRAM, IWRAM, I/O, palette, VRAM, OAM, ROM and save-memory regions.
- [ ] Width-specific behavior, mirroring, open bus and unaligned reads.
- [ ] Sequential/nonsequential accesses, `WAITCNT`, Game Pak prefetch, and video contention.
- [ ] BIOS read restrictions.

## 3. Game Pak Phase A: Cartridge Execution
- [ ] Load and own ROM data.
- [ ] Validate file size and cartridge header.
- [ ] Parse title, game code, maker code, and version.
- [ ] Map ROM into `0x08000000`—`0x0DFFFFFF`.
- [ ] Implement ROM mirroring behavior.
- [ ] Distinguish Game Pak wait-state regions 0, 1, and 2.
- [ ] Implement sequential versus nonsequential ROM accesses.
- [ ] Connect `WAITCNT`.
- [ ] Establish Game Pak prefetch behavior.
- [ ] Provide deterministic invalid/out-of-range behavior

## 4. ARM7TDMI
- [ ] Banked registers, CPSR/SPSR and exception modes.
- [ ] Three-stage pipeline and refill behavior.
- [ ] ARM and Thumb decoders.
- [ ] Operand-PC semantics, condition failures, multiply timing, exceptions, IRQ latency and unaligned-load rotation.
- [ ] Instruction execution represented as timed bus/micro-operations.

## 5. DMA, Timers, Interrupts, and Power States
- [ ] Four DMA channels with priority and bus ownership.
- [ ] Immediate, HBlank, VBlank and FIFO triggers.
- [ ] Cascaded timers and overflow ordering.
- [ ] `IE`, `IF`, `IME`, HALT and STOP behavior.

## 6. Game Pak Phase B: Writable Media and Peripherals
- [ ] Implement SRAM.
- [ ] Flash command protocols and bank switching.
- [ ] EEPROM serial protocol.
- [ ] Save-size configuration and detection.
- [ ] GPIO
- [ ] Real-time clock.
- [ ] Rumble.
- [ ] Solar sensor.
- [ ] Gyroscope or tilt hardware where applicable.
- [ ] Other cartridge-specific hardware.

## 7. PPU
- [ ] Exactly 960 visible cycles plus 272 HBlank cycles per line.
- [ ] HBlank/VBlank flag, IRQ and DMA transition timing.
- [ ] Modes 0-5, text and affine backgrounds, OBJ evaluation, windows, mosaic and color effects.
- [ ] CPU/PPU contention for VRAM, palette RAM and OAM.

## 8. APU
- [ ] Four PSG channels, wave RAM behavior, Direct Sound FIFOs and timer-driven FIFO DMA.
- [ ] Accurate bias, clipping and internal channel timing.
- [ ] Produce the GBA's hardware-accurate stereo output at the sampling cadence and resolution selected by `SOUNDBIAS`. Expose the resulting native-format stream and its metadata to Proteus. Perform device resampling, channel conversion, and user-selected downmixing in the host

## 9. PROTEUS Integration
- [ ] Concrete `IConsole` implementation and required exports.
- [ ] `clockFrame()` advances exactly 280,896 emulated cycles.
- [ ] `clock()` calls `clockFrame()` without altering emulated behavior.
- [ ] Stable 240x160 ABGR8888 framebuffer.
- [ ] Ten-button active-low keypad mapping.
- [ ] GBA mappings in the plugin and input managers.
- [ ] Guard the optional debugger during session reset.
- [ ] User-supplied real BIOS support; HLE BIOS should be a secondary compatibility mode, not eh cycle-accuracy reference.