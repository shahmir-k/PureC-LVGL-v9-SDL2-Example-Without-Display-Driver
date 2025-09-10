# PureC-LVGL-v9-SDL2-Example-Without-Display-Driver

[![LVGL](https://img.shields.io/badge/LVGL-9.3-brightgreen.svg)](https://lvgl.io/)
[![SDL2](https://img.shields.io/badge/SDL-2.0-orange.svg)](https://www.libsdl.org/)
[![Language](https://img.shields.io/badge/language-C-blue.svg)]()
[![Build System](https://img.shields.io/badge/build-Make-red.svg)]()
[![Dependencies](https://img.shields.io/badge/dependencies-none-green.svg)]()

### Platforms
[![Linux](https://img.shields.io/badge/Linux-FCC624?style=flat&logo=linux&logoColor=black)]()
[![Windows](https://img.shields.io/badge/Windows-0078D6?style=flat&logo=windows&logoColor=white)]()
[![macOS](https://img.shields.io/badge/macOS-000000?style=flat&logo=apple&logoColor=white)]()
[![Embedded](https://img.shields.io/badge/Embedded%20Systems-2C3E50?style=flat&logo=arduino&logoColor=white)]()

### Architectures
[![x86](https://img.shields.io/badge/x86-Intel-blue.svg?style=flat&logo=intel&logoColor=white)]()
[![x86_64](https://img.shields.io/badge/x86__64-AMD-ED1C24.svg?style=flat&logo=amd&logoColor=white)]()
[![ARM](https://img.shields.io/badge/ARM-FA7343?style=flat&logo=arm&logoColor=white)]()
[![ARM64](https://img.shields.io/badge/ARM64-FA7343?style=flat&logo=arm&logoColor=white)]()

### Development
[![Cross Platform](https://img.shields.io/badge/Cross%20Platform-✓-success.svg)]()
[![Build Status](https://img.shields.io/badge/build-passing-success.svg)]()
[![Code Size](https://img.shields.io/badge/code%20size-lightweight-blue.svg)]()
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)]()
[![Documentation](https://img.shields.io/badge/docs-up%20to%20date-brightgreen.svg)]()
[![Maintenance](https://img.shields.io/badge/Maintained%3F-yes-green.svg)]()
[![Last Commit](https://img.shields.io/badge/last%20commit-active-success.svg)]()
[![Release](https://img.shields.io/badge/latest%20release-v1.0-blue.svg)]()

A cross-platform example application demonstrating the latest LVGL (9.3) in pure C. Using direct SDL2 access without any display drivers. This project showcases modern UI development and comprehensive input support in a low level language.

This project was started as an attempt to make a muOS application using pure C and the latest LVGL. Due to a custom SDL2 integration in muOS, this project bypasses all LVGL display drivers and utilizes an internal buffer to directly draw to the screen. This should allow the app even greater cross-platform support as it doesn't rely on the SDL display driver. 

Due to the low level implementation of this app it should be compatible with nearly all operating systems and embedded environments. 

If SDL2 can draw to your screen, then your device can run this app. 

#### TODO
- make multiple controllers work at the same time(ez im just lazy and keep forgetting about this)

## 🚀 Features

- 🖥️ Pure C implementation
- 🏎️ EXTREMELY fast, lightweight, and resource efficient
- ✅ Valgrind tested for proper memory management (no leaks)
- 🎮 Full gamepad support with SDL2
- ⌨️ Keyboard and mouse input handling
- 📱 Cross-platform compatibility
- 🖼️ Hardware-accelerated graphics
- 🎯 Dynamic window resizing
- 📦 Comprehensive build system
- 🔄 Double-buffered rendering
- 🎨 Modern UI with LVGL 9.3

## 📸 Screenshots

### Desktop Screenshot
![Desktop Screenshot](pictures/desktop.png)

### muOS Screenshot
![muOS Interface](pictures/muOS.jpg)



## 📋 Build Prerequisites

- SDL2 development libraries installed
- SDL2_ttf development libraries installed (Will be removed in the future)
- GCC/Make build tools
- For ARM64 cross-compilation:
  - ARM64 toolchain
  - ARM64 SDL2 libraries
  - Raspberry Pi will make things a lot easier if you only have x86 computers

## 🛠️ Installation

### Normal Install

Move the app-exampleLVGL folder from the repo into your muOS applications folder

OR 

Place release archive in ARCHIVE folder of your muOS sdcard then install with archive manager


Done!



<br>
<br>
<br>

### Building From Scratch (For Devs)

For native (x86) build:
```bash
cd app-exampleLVGL
make
```

For ARM64 build:
```bash
# Use a raspberry pi

# update the make-rpi.sh script with your ssh credentials

./make-rpi.sh

# To automatically upload to your muOS device do:

./make-rpi.sh -upload
```



## 📁 Project Structure

```
PureC-LVGL-9.3-SDL2-Example-Without-Display-Driver/
├── app-exampleLVGL/           # Main application directory
│   ├── components/           # External components
│   │   └── lvgl/            # Place LVGL 9.3 library here
│   ├── main/                # Application source code
│   │   ├── config/         # Configuration files
│   │   │   └── lv_conf.h   # LVGL configuration
│   │   ├── fonts/         # Font resources
│   │   ├── input.c        # Input handling
│   │   ├── input.h        # Input declarations
│   │   └── main.c         # Main application
│   ├── Makefile           # Build configuration
│   ├── mux_launch.sh      # Launch script for muOS
│   └── valgrind.supp      # Valgrind suppressions
├── make_rpi.sh            # Raspberry Pi build script
├── README.md              # Project documentation
└── upload.sh              # Device upload script
```

## 🎮 Input Support

### Gamepad
- Full SDL2 gamepad support
- Configurable button mappings
- Multiple controller support
- Hot-plugging support

### Keyboard & Mouse
- Complete keyboard navigation
- Mouse interaction support
- Focus management
- Scroll wheel support

## 🖥️ Display Features

- Hardware-accelerated rendering
- Double-buffered display
- Dynamic resolution support
- Partial screen updates
- ARGB8888 32-bit color format
- Vsync support

