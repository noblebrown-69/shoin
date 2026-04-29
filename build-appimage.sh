#!/bin/bash
set -e

echo "=== Shoin AppImage Builder ==="
echo "Starting build process..."

echo "Running rebuild.sh to get latest binary..."
./rebuild.sh

# Download deployment tools only once
if [ ! -f linuxdeploy-x86_64.AppImage ]; then
    echo "Downloading linuxdeploy-x86_64.AppImage..."
    wget -q --show-progress https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
fi
if [ ! -f linuxdeploy-plugin-qt-x86_64.AppImage ]; then
    echo "Downloading linuxdeploy-plugin-qt-x86_64.AppImage..."
    wget -q --show-progress https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
fi

echo "Making deployment tools executable..."
chmod +x linuxdeploy-x86_64.AppImage linuxdeploy-plugin-qt-x86_64.AppImage

# Install Hunspell if not present (one-time)
if ! dpkg -s libhunspell-dev >/dev/null 2>&1 || ! dpkg -s hunspell-en-us >/dev/null 2>&1; then
    echo "Installing Hunspell..."
    sudo apt-get update -qq
    sudo apt-get install -y --no-install-recommends libhunspell-dev hunspell-en-us
else
    echo "Hunspell already installed."
fi

echo "Cleaning up previous AppDir..."
rm -rf AppDir

mkdir -p AppDir/usr/share/icons/hicolor/512x512/apps
cp shoin.png AppDir/usr/share/icons/hicolor/512x512/apps/shoin.png 2>/dev/null || true

echo "Bundling Qt application + Hunspell into AppImage..."
./linuxdeploy-x86_64.AppImage --appdir AppDir \
    --plugin qt \
    --executable build/Shoin \
    --desktop-file Shoin.desktop \
    --library /usr/lib/x86_64-linux-gnu/libhunspell-1.7.so.0 \
    --output appimage

# Manually copy dictionary files into the AppImage (Hunspell needs them)
mkdir -p AppDir/usr/share/hunspell
cp /usr/share/hunspell/en_US.aff AppDir/usr/share/hunspell/ 2>/dev/null || true
cp /usr/share/hunspell/en_US.dic AppDir/usr/share/hunspell/ 2>/dev/null || true

echo "Renaming to Shoin.AppImage..."
mv Shoin-x86_64.AppImage Shoin.AppImage 2>/dev/null || true

echo "✅ Success! Shoin.AppImage is ready with Hunspell spell checking."
ls -lh Shoin.AppImage
echo "Copy this to your Dropbox folder."