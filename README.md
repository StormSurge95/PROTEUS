<a id="top"></a>

# PROTEUS: GOD OF EMULATION

PROTEUS is a multi-console emulator project focused on hardware-accurate
software emulation, with integrated tooling intended primarily to support
console game-development workflows (especially homebrew and testing).

### Table of Contents
- [Project Status](#project-status)
- [Core Objectives](#core-objectives)
- [Roadmap Direction](#roadmap-direction)
- [Current Notes](#current-notes)
- [License and Legal Scope](#license-and-legal-scope)
- [Third-Party Dependencies](#third-party-dependencies)
- [Problems/Questions/Comments](#problemsquestionscomments)

---

## Project Status
This project is in active development and is not production-ready.

Current system status:
- NES: in progress (currently supports mappers 0-3)
- SNES: planned
- GBA: planned
- other systems to be announced

[Back to top](#top)

---

## Core Objectives
- Emulate supported systems as accurately as practical
- Provide a unified interface for selecting systems and loading ROMs
- Provide debugging and analysis tools for game-development and test workflows
  on implemented systems

[Back to top](#top)

---

## Roadmap Direction
- NES is the current primary focus
- Next major core is expected to be SNES or GBA
- Additional systems will be added incrementally as each implementation matures

[Back to top](#top)

---

## Current Notes
- The central UI flow is functional, but still under active design iteration.
- Compatibility and tooling coverage will expand per-system as development progresses.

[Back to top](#top)

---

## License and Legal Scope
This project is distributed under the license in `LICENSE.md`.

That license applies to the software project distributed as "Proteus"
(the "Software"), including source code, binaries, build scripts,
documentation, and related materials in this repository, unless a file or
directory explicitly states otherwise.

Summary:
- Personal use is permitted.
- Professional non-commercial use is permitted.
- Direct commercial use requires a separate written agreement from the Original Developer.
- No copyrighted/trademarked third-party firmware/ROM/BIOS assets are intentionally included.

For commercial licensing requests, use the contact method listed in `LICENSE.md`.

[Back to top](#top)

---

## Third-Party Dependencies
This repository uses third-party components with separate license terms:
- `third_party_licenses/imgui-MIT.txt`
- `third_party_licenses/sdl3-zlib.txt`
- `third_party_licenses/nlohmann-json-MIT.txt`
- `third_party_licenses/openssl-Apache-2.0.txt`
- `Proteus/THIRD_PARTY_NOTICES.md`

[Back to top](#top)

---

## Problems/Questions/Comments
If you have any issues and/or suggestions, please submit an issue with all
relevant details.

[Back to top](#top)

