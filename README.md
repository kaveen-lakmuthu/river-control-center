# River Control Center

A small Qt 6/Kirigami control center for a River Classic desktop. It provides
one interface for the shared colour palette, River window management and
rules, Waybar layout, and the custom desktop panels used by this configuration.

The application is deliberately split from the dotfiles it manages. Source
code can live anywhere, while user state is stored according to the XDG base
directory convention:

```text
$XDG_CONFIG_HOME/river-control-center/settings.json
$XDG_CONFIG_HOME/river-control-center/generated/
$XDG_CONFIG_HOME/river-control-center/themes/
$XDG_CONFIG_HOME/river-control-center/backups/
```

When `XDG_CONFIG_HOME` is unset, this resolves to
`~/.config/river-control-center`.

## Features

- Semantic desktop colour palette with built-in and saved custom themes
- Theme save, duplicate, rename, JSON import, and Documents export
- Independent Waybar, notification, launcher, and terminal previews
- Per-component fonts, shared corner radius, and popup spacing
- Solid-colour or wallpaper background through `swaybg`
- Per-component theme following controls and WCAG-style contrast warnings
- River border colours, border width, and rivertile spacing
- Rivertile main ratio, count and position, attach behavior and default mode
- Configurable tag count and mode-aware focus-follows-pointer behavior
- Graphical app-ID/title rule editor with live open-window inspection
- Searchable keyboard, pointer and media-binding editor with conflict checks
- Per-binding reset, live apply and printable Markdown keybinding export
- Waybar size, spacing, module visibility, clock, and battery thresholds
- Clipboard and notification history limits
- Shared popup dimensions, padding, and screen-edge margins
- Idle action toggles and lock, display-off, battery and AC timeouts
- Default power profile and a fixed night-light schedule
- Read-only reporting of systemd-logind's lid-close policy
- Live touchpad, keyboard repeat, layout and Compose-key configuration
- Configurable Fusuma swipe direction and finger count
- River-only autostart controls for Waybar, Mako, Kanshi, Fusuma and swayidle
- Live component status, targeted restarts and duplicate-process detection
- Portal backend and `river-session.target` diagnostics with ownership labels
- Atomic settings writes with ten timestamped backups
- Generated fragments for River, Waybar, Mako, Foot, and Fuzzel
- Live River, Waybar, Mako, and background updates where supported

This version is tailored to the companion River dotfiles. The generated files
only take effect when those programs include them from the runtime directory.

## Requirements

- CMake 3.25 or newer
- Ninja
- A C++20 compiler
- Qt 6.8 or newer: Core, Gui, Qml, Quick, and QuickControls2
- KDE Frameworks 6 Kirigami
- Wayland client development files and `wayland-scanner`

On Fedora:

```sh
sudo dnf install \
  gcc-c++ cmake ninja-build pkgconf-pkg-config wayland-devel \
  qt6-qtbase-devel qt6-qtdeclarative-devel \
  kf6-kirigami-devel kf6-qqc2-desktop-style
```

## Build and install

The helper script performs an out-of-tree Release build and installs for the
current user:

```sh
./build.sh
```

It installs:

```text
~/.local/bin/river-control-center
~/.local/share/applications/river-control-center.desktop
```

Make sure `~/.local/bin` is in the graphical session's `PATH` if launching the
application from an app launcher.

For a manual development build:

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/river-control-center
```

## Configuration

The first launch uses built-in defaults. `settings.example.json` documents the
current schema and may be copied into the runtime directory:

```sh
mkdir -p "${XDG_CONFIG_HOME:-$HOME/.config}/river-control-center"
cp settings.example.json \
  "${XDG_CONFIG_HOME:-$HOME/.config}/river-control-center/settings.json"
```

Useful non-graphical commands:

```sh
river-control-center --check
river-control-center --regenerate
river-control-center --list-windows
river-control-center --export-keybindings
```

`--check` validates the active settings. `--regenerate` rebuilds all generated
integration fragments without starting the graphical interface.
`--list-windows` prints the live app IDs and titles reported by River's
foreign-toplevel protocol, which is also what the graphical rule tester uses.

Custom themes live in `themes/`. Disabling a component's “Follow” switch
freezes its last generated theme while other components continue to follow the
shared palette. GTK and KDE application themes are intentionally not edited:
those settings are system-wide and may also change the Plasma session.

Applying changed power settings restarts only the River session's PID-tracked
`swayidle` process. It also selects the configured default power profile and
restarts `wlsunset` with the new schedule. Lid policy remains owned by
systemd-logind and is never changed by this application.

Input changes are applied immediately with `riverctl`. The application also
regenerates the Fusuma gesture configuration and restarts only the PID-tracked
Fusuma process started by the River session. English (US) remains mandatory so
the lock screen can always switch to a predictable password-entry layout.

Window settings are written to generated River fragments. Main-area settings,
attach mode, global tiled/floating mode and pointer focus apply live. Edited
rules are installed live and affect newly created windows; decoration rules
may also update existing windows. The tag count is shared with Waybar and
Fusuma, so all three remain in sync.

Keybindings are stored separately in `keybindings.json` and generated as
`generated/keybindings.sh`; `river/init` only sources that fragment. Applying
the Keybindings page first unmaps the previous set, then installs the edited
set in the active River session. A printable map is always generated at
`generated/keybindings.md`, and the Export button writes a copy named
`River-Classic-Keybindings.md` to the user's Documents directory.

The **Session & Startup** page writes `generated/session-startup.conf`, which
is sourced only by `river/init`. It does not create user services or modify
Plasma autostart. Runtime checks distinguish River-only components from portal
services shared with Plasma, detect duplicate processes, and allow targeted
restarts only while the control center is running inside River. Portals remain
D-Bus/systemd activated and are never started manually by the application.

## Repository layout

```text
data/       Desktop-entry template
qml/        Kirigami user interface
src/        C++ settings and generation backend
protocol/   Vendored upstream wlroots foreign-toplevel protocol XML
build.sh    User-local build/install helper
```

## License

No license has been selected yet. Add one before publishing the repository if
you want others to be able to copy, modify, or redistribute the project.
