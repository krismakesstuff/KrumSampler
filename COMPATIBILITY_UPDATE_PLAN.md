# KrumSampler Compatibility Update Plan

## Overview

This document outlines the complete plan for updating KrumSampler to work on modern macOS and Windows systems, migrating from Projucer to CMake, and setting up a cross-platform VSCode development environment.

**Current State:**
- JUCE Version: Unknown (estimated 6.x from 2022)
- Build System: Projucer (.jucer file)
- IDE: Visual Studio 2019 (Windows), Xcode (macOS)
- Last Updated: March 2022

**Target State:**
- JUCE Version: 8.x (latest stable)
- Build System: CMake 3.25+
- IDE: VSCode (cross-platform)
- Target OS: macOS 10.13+, Windows 10+
- Architecture: Universal Binary (arm64 + x86_64) for macOS

---

## Phase 1: Preparation and Branch Setup

### 1.1 Merge Dev Branch
The `dev` branch contains significant updates including initial Apple Silicon testing on M3 Sonoma 14.3.1.

```bash
git checkout main
git merge dev
```

### 1.2 Create Backup of Current Project Structure
Before migration, preserve the original Projucer setup:

```bash
git checkout -b backup/projucer-original
git push origin backup/projucer-original
git checkout main
```

### 1.3 Document Current Source Files
Create an inventory of all source files that will need to be included in CMake:

**Source Files (from Source/ directory):**
- PluginProcessor.h/cpp
- PluginEditor.h/cpp
- KrumSampler.h/cpp
- KrumModule.h/cpp
- KrumModuleEditor.h/cpp
- KrumModuleContainer.h/cpp
- KrumFileBrowser.h/cpp
- SimpleAudioPreviewer.h/cpp
- InfoPanel.h/cpp
- ColorPalette.h
- TimeHandle.h/cpp
- ModuleSettingsOverlay.h/cpp
- DragAndDropThumbnail.h/cpp
- And additional source files

**Resources (from Resources/ directory):**
- Fonts (Montserrat family - 11 TTF files)
- Demo Kit (7 WAV files)
- SVG icons (20+ icons)
- PNG images

---

## Phase 2: Development Environment Setup

### 2.1 Install Required Tools

#### All Platforms
- **CMake 3.25+**: https://cmake.org/download/
- **Git**: Latest version with LFS support
- **VSCode**: https://code.visualstudio.com/

#### VSCode Extensions (Required)
```
ms-vscode.cpptools              # C/C++ IntelliSense
ms-vscode.cmake-tools           # CMake Tools
twxs.cmake                      # CMake Language Support
llvm-vs-code-extensions.vscode-clangd  # Clangd (recommended over IntelliSense)
```

#### macOS Specific
- **Xcode Command Line Tools**: `xcode-select --install`
- **Xcode** (full installation for code signing): App Store
- **Clang/LLVM**: Comes with Xcode

#### Windows Specific
- **Visual Studio 2022 Build Tools** (or full VS2022)
  - Select "Desktop development with C++" workload
  - Include Windows 10/11 SDK
- **LLVM/Clang** (optional, for clangd): https://releases.llvm.org/

### 2.2 VSCode Workspace Configuration

Create `.vscode/settings.json`:
```json
{
    "cmake.configureOnOpen": true,
    "cmake.buildDirectory": "${workspaceFolder}/build",
    "cmake.generator": "Ninja",
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools",
    "files.associations": {
        "*.h": "cpp",
        "*.cpp": "cpp"
    },
    "editor.formatOnSave": true,
    "C_Cpp.clang_format_fallbackStyle": "LLVM"
}
```

Create `.vscode/launch.json` for debugging:
```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug Standalone",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/KrumSampler_artefacts/Debug/Standalone/KrumSampler.app/Contents/MacOS/KrumSampler",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "MIMode": "lldb"
        },
        {
            "name": "Debug with Plugin Host",
            "type": "cppdbg",
            "request": "launch",
            "program": "/path/to/AudioPluginHost.app/Contents/MacOS/AudioPluginHost",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "MIMode": "lldb"
        }
    ]
}
```

---

## Phase 3: JUCE 8.x Integration

