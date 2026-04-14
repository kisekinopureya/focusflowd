# FocusFlow Daemon

A D-Bus listener service for managing system actions based on application focus changes.

## Features

- D-Bus based IPC for focus change events
- JSON configuration for action definitions
- String-based action execution
- Support for start and end actions per application focus

## Building

### Prerequisites

- C++17 compiler
- Qt development files
- Qt DBus development files
- nlohmann_json

### Build Instructions

```sh
mkdir build
cd build
cmake (-DSYSTEMD=true or -DOPENRC=true) ..
make
sudo make install
```

## Configuration

Actions are stored in `~/.config/focusflow/actions.json` see `example-actions.json`

### Starting the Service

```sh
$ focusflowd
```

with systemd:

```sh
$ systemctl --user enable --now focusflowd
```

with openrc:

```sh
$ rc-update --user add focusflowd; rc-service --user focusflowd start
```

## Configuration Tool

Use [focusflow-config](https://github.com/kisekinopureya/focusflow-config) to manage actions visually instead of editing JSON manually.
