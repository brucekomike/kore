# kore

a projects managing software.

A cross-platform Qt desktop application that gives you a panel view of your
workspace folder (auto-resolved to `~/Workspace` or `~/Projects`, or a
custom folder of your choosing).

## Features

- Auto-resolves the workspace root (`~/Workspace`, then `~/Projects`, falling
  back to your home folder), with an optional manual override.
- Grouped view: projects nested one folder deep (e.g. `Workspace/work/service`)
  are grouped by that subfolder; top-level git repositories are shown
  ungrouped.
- Recently opened projects list, tracked automatically whenever you open a
  project through kore.
- Repo basics at a glance: on-disk size, last update time (last commit time
  for git repositories, otherwise the most recent file modification), and
  last opened time.
- Each project card has buttons to open it in your configured IDE, CLI, or
  agent IDE. All three commands are configurable via
  **Settings**.

## Building

Requirements: CMake >= 3.16, a C++17 compiler, and Qt 6 (Widgets + Test
modules).

```sh
cmake -B build -S .
cmake --build build
```

Or use the quick-build helper:

```sh
./bin/build
```

This produces the `kore` executable, plus a `kore_tests` test suite runnable
via `ctest`:

```sh
cd build
ctest --output-on-failure
```

On macOS, the application is generated as `build/kore.app`.

Tagged releases build a Linux package with desktop registration, a macOS DMG,
and a Windows ZIP containing the compiled executable. The Linux package installs the `kore` desktop entry
and application icon under the standard XDG directories.

## Configuration

Open **Settings** from the toolbar to configure:

- The workspace folder (leave empty to auto-detect).
- The command used to open a project in your IDE, CLI, or agent IDE. Use `%1`
  as a placeholder for the project path — it is
  automatically shell-quoted before substitution, so do not wrap it in your
  own quotes, e.g. `code %1`.
- IDE and agent IDE command templates can be auto-detected from installed
  applications in **Settings**.

Settings are persisted using Qt's native per-platform storage (the Windows
registry, macOS preferences, or an INI file under `~/.config` on Linux).