### 3.1 Add JUCE as Git Submodule

```bash
# Remove old JUCE references (external path)
# Add JUCE 8.x as submodule
mkdir -p libs
git submodule add https://github.com/juce-framework/JUCE.git libs/JUCE
cd libs/JUCE
git checkout master  # or specific tag like 8.0.4
cd ../..
git add .gitmodules libs/JUCE
git commit -m "Add JUCE 8.x as git submodule"
```

### 3.2 JUCE 8.x Migration Notes

**Breaking Changes from JUCE 6/7 to 8:**
- Review JUCE changelog for API changes
- `juce::ScopedPointer` fully removed (already using `std::unique_ptr`)
- Some deprecated methods removed
- Check for namespace changes

**Required Code Updates (verify after migration):**
- Ensure all `#include` paths are correct
- Verify JUCE module includes
- Check for deprecated API usage

---

## Phase 4: CMake Project Setup

### 4.1 Create Root CMakeLists.txt

Create `CMakeLists.txt` in project root:

```cmake
cmake_minimum_required(VERSION 3.25)

# Project version - update from PluginProcessor.h
project(KrumSampler VERSION 1.5.0)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Export compile commands for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# macOS deployment target
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS version")
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build universal binary")
endif()

# Add JUCE
add_subdirectory(libs/JUCE)

# Plugin source files
set(PLUGIN_SOURCES
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/KrumSampler.cpp
    Source/KrumSampler.h
    Source/KrumModule.cpp
    Source/KrumModule.h
    Source/KrumModuleEditor.cpp
    Source/KrumModuleEditor.h
    Source/KrumModuleContainer.cpp
    Source/KrumModuleContainer.h
    Source/KrumFileBrowser.cpp
    Source/KrumFileBrowser.h
    Source/SimpleAudioPreviewer.cpp
    Source/SimpleAudioPreviewer.h
    Source/InfoPanel.cpp
    Source/InfoPanel.h
    Source/ColorPalette.h
    Source/TimeHandle.cpp
    Source/TimeHandle.h
    Source/ModuleSettingsOverlay.cpp
    Source/ModuleSettingsOverlay.h
    Source/DragAndDropThumbnail.cpp
    Source/DragAndDropThumbnail.h
    # Add any additional source files here
)

# Create the audio plugin
juce_add_plugin(KrumSampler
    COMPANY_NAME "Kris Crawford"
    COMPANY_WEBSITE "www.krismakesmusic.com"
    COMPANY_EMAIL "kris@krismakesmusic.com"

    PLUGIN_MANUFACTURER_CODE Kris
    PLUGIN_CODE Krum

    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE

    COPY_PLUGIN_AFTER_BUILD TRUE

    FORMATS VST3 AU Standalone  # Add AAX when SDK is available

    PRODUCT_NAME "KrumSampler"

    VST3_CATEGORIES "Instrument" "Sampler"
    AU_MAIN_TYPE "kAudioUnitType_MusicDevice"
)

# Add source files to target
target_sources(KrumSampler PRIVATE ${PLUGIN_SOURCES})

# Add binary resources (fonts, images, audio files)
juce_add_binary_data(KrumSamplerData
    SOURCES
        # Fonts
        Resources/Fonts/Montserrat-Black.ttf
        Resources/Fonts/Montserrat-Bold.ttf
        Resources/Fonts/Montserrat-ExtraBold.ttf
        Resources/Fonts/Montserrat-ExtraLight.ttf
        Resources/Fonts/Montserrat-Italic-VariableFont_wght.ttf
        Resources/Fonts/Montserrat-Light.ttf
        Resources/Fonts/Montserrat-Medium.ttf
        Resources/Fonts/Montserrat-Regular.ttf
        Resources/Fonts/Montserrat-SemiBold.ttf
        Resources/Fonts/Montserrat-Thin.ttf
        Resources/Fonts/Montserrat-VariableFont_wght.ttf
        # Demo Kit
        Resources/DemoKit/21\ Pilots\ Kick\ Sample.wav
        Resources/DemoKit/808\ and\ House\ Kick\ blend.wav
        Resources/DemoKit/GW\ Monster\ clap_snare.wav
        Resources/DemoKit/HI\ HATS\ V4\ -\ A.wav
        Resources/DemoKit/HI\ HATS\ V10\ -\ A.wav
        Resources/DemoKit/Marvin\ Snap.wav
        # SVG Icons (add all from Resources/)
        Resources/add-black-18dp.svg
        Resources/add_white_24dp.svg
        Resources/arrow_upward_white_24dp.svg
        Resources/audio_file_white_24dp.svg
        Resources/chevron_left_black_24dp.svg
        Resources/chevron_right_black_24dp.svg
        Resources/clear-black-18dp.svg
        Resources/drag_handle-black-18dp.svg
        Resources/folder_open_white_24dp.svg
        Resources/folder_white_24dp.svg
        Resources/file_white_24dp.svg
        Resources/info_white_24dp.svg
        Resources/info_white_filled_24dp.svg
        Resources/play-black-18dp.svg
        Resources/menu-black-18dp.svg
        Resources/lock_open-black-18dp.svg
        # Add remaining resources...
)

# Link binary data to plugin
target_link_libraries(KrumSampler PRIVATE KrumSamplerData)

# JUCE compile definitions
target_compile_definitions(KrumSampler
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0  # Requires JUCE license for commercial use
)

# Link JUCE modules
target_link_libraries(KrumSampler
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_dsp
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
```

