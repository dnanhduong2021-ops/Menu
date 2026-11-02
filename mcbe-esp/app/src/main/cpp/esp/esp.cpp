#include "esp.h"
#include "../memory/memory.h"
#include "../offsets/offsets.h"
#include "../utils/logger.h"
#include <cmath>
#include <unordered_set>
#include <string>

namespace esp {

std::vector<BlockEntry> g_blocks;
std::mutex              g_blocksMtx;
bool                    g_enabled = true;

// Blocks to highlight
static const std::unordered_set<std::string> ESP_BLOCKS = {
    "minecraft:respawn_anchor",
    "minecraft:ancient_debris",
    "minecraft:beacon",
    "minecraft:chest",
    "minecraft:trapped_chest",
    "minecraft:ender_chest",
    "minecraft:shulker_box",
    "minecraft:white_shulker_box",
    "minecraft:orange_shulker_box",
    "minecraft:magenta_shulker_box",
    "minecraft:light_blue_shulker_box",
    "minecraft:yellow_shulker_box",
    "minecraft:lime_shulker_box",
    "minecraft:pink_shulker_box",
    "minecraft:gray_shulker_box",
    "minecraft:light_gray_shulker_box",
    "minecraft:cyan_shulker_box",
    "minecraft:purple_shulker_box",
    "minecraft:blue_shulker_box",
    "minecraft:brown_shulker_box",
    "minecraft:green_shulker_box",
    "minecraft:red_shulker_box",
    "minecraft:black_shulker_box",
    "minecraft:spawner",
    "minecraft:diamond_ore",
    "minecraft:deepslate_diamond_ore",
    "minecraft:emerald_ore",
    "minecraft:deepslate_emerald_ore",
    "minecraft:gold_ore",
    "minecraft:deepslate_gold_ore",
    "minecraft:iron_ore",
    "minecraft:deepslate_iron_ore",
    "minecraft:netherite_block",
    "minecraft:crying_obsidian",
    "minecraft:amethyst_cluster",
    "minecraft:sculk_shrieker",
    "minecraft:brewing_stand",
    "minecraft:enchanting_table",
    "minecraft:end_portal_frame",
};

// ── Helpers ──────────────────────────────────────────────────

// FILL: implement getPlayerPos() using offsets::ClientInstance + offsets::Actor_pos
// Returns Vec3(0,0,0) until offsets are filled
static Vec3 getPlayerPos() {
    uintptr_t base   = mem::getBase();
    if (!base) return {};
    // FILL: walk offset chain to LocalPlayer, then read Vec3 at Actor_pos
    // Example (fill offsets):
    // uintptr_t client = mem::readPtr(base + offsets::ClientInstance);
    // uintptr_t player = mem::readPtr(client + offsets::MinecraftGame_player);
    // return mem::read<Vec3>(player + offsets::Actor_pos);
    (void)base;
    return {};
}

// FILL: implement getBlockName() — read BlockSource at given world coord
// Returns empty string for air / unloaded
static std::string getBlockName(int x, int y, int z) {
    // FILL: use BlockSource virtual getBlock(x,y,z) -> BlockLegacy -> getName()
    // Stub returns empty — replace with actual vtable call once offsets known
    (void)x; (void)y; (void)z;
    return "";
}

// ── Main scan ────────────────────────────────────────────────

static int s_tick = 0;

void scan() {
    if (!g_enabled) return;

    // Run full scan every 20 ticks (~1s), lightweight check every tick
    if (++s_tick % 20 != 0) return;

    Vec3 playerPos = getPlayerPos();
    const int RADIUS = 80; // ~5 chunks; set to 256 for 16 chunks

    std::vector<BlockEntry> found;
    found.reserve(64);

    for (int dx = -RADIUS; dx <= RADIUS; dx += 1) {
    for (int dz = -RADIUS; dz <= RADIUS; dz += 1) {
    for (int dy = -64;     dy <= 320;    dy += 1) {
        int bx = (int)playerPos.x + dx;
        int by = dy;
        int bz = (int)playerPos.z + dz;

        std::string name = getBlockName(bx, by, bz);
        if (name.empty()) continue;
        if (ESP_BLOCKS.find(name) == ESP_BLOCKS.end()) continue;

        Vec3 bpos((float)bx + 0.5f, (float)by + 0.5f, (float)bz + 0.5f);
        float dist = (bpos - playerPos).length();

        found.push_back({bpos, name, dist});
        if (found.size() >= 512) goto done; // cap results
    }}}
    done:

    {
        std::lock_guard<std::mutex> lk(g_blocksMtx);
        g_blocks = std::move(found);
    }
    LOGD("ESP scan: %zu blocks found", g_blocks.size());
}

} // namespace esp
