#pragma once
#include <cstdint>
#include <cstring>

// KittyMemory-style helpers — in-process read/write
// (same process, no /proc/mem needed — .so is loaded inside Minecraft)

namespace mem {

inline uintptr_t getBase() {
    // Walk /proc/self/maps to find libminecraftpe.so base
    static uintptr_t base = 0;
    if (base) return base;
    FILE* f = fopen("/proc/self/maps", "r");
    if (!f) return 0;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, "libminecraftpe.so") && strstr(line, "r-xp")) {
            base = (uintptr_t)strtoul(line, nullptr, 16);
            break;
        }
    }
    fclose(f);
    return base;
}

template<typename T>
inline T read(uintptr_t addr) {
    T val{};
    memcpy(&val, reinterpret_cast<void*>(addr), sizeof(T));
    return val;
}

template<typename T>
inline void write(uintptr_t addr, T val) {
    memcpy(reinterpret_cast<void*>(addr), &val, sizeof(T));
}

inline uintptr_t readPtr(uintptr_t addr) {
    return read<uintptr_t>(addr);
}

// Dereference chain: base + off[0] -> +off[1] -> ... -> result
inline uintptr_t chain(uintptr_t base, std::initializer_list<uintptr_t> offsets) {
    uintptr_t cur = base;
    for (auto off : offsets) {
        if (!cur) return 0;
        cur = readPtr(cur) + off;
    }
    return cur;
}

} // namespace mem
