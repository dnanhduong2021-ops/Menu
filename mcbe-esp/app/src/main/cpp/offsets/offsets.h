#pragma once
#include <cstdint>

// ============================================================
// Offsets for libminecraftpe.so — Minecraft Bedrock 1.21.60
// Dump with: Ghidra or IDA Pro on libminecraftpe.so (arm64)
// All offsets relative to libminecraftpe.so base address
// ============================================================

namespace offsets {

// -- ClientInstance --
constexpr uintptr_t ClientInstance          = 0x0; // FILL: dump from IDA/Ghidra 1.21.60

// -- ClientInstance -> MinecraftGame --
constexpr uintptr_t ClientInstance_mcGame   = 0x0; // FILL: dump from IDA/Ghidra 1.21.60

// -- MinecraftGame -> LocalPlayer --
constexpr uintptr_t MinecraftGame_player    = 0x0; // FILL: dump from IDA/Ghidra 1.21.60

// -- Actor (LocalPlayer base) --
constexpr uintptr_t Actor_pos               = 0x0; // FILL: Vec3 position offset in Actor
constexpr uintptr_t Actor_health            = 0x0; // FILL: float health offset in Actor

// -- Level -> ActorList --
constexpr uintptr_t Level_actorList         = 0x0; // FILL: dump from IDA/Ghidra 1.21.60
constexpr uintptr_t ActorList_size          = 0x0; // FILL: std::vector size offset
constexpr uintptr_t ActorList_data          = 0x8; // FILL: std::vector data ptr offset

// -- BlockSource --
constexpr uintptr_t Level_blockSource       = 0x0; // FILL: dump from IDA/Ghidra 1.21.60

// -- LevelChunk --
constexpr uintptr_t BlockSource_getChunk    = 0x0; // FILL: virtual function index or offset

// -- Game tick hook --
// Hook target: ClientInstance::update() or AppPlatform::update()
constexpr uintptr_t GameTick_fn             = 0x0; // FILL: dump from IDA/Ghidra 1.21.60

// -- eglSwapBuffers (for render hook) --
// Usually resolved via dlsym("eglSwapBuffers") — no offset needed
// but if hooked via GOT patch:
constexpr uintptr_t eglSwapBuffers_got      = 0x0; // FILL: optional GOT patch offset

// -- Projection matrix (for world-to-screen) --
constexpr uintptr_t ViewMatrix              = 0x0; // FILL: float[16] view matrix offset
constexpr uintptr_t ProjMatrix              = 0x0; // FILL: float[16] projection matrix offset

// -- Screen dimensions (from ClientInstance or AppPlatform) --
constexpr uintptr_t ScreenWidth             = 0x0; // FILL
constexpr uintptr_t ScreenHeight            = 0x0; // FILL

} // namespace offsets
