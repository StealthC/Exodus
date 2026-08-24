# StealthC fork changelog

Changes made in this fork of [Exodus](https://github.com/RogerSanders/Exodus)
(upstream) on top of the pinned upstream base
[`08f388f`](https://github.com/RogerSanders/Exodus/commit/08f388f77040af28d16d44fdfbddb73252953161).
This fork exists to support the `exodus-mcp` project; the fork's own remote is
[`StealthC/Exodus`](https://github.com/StealthC/Exodus).

Each entry lists the fork commit, its date, and what changed and why. Entries
are newest first. Stable upstream behaviour is never altered silently; anything
that touches emulator semantics or packaging is called out here.

---

## [382d602] — 2026-08-24 — MegaDriveROMLoader: replace an externally loaded cartridge before loading

`Extensions/MDExtensions/MegaDriveROMLoader.cpp|h`

A ROM loaded by another extension (for example the MCP plugin) never enters
`MegaDriveROMLoader::_currentlyLoadedROMModuleFilePaths`, so the File-menu
loader's unload guard (list non-empty) skipped its housekeeping and then failed
with `No available connector of type SegaMegaDrive.CartridgePort could be
found!` even though that cartridge was occupying the only cartridge connector.

`LoadROMFile` now evaluates `CanModuleBeLoaded` unconditionally. When the
system reports the new module cannot be loaded, it still prefers to unload the
oldest ROM this loader loaded itself, but falls back to the oldest loaded
program module otherwise, so an active cartridge loaded outside this loader is
replaced instead of erroring. Added the private `UnloadOldestProgramModule`
helper for the fallback (module IDs are allocated in increasing order, so the
first program module in `GetLoadedModuleIDs()` is the oldest).

User-visible: the GUI "Load ROM File..." item now transparently swaps whatever
cartridge currently occupies the single Mega Drive cartridge port, including
cartridges loaded through the `exodus-mcp` `rom_load` tool.

## [82548a6] — 2026-08-24 — ci: drop Node 20 actions and the redundant 7z archive

`.github/workflows/build.yml`

`checkout` and `upload-artifact` move to v5/v6, which run on the Node 24
runtime and clear the Node 20 deprecation warning (`upload-artifact@v5` still
defaulted to Node 20, so v6 is required). The artifact is now a single GitHub
zip of `dist/Exodus`; the standalone `Exodus-<tag>.7z` archive that previously
duplicated the same files is removed.

## [21b2049] — 2026-08-24 — ci: assemble a release package matching the Exodus 2.1 layout

`.github/workflows/build.yml`, `Packaging/ReleaseSettings.xml`

The artifact no longer dumps `Assemblies/*.dll` flat at the root. It now
replicates the original Exodus 2.1 distribution:

- `Exodus.exe` and `System.dll` at the root;
- device/extension DLLs under `Plugins/`;
- the release-mode `settings.xml` (root-relative
  `Modules/Workspaces/Savestates/PersistentState/Captures` and
  `AssembliesPath=Plugins`) from `Packaging/ReleaseSettings.xml`;
- the default module and workspace XMLs from `Data/`;
- the standard `License.txt` and `ReleaseNotes.txt` plus empty runtime
  directories.

Tag pushes produce `Exodus-<tag>.7z`; branch pushes and PRs use the short SHA.
The build itself is unchanged (`Release | x64` for `ThirdPartyLibraries.sln`
then `Exodus.sln`); guards now fail fast if build or staging outputs are
missing.

## [de9c69e] — 2026-08-23 — feat: add POD trace-path setter and make trace-enable flag atomic

`Exodus/ExodusInterface.cpp|h`, `System/System.cpp|h`

Extension-boundary support for the MCP plugin's file-based trace capture, which
replaces `GetTraceLog()` after its marshaled return value was shown to crash
independently built extensions:

- `SetTraceFileLoggingPathAscii(const char*)` lets extensions route the
  processor trace log to a file without crossing the extension boundary
  through `Marshal::In<std::wstring>`, whose template instantiations do not
  interoperate with independently built extension binaries. It reuses the
  existing `SetTraceLoggingFilePath` open/close logic inside this module.
- `_traceLogEnabled` becomes `std::atomic<bool>`: the flag is written by
  debuggers while the CPU worker reads it unlocked in the `RecordTrace` hot
  path, which was a confirmed data race with `volatile bool`.

## [3cd0e68] — 2026-08-22 — fix: include algorithm for set difference

`Exodus/ViewManager.cpp`

Adds the missing `<algorithm>` include for `std::set_difference`, required by
the newer MSVC STL used in the validated build baseline.

## [d8f44e4] — 2026-08-22 — build: vendor third-party sources and add CI

`Third/*`, `Third/THIRD_PARTY_PROVENANCE.md`, `.github/workflows/build.yml`

Vendors the third-party library sources under `Third/` (with
`Third/THIRD_PARTY_PROVENANCE.md` recording provenance and licensing) so the
fork can be built without a separate third-party checkout, and adds a GitHub
Actions workflow that exercises the vendored third-party source check.

[382d602]: https://github.com/StealthC/Exodus/commit/382d602
[82548a6]: https://github.com/StealthC/Exodus/commit/82548a6
[21b2049]: https://github.com/StealthC/Exodus/commit/21b2049
[de9c69e]: https://github.com/StealthC/Exodus/commit/de9c69e
[3cd0e68]: https://github.com/StealthC/Exodus/commit/3cd0e68
[d8f44e4]: https://github.com/StealthC/Exodus/commit/d8f44e4
