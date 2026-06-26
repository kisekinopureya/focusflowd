# FocusFlow

A Qt desktop application that starts the FocusFlow D-Bus listener on launch, lets users edit action rules, and stays available in the system tray.

## Features

- D-Bus based IPC for focus change events
- Built-in action editor for start and end commands
- JSON configuration for action definitions
- Tray icon support so the app can keep running when hidden
- Support for start and end actions per application focus

## Building

### Prerequisites

- C++17 compiler
- Qt6 development files
- Qt6 DBus and Widgets development files
- nlohmann_json

### Build Instructions

```sh
mkdir build
cd build
cmake ..
make
sudo make install
```

## Configuration

Actions are stored in `~/.config/focusflow/actions.json` see `example-actions.json`

## Running FocusFlow

```sh
$ focusflowd
```

Launching the application starts the embedded daemon automatically. Closing or minimizing the window hides it to the system tray, and the tray menu can restore or quit the app.