### 4.2 Create CMake Presets (Optional but Recommended)

Create `CMakePresets.json`:

```json
{
    "version": 6,
    "cmakeMinimumRequired": {
        "major": 3,
        "minor": 25,
        "patch": 0
    },
    "configurePresets": [
        {
            "name": "default",
            "hidden": true,
            "generator": "Ninja",
            "binaryDir": "${sourceDir}/build/${presetName}",
            "cacheVariables": {
                "CMAKE_EXPORT_COMPILE_COMMANDS": "ON"
            }
        },
        {
            "name": "debug",
            "inherits": "default",
            "displayName": "Debug",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Debug"
            }
        },
        {
            "name": "release",
            "inherits": "default",
            "displayName": "Release",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Release"
            }
        },
        {
            "name": "macos-universal",
            "inherits": "release",
            "displayName": "macOS Universal",
            "condition": {
                "type": "equals",
                "lhs": "${hostSystemName}",
                "rhs": "Darwin"
            },
            "cacheVariables": {
                "CMAKE_OSX_ARCHITECTURES": "arm64;x86_64",
                "CMAKE_OSX_DEPLOYMENT_TARGET": "10.13"
            }
        },
        {
            "name": "windows-x64",
            "inherits": "release",
            "displayName": "Windows x64",
            "condition": {
                "type": "equals",
                "lhs": "${hostSystemName}",
                "rhs": "Windows"
            },
            "architecture": {
                "value": "x64",
                "strategy": "set"
            }
        }
    ],
    "buildPresets": [
        {
            "name": "debug",
            "configurePreset": "debug"
        },
        {
            "name": "release",
            "configurePreset": "release"
        },
        {
            "name": "macos-universal",
            "configurePreset": "macos-universal"
        },
        {
            "name": "windows-x64",
            "configurePreset": "windows-x64"
        }
    ]
}
```

---

## Phase 5: Build and Test

### 5.1 Initial Build Commands

#### macOS
```bash
# Configure
cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=10.13 \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"

# Build
cmake --build build

# Build artifacts location
# VST3: build/KrumSampler_artefacts/Debug/VST3/KrumSampler.vst3
# AU:   build/KrumSampler_artefacts/Debug/AU/KrumSampler.component
# App:  build/KrumSampler_artefacts/Debug/Standalone/KrumSampler.app
```

#### Windows (PowerShell)
```powershell
# Configure
cmake -B build -G "Visual Studio 17 2022" -A x64

# Build
cmake --build build --config Debug

# Build artifacts location
# VST3: build\KrumSampler_artefacts\Debug\VST3\KrumSampler.vst3
# App:  build\KrumSampler_artefacts\Debug\Standalone\KrumSampler.exe
```

### 5.2 VSCode Build Workflow

