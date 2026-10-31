#!/usr/bin/env bash
# Install a built Shoin AppImage into ~/Dropbox/Shoin/ safely.
# - Backups go to ~/Dropbox/Development/_backups/appimages/shoin/ (never the app folder).
# - The new image is staged OUTSIDE Dropbox, then atomically renamed into place,
#   so Dropbox never syncs a half-written file and a running copy is not truncated.
# Usage: ./install-appimage.sh [path/to/Shoin.AppImage] [--force]
set -euo pipefail
SRC="${1:-$(dirname "$0")/Shoin.AppImage}"
FORCE="${2:-}"
DEST_DIR="$HOME/Dropbox/Shoin"
DEST="$DEST_DIR/Shoin.AppImage"
BK="$HOME/Dropbox/Development/_backups/appimages/shoin"
STAGE="$HOME/.cache/appimage-staging"
[ -f "$SRC" ] || { echo "no such AppImage: $SRC" >&2; exit 1; }
if [ "$FORCE" != "--force" ] && pgrep -f "mount_Shoin|/Shoin.AppImage|appimage_extracted_.*/usr/bin/Shoin" >/dev/null; then
  echo "Shoin is running; close it first (or pass --force)." >&2; exit 2
fi
mkdir -p "$BK" "$STAGE" "$DEST_DIR"
tmp="$STAGE/Shoin.AppImage.$$.new"
cp -f "$SRC" "$tmp"; chmod +x "$tmp"; sync -f "$tmp" 2>/dev/null || sync
if [ -f "$DEST" ]; then
  cp -p "$DEST" "$BK/Shoin.AppImage.pre-$(date +%Y%m%d-%H%M%S)"
fi
mv -f "$tmp" "$DEST"   # same filesystem -> atomic rename
sha256sum "$DEST"
