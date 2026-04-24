# LUNAR --FX

<div align="center">

**Unique After Effects Plugins for Video Editors**

[![GitHub](https://img.shields.io/badge/GitHub-View%20Source-lightgrey?logo=github)](https://github.com/NiklasNK-Creator/LUNAR--FX)
[![License](https://img.shields.io/badge/License-Proprietary-red.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Windows-blue.svg)](https://www.microsoft.com/windows)
[![After Effects](https://img.shields.io/badge/After%20Effects-2020%2B-purple.svg)](https://www.adobe.com/products/aftereffects)
[![Version](https://img.shields.io/badge/Version-1.0.0-green.svg)](https://github.com/NiklasNK-Creator/LUNAR--FX/releases)

</div>

---

## 📖 Overview

LUNAR --FX is a collection of After Effects plugins designed to **let your edit shine like a moon**. Our effects provide subtle but powerful enhancements for professional video editing.

**Source Code**: Available for viewing on GitHub (read-only)
**Distribution**: Via standalone installer that bundles all plugins internally

---

## ✨ LUNAR.SNITCH

The first effect in the LUNAR --FX collection - let your edit shine like a moon.

### Features

- **Single Intensity Control**: One slider controls all effects proportionally
- **Chromatic Aberration**: Position-based chromatic shift (30% scale)
- **Contrast Enhancement**: Enhanced contrast for cinematic pop (40% scale)
- **Glow Effect**: Subtle mid-tone glow (60% scale)
- **Smart Rendering**: Optimized for After Effects' smart rendering
- **Multiple Formats**: Supports 8-bit, 16-bit, 32-bit pixel formats
- **Premiere Pro Compatible**: Works in both AE and Premiere Pro

### Parameters

- **Intensity** (0-100): Single control for all enhancement effects

### Usage

1. Apply LUNAR.SNITCH to your footage
2. Adjust the Intensity slider
3. All effects scale proportionally for consistent results

---

## 🚀 Installation

### Quick Install

1. Download `LUNAR_FX_Installer.exe` from the [Releases](https://github.com/LUNAR-FX/LUNAR--FX/releases) section
2. Run the installer
3. Select your After Effects version (2020 or newer)
4. Click "Install LUNAR --FX"
5. Restart After Effects

The installer automatically bundles all plugins and installs them to the correct location.

### Manual Installation

1. Navigate to After Effects Plug-ins folder
2. Create `LUNAR --FX` directory
3. Copy `.aex` files into the directory
4. Restart After Effects

---

## 🔧 Compatibility

| Feature | Support |
|---------|---------|
| After Effects | 2020 and newer |
| Premiere Pro | Compatible |
| Platform | Windows only |
| Pixel Formats | 8-bit, 16-bit, 32-bit float |

---

## 📁 Project Structure

```
LUNAR --FX/
├── Source/
│   ├── LUNAR_SNITCH/          # Plugin source code
│   └── Installer/             # Python installer
├── Build/                     # Compiled plugins & installer
├── SDK/                       # After Effects SDK
└── Documentation/             # Development docs
```

---

## 🛠️ Development

### Building from Source

**Prerequisites:**
- Windows 10+
- Visual Studio 2026 with C++ build tools
- After Effects SDK
- Python 3.8+

**Build Steps:**
```bash
# Build plugin
msbuild Source/LUNAR_SNITCH/LUNAR_SNITCH_minimal.vcxproj /p:Configuration=Release /p:Platform=x64

# Build installer
python Source/Installer/build_installer.py
```

---

## 📜 License

**LUNAR --FX is proprietary software.**

- **Source Code**: Available for viewing on GitHub (read-only)
- **Distribution**: Via official installer only
- **Commercial Use**: Contact LUNAR --FX Team
- All rights reserved © 2026 LUNAR --FX

---

## 🤝 Support

For issues, questions, or feature requests, please open an issue on GitHub.

---

<div align="center">

**Made with ❤️ for TikTok editors**

[![GitHub](https://img.shields.io/badge/GitHub-View%20Source-lightgrey?logo=github)](https://github.com/LUNAR-FX/LUNAR--FX)

</div>