1. Open project folder in VSCode
2. CMake Tools should auto-detect CMakeLists.txt
3. Press `Ctrl/Cmd + Shift + P` → "CMake: Configure"
4. Select kit (Clang on macOS, MSVC or Clang on Windows)
5. Press `Ctrl/Cmd + Shift + P` → "CMake: Build" or press F7

### 5.3 Testing Checklist

**Standalone App:**
- [ ] App launches without crashes
- [ ] Audio output works
- [ ] MIDI input recognized
- [ ] File browser loads
- [ ] Demo kit loads correctly
- [ ] Drag and drop samples work

**Plugin (VST3/AU):**
- [ ] Plugin loads in DAW
- [ ] No audio glitches
- [ ] State save/restore works
- [ ] UI renders correctly
- [ ] MIDI mapping works

**Platform-Specific:**
- [ ] macOS Intel: Test on 10.13+ if available
- [ ] macOS Apple Silicon: Test native arm64 performance
- [ ] Windows 10: Test VST3 loading
- [ ] Windows 11: Verify compatibility

---

## Phase 6: Known Bugs to Address

### 6.1 Audio Previewer Bug (Priority: High)

**Issue:** Intermittent audio previewer issue in Logic Pro only.

**Location:** `Source/SimpleAudioPreviewer.h/cpp`

**Investigation Steps:**
1. Review Logic Pro's audio threading model
2. Check for thread-safety issues in preview playback
3. Verify buffer handling matches Logic's requirements
4. Test with JUCE's `AudioProcessorGraph` for isolation

**Commit Reference:** `e7541ce` - "Fixed bug in SimpleAudioPreviewer.h"

### 6.2 Additional Bugs

Document additional bugs here as they are identified during testing:

| Bug ID | Description | File(s) | Status |
|--------|-------------|---------|--------|
| BUG-001 | Audio previewer intermittent in Logic | SimpleAudioPreviewer.cpp | Open |
| BUG-002 | (Add as discovered) | | |

---

## Phase 7: macOS Code Signing and Notarization

### 7.1 Code Signing Setup

**Prerequisites:**
- Apple Developer Account ($99/year)
- Developer ID Application certificate
- Developer ID Installer certificate (for .pkg)

**CMake Configuration:**
```cmake
# Add to CMakeLists.txt for release builds
if(APPLE AND CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Developer ID Application")
    set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "YOUR_TEAM_ID")
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Manual")
    set(CMAKE_XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES)
endif()
```

### 7.2 Notarization Process

```bash
# After building release
# 1. Create zip of plugin
ditto -c -k --keepParent build/KrumSampler_artefacts/Release/VST3/KrumSampler.vst3 KrumSampler.zip

# 2. Submit for notarization
xcrun notarytool submit KrumSampler.zip \
    --apple-id "your@email.com" \
    --team-id "TEAM_ID" \
    --password "app-specific-password" \
    --wait

# 3. Staple ticket
xcrun stapler staple build/KrumSampler_artefacts/Release/VST3/KrumSampler.vst3
```

### 7.3 Entitlements File

Create `Entitlements.plist`:
```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>com.apple.security.cs.allow-unsigned-executable-memory</key>
    <true/>
    <key>com.apple.security.cs.disable-library-validation</key>
    <true/>
    <key>com.apple.security.device.audio-input</key>
    <true/>
</dict>
</plist>
```

---

## Phase 8: Windows Code Signing (Optional)

### 8.1 Options

1. **Traditional EV Code Signing Certificate** (~$400/year)
   - DigiCert, Sectigo, GlobalSign

2. **Azure Trusted Signing** (Newer, cloud-based)
   - Requires Azure subscription
   - See Pamplejuce template for GitHub Actions integration

### 8.2 Signing Command
```powershell
signtool sign /fd SHA256 /tr http://timestamp.digicert.com /td SHA256 /a "KrumSampler.vst3"
```

---

## Phase 9: CI/CD Setup (Optional)

### 9.1 GitHub Actions Workflow

Create `.github/workflows/build.yml`:

