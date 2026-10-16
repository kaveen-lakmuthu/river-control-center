# River Control Center

A small Qt 6/Kirigami control center for a River Classic desktop. It provides
one interface for the shared colour palette, River borders and layout spacing,
Waybar layout, and the custom desktop panels used by this configuration.

The application is deliberately split from the dotfiles it manages. Source
code can live anywhere, while user state is stored according to the XDG base
directory convention:

```text
$XDG_CONFIG_HOME/river-control-center/settings.json
$XDG_CONFIG_HOME/river-control-center/generated/
$XDG_CONFIG_HOME/river-control-center/backups/
```

When `XDG_CONFIG_HOME` is unset, this resolves to
`~/.config/river-control-center`.

## Features

- Semantic desktop colour palette with built-in presets
- River border colours, border width, and rivertile spacing
- Waybar size, spacing, module visibility, clock, and battery thresholds
- Clipboard and notification history limits
- Shared popup dimensions, padding, and screen-edge margins
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

On Fedora:

```sh
sudo dnf install \
  gcc-c++ cmake ninja-build \
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
```

`--check` validates the active settings. `--regenerate` rebuilds all generated
integration fragments without starting the graphical interface.

## Repository layout

```text
data/       Desktop-entry template
qml/        Kirigami user interface
src/        C++ settings and generation backend
build.sh    User-local build/install helper
```

## License

No license has been selected yet. Add one before publishing the repository if
you want others to be able to copy, modify, or redistribute the project.
