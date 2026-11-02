#pragma once
#include <vector>
#include <string>
#include <mutex>
#include "../utils/vec.h"

namespace esp {

struct BlockEntry {
    Vec3        worldPos;
    std::string name;
    float       distance;
};

// Thread-safe result buffer — written by scan(), read by overlay
extern std::vector<BlockEntry> g_blocks;
extern std::mutex              g_blocksMtx;
extern bool                    g_enabled;

// Called from game tick hook
void scan();

} // namespace esp
