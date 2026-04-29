# � Shoin

**A lightweight, portable C++ document organizer and journal application built with Qt.**

Designed for Linux (Ubuntu/Mint), Shoin provides a clean interface for managing articles, journal entries, and notes with rich-text editing capabilities. Single executable. Zero bloat. Runs anywhere.

![Shoin Screenshot](https://github.com/noblebrown-69/shoin/raw/main/shoin.png)

## Features

- Rich-text editing with HTML save/load (compatible with Word, LibreOffice, etc.)
- Organized document structure: Articles and Journal folders
- Auto-save functionality to `Docs/Shoin_AutoSave.html`
- Checklist and strikethrough support for task management
- Print support
- Modular `ShoinEditor` widget for extensibility
- Single-file AppImage or tiny executable — truly portable
- Warm, readable interface

## Building from Source

### Prerequisites
- Qt6 (Core, Widgets, Gui, PrintSupport)
- CMake
- C++ compiler (GCC/Clang)

### Build Steps
```bash
mkdir build
cd build
cmake ..
make
```

### Creating AppImage
Use the provided scripts:
```bash
./build-appimage.sh
```

## Download (Recommended)

Download the latest `Shoin.AppImage` from the [Releases page](https://github.com/noblebrown-69/shoin/releases).

```bash
chmod +x Shoin.AppImage
./Shoin.AppImage
```

## Usage

- Launch the application to access your documents
- Articles are stored in `Docs/Articles/`
- Journal entries in `Docs/Journal/`
- Auto-saved drafts in `Docs/Shoin_AutoSave.html`

## License

[Specify license if applicable]


