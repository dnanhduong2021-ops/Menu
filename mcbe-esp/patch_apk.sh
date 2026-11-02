#!/usr/bin/env bash
# ================================================================
# patch_apk.sh — inject libmcbe_esp.so into Minecraft Bedrock APK
# Requirements: apktool, apksigner, zipalign, Java 11+
# ================================================================
set -euo pipefail

MINECRAFT_APK="${1:-minecraft.apk}"
SO_PATH="${2:-build/lib/arm64-v8a/libmcbe_esp.so}"
OUT_APK="minecraft_esp.apk"
WORK_DIR="mc_patched"
KEYSTORE="esp_key.jks"

if [[ ! -f "$MINECRAFT_APK" ]]; then
    echo "Usage: $0 <minecraft.apk> <libmcbe_esp.so>"
    exit 1
fi

if [[ ! -f "$SO_PATH" ]]; then
    echo "ERROR: .so not found at $SO_PATH — build it first"
    exit 1
fi

echo "[1/6] Decoding APK with apktool..."
apktool d -f "$MINECRAFT_APK" -o "$WORK_DIR"

echo "[2/6] Copying .so into APK lib dir..."
mkdir -p "$WORK_DIR/lib/arm64-v8a"
cp "$SO_PATH" "$WORK_DIR/lib/arm64-v8a/libmcbe_esp.so"

# ----------------------------------------------------------------
# [3/6] Inject System.loadLibrary("mcbe_esp") into MainActivity
# ----------------------------------------------------------------
echo "[3/6] Patching smali to load libmcbe_esp.so..."

# Find MainActivity smali — path varies by Minecraft version
MAIN_SMALI=$(find "$WORK_DIR/smali"* -name "MainActivity.smali" 2>/dev/null | head -1)
if [[ -z "$MAIN_SMALI" ]]; then
    # Fallback: find any Activity with onCreate
    MAIN_SMALI=$(grep -rl "onCreate\|onStart" "$WORK_DIR/smali"* 2>/dev/null | grep -i "main\|launch\|nativeactivity" | head -1)
fi

if [[ -z "$MAIN_SMALI" ]]; then
    echo "WARNING: Could not find MainActivity smali — inject manually"
    echo "  Add this to any Activity onCreate, before super.onCreate:"
    echo '  const-string v0, "mcbe_esp"'
    echo '  invoke-static {v0}, Ljava/lang/System;->loadLibrary(Ljava/lang/String;)V'
else
    echo "  Patching: $MAIN_SMALI"
    # Insert after ".method protected onCreate" line, before first return
    python3 - "$MAIN_SMALI" <<'PYEOF'
import sys, re

path = sys.argv[1]
with open(path, 'r') as f:
    content = f.read()

INJECT = '''
    # --- mcbe_esp inject ---
    const-string v0, "mcbe_esp"
    invoke-static {v0}, Ljava/lang/System;->loadLibrary(Ljava/lang/String;)V
    # -----------------------
'''

# Insert after first "invoke-super" in onCreate method
pattern = r'(\.method protected onCreate\(.*?\n(?:.*?\n)*?)(    invoke-super)'
match = re.search(pattern, content)
if match:
    insert_pos = match.start(2)
    content = content[:insert_pos] + INJECT + content[insert_pos:]
    with open(path, 'w') as f:
        f.write(content)
    print("  Smali patched successfully")
else:
    print("  WARNING: Could not auto-patch — patch manually")
    print("  Add to onCreate before super call:")
    print('  const-string v0, "mcbe_esp"')
    print('  invoke-static {v0}, Ljava/lang/System;->loadLibrary(Ljava/lang/String;)V')
PYEOF
fi

echo "[4/6] Rebuilding APK..."
apktool b "$WORK_DIR" -o "${OUT_APK}.unsigned"

echo "[5/6] Generating keystore if needed..."
if [[ ! -f "$KEYSTORE" ]]; then
    keytool -genkey -v \
        -keystore "$KEYSTORE" \
        -alias esp \
        -keyalg RSA \
        -keysize 2048 \
        -validity 10000 \
        -storepass esppass \
        -keypass esppass \
        -dname "CN=ESP, OU=ESP, O=ESP, L=ESP, ST=ESP, C=US"
fi

echo "[6/6] Zipalign + signing..."
zipalign -v 4 "${OUT_APK}.unsigned" "${OUT_APK}.aligned"
apksigner sign \
    --ks "$KEYSTORE" \
    --ks-pass pass:esppass \
    --key-pass pass:esppass \
    --out "$OUT_APK" \
    "${OUT_APK}.aligned"

rm -f "${OUT_APK}.unsigned" "${OUT_APK}.aligned"

echo ""
echo "Done! Patched APK: $OUT_APK"
echo "Install: adb install -r $OUT_APK"
echo "  (or transfer to device and install, allow unknown sources)"
