# Contract Version History

## Purpose
This file is the canonical history for public contract version changes between frontend and plugin cores.

Tracked contracts:
- `IConsole` (`ICONSOLE_CONTRACT_VERSION`)
- `IDebugger` (`IDEBUGGER_CONTRACT_VERSION`)

Current source-of-truth headers:
- `PROTEUS/Proteus/src/shared/IConsole.h`
- `PROTEUS/Proteus/src/shared/IDebugger.h`

## Versioning Policy

Increment contract version when a change is ABI/API-breaking for plugin binaries, including:
- method signature changes
- method add/remove/reorder in interface vtable
- ownership/lifetime contract changes that invalidate existing plugins
- struct/layout changes that cross plugin boundary

Do not increment for:
- comments/docs only
- internal implementation changes that do not alter external contract behavior

Current host behavior requires exact version match at plugin load time.

## IConsole History

| Version | Date | Change Summary | Migration Notes |
|---|---|---|---|
| 1 | 2026-05-30 | Baseline contract established for plugin architecture migration. | Existing plugins must export manifest with `iConsoleContractVersion = 1`. |

## IDebugger History

| Version | Date | Change Summary | Migration Notes |
|---|---|---|---|
| 1 | 2026-05-30 | Baseline debugger contract established for plugin architecture migration. | Existing plugins must export manifest with `iDebuggerContractVersion = 1`. |

## Contract Bump Checklist

When changing either contract:
1. Update `ICONSOLE_CONTRACT_VERSION` and/or `IDEBUGGER_CONTRACT_VERSION`.
2. Update plugin manifests for all in-repo plugins.
3. Add a new row in this file for each changed contract.
4. Update/extend compatibility and mismatch tests.
5. Update `docs/PLUGIN_API.md` references if behavior changed.

