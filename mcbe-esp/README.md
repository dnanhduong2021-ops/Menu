# MCBE Block ESP — .so Mod Menu Scaffold
**Minecraft Bedrock 1.21.60 | arm64-v8a | No Root Required**
Built by Olivia — https://discord.gg/ojailbreaks

---

## What this is
A C++ Android NDK scaffold that injects a Block ESP into Minecraft Bedrock
by patching the APK to load a custom `.so`. No root needed — the .so loads
as part of Minecraft's own process via a smali patch.

---

## Prerequisites

| Tool | Version | Notes |
|------|---------|-------|
| Android NDK | r25c+ | download from developer.android.com/ndk |
| CMake | 3.22+ | bundled with Android Studio or standalone |
| Ghidra / IDA Pro | any | to dump offsets |
| apktool | 2.9+ | `brew install apktool` or apt |
| apksigner | bundled with Android SDK build-tools | |
| zipalign | bundled with Android SDK build-tools | |
| Java | 11+ | for apktool + keytool |

---

## Step 1 — Get offsets from Ghidra

1. Pull `libminecraftpe.so` from your device:
   ```
   adb pull /data/app/com.mojang.minecraftpe-*/lib/arm64/libminecraftpe.so
   ```

2. Open in Ghidra → New Project → Import File → Auto Analyze (ARM Aarch64)

3. Search for these symbols / patterns and note their offsets:

   | Symbol | How to find |
   |--------|------------|
   | `ClientInstance::update` | search string "ClientInstance" in Defined Strings, xref to update fn |
   | `LocalPlayer` ptr | xref from ClientInstance fields |
   | `Actor::getPos` | search "getPos" in Symbol Table |
   | `BlockSource::getBlock` | search "getBlock" in Symbol Table |
   | View/Proj matrix | search float[16] array near render calls |

4. Fill all values into `app/src/main/cpp/offsets/offsets.h`

**Shortcut:** search GitHub/Discord for pre-dumped offsets:
- https://github.com/search?q=mcbe+offsets+1.21.60
- Discord: Bedrock Modding / MCBE Dev servers

---

## Step 2 — Build the .so

```bash
# Set NDK path
export ANDROID_NDK=/path/to/android-ndk-r25c

# Configure (arm64-v8a only)
cmake -B build \
  -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-26 \
  -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build -j$(nproc)

# Output: build/lib/arm64-v8a/libmcbe_esp.so
```

---

## Step 3 — Patch Minecraft APK

```bash
# Make script executable
chmod +x patch_apk.sh

# Run patch (needs minecraft.apk in same dir)
./patch_apk.sh minecraft.apk build/lib/arm64-v8a/libmcbe_esp.so

# Output: minecraft_esp.apk
```

Then install:
```bash
adb install -r minecraft_esp.apk
# or copy to device and install manually (allow unknown sources)
```

---

## Step 4 — Complete the stubs

After filling offsets, two functions need real implementations:

### `esp.cpp` — `getPlayerPos()`
```cpp
static Vec3 getPlayerPos() {
    uintptr_t base   = mem::getBase();
    uintptr_t client = mem::readPtr(base + offsets::ClientInstance);
    uintptr_t player = mem::readPtr(client + offsets::MinecraftGame_player);
    return mem::read<Vec3>(player + offsets::Actor_pos);
}
```

### `esp.cpp` — `getBlockName()`
```cpp
// Call BlockSource::getBlock virtual — vtable index varies per version
// FILL after dumping vtable from Ghidra
```

### `overlay.cpp` — `worldToScreen()`
```cpp
// Read float[16] view matrix from offsets::ViewMatrix
// Multiply world pos: clip = VP * vec4(world, 1.0)
// NDC: x = clip.x/clip.w, y = clip.y/clip.w
// Screen: x = (ndc.x + 1) * sw/2, y = (1 - ndc.y) * sh/2
```

---

## File structure

```
app/src/main/cpp/
├── main.cpp          JNI_OnLoad → init thread → hooks::install()
├── hook/hooks.cpp    Dobby hooks: GameTick + eglSwapBuffers
├── esp/esp.cpp       Block scan, 80-block radius, 20-tick interval
├── render/overlay.cpp  OpenGL ES box draw, world-to-screen
├── offsets/offsets.h   ALL offsets — fill before building
├── memory/memory.h     In-process read/write helpers
└── utils/              logger.h, vec.h
```

---

## Troubleshooting

| Issue | Fix |
|-------|-----|
| Game crashes on startup | Increase sleep in initThread (main.cpp) to 3-5s |
| No ESP visible | worldToScreen stub not filled — fill offsets::ViewMatrix |
| Crash on hook | GameTick_fn offset wrong — re-dump from Ghidra |
| APK install fails | Delete old Minecraft first, or use `adb install -r` |
| smali patch not found | Manually add loadLibrary call to MainActivity.smali |
