# INX - Infinity System Information

A minimal and elegant system information tool for Linux, displaying system details in a clean ASCII art format.

## Features

- **OS** - Operating System name and version
- **Kernel** - Linux kernel version
- **Uptime** - System uptime
- **Packages** - Number of installed packages
- **Shell** - Current shell
- **Terminal** - Terminal emulator
- **WM** - Window Manager / Desktop Environment
- **CPU** - CPU model and core count
- **GPU** - Graphics card information
- **Memory** - RAM usage with visual bar

## Building

### Requirements

- C++ compiler (g++ or clang++)
- Make

### Compilation

```bash
make
```

### Clean build artifacts

```bash
make clean
```

## Usage

```bash
./main
```

## Example Output

```
    user @ hostname
    ────────────────────────────────────────────
    OS          │ Ubuntu 22.04 LTS
    Kernel      │ 5.15.0-91-generic
    Uptime      │ 2 hours, 34 minutes
    Packages    │ 2145
    Shell       │ bash
    Terminal    │ GNOME Terminal
    WM          │ GNOME
    CPU         │ Intel Core i7
    GPU         │ NVIDIA GeForce RTX 3060
    Memory      │ 8.2 GiB / 16.0 GiB (51%)
    ────────────┴───────────────────────────────
    Usage       │ ███████████████------------- 51%
```

## License

This project is open source.
# INX