```yaml
name: Build

on:
  push:
    branches: [main, dev]
  pull_request:
    branches: [main]

jobs:
  build:
    strategy:
      matrix:
        os: [macos-latest, windows-latest]

    runs-on: ${{ matrix.os }}

    steps:
      - uses: actions/checkout@v4
        with:
          submodules: recursive

      - name: Install Dependencies (macOS)
        if: runner.os == 'macOS'
        run: brew install ninja

      - name: Install Dependencies (Windows)
        if: runner.os == 'Windows'
        run: choco install ninja

      - name: Configure
        run: cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

      - name: Build
        run: cmake --build build --config Release

      - name: Upload Artifacts
        uses: actions/upload-artifact@v4
        with:
          name: KrumSampler-${{ runner.os }}
          path: build/KrumSampler_artefacts/Release/
```

---

## Phase 10: Project Cleanup

### 10.1 Files to Remove After Migration

Once CMake build is verified working:

```bash
# Remove Projucer-specific files
rm KrumSampler.jucer
rm -rf Builds/              # Old IDE project files
rm -rf JuceLibraryCode/     # Auto-generated by Projucer

# Keep or update
# - Windows Installer Script.iss (update paths)
# - MacOS Installer/MacOS Installer.pkgproj (update paths)
```

### 10.2 Update .gitignore

```gitignore
# Build output
build/
cmake-build-*/

# IDE files
.vscode/
!.vscode/settings.json
!.vscode/launch.json
!.vscode/tasks.json
*.user
.idea/

# OS files
.DS_Store
Thumbs.db

# Compiled files
*.o
*.obj
*.vst3
*.component
*.aaxplugin
```

---

## Appendix A: Resource Reference

### Official Documentation
- [JUCE CMake API](https://github.com/juce-framework/JUCE/blob/master/docs/CMake%20API.md)
- [JUCE GitHub Repository](https://github.com/juce-framework/JUCE)

### Templates & Examples
- [Pamplejuce](https://github.com/sudara/pamplejuce) - Full-featured JUCE 8 + CMake template with CI/CD
- [JUCE-CMake-Plugin-Template](https://github.com/anthonyalfimov/JUCE-CMake-Plugin-Template) - Minimal CMake template
- [juce_cmake_vscode_example](https://github.com/tomoyanonymous/juce_cmake_vscode_example) - VSCode-focused boilerplate

### Tutorials
- [Developing Audio Plugins with JUCE and VSCode](https://trirpi.github.io/posts/developing-audio-plugins-with-juce-and-visual-studio-code/)
- [How To Build An Audio Plugin With JUCE & CMake](https://thewolfsound.com/how-to-build-audio-plugin-with-juce-cpp-framework-cmake-and-unit-tests/)

---

## Appendix B: Quick Reference Commands

### CMake
```bash
# Configure Debug
cmake -B build -DCMAKE_BUILD_TYPE=Debug

# Configure Release (macOS Universal)
cmake -B build -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=10.13

# Build
cmake --build build

# Clean
cmake --build build --target clean
```

### Git Submodule
```bash
# Clone with submodules
git clone --recursive <repo-url>

# Initialize submodules after clone
git submodule update --init --recursive

# Update JUCE to latest
cd libs/JUCE && git pull origin master && cd ../..
```

### VSCode
```
Ctrl/Cmd + Shift + P  →  CMake: Configure
Ctrl/Cmd + Shift + P  →  CMake: Build
F5                    →  Start Debugging
F7                    →  Build
```

---

## Execution Checklist

- [ ] **Phase 1:** Merge dev branch, create backup
- [ ] **Phase 2:** Install tools, configure VSCode
- [ ] **Phase 3:** Add JUCE 8.x as submodule
- [ ] **Phase 4:** Create CMakeLists.txt, verify all sources included
- [ ] **Phase 5:** Build and test on both platforms
- [ ] **Phase 6:** Fix audio previewer bug and any others
- [ ] **Phase 7:** Set up macOS code signing (if distributing)
- [ ] **Phase 8:** Set up Windows code signing (if distributing)
- [ ] **Phase 9:** Set up CI/CD (optional)
- [ ] **Phase 10:** Clean up old Projucer files

---

*Document Created: January 2026*
*Target JUCE Version: 8.x*
*Target Platforms: macOS 10.13+, Windows 10+*
