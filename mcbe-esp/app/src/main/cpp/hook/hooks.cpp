#include "hooks.h"
#include "../offsets/offsets.h"
#include "../memory/memory.h"
#include "../esp/esp.h"
#include "../render/overlay.h"
#include "../utils/logger.h"

#include <dobby.h>
#include <EGL/egl.h>
#include <dlfcn.h>

namespace hooks {

// ── Game tick hook ────────────────────────────────────────────

using GameTick_t = void(*)(void* self);
static GameTick_t orig_GameTick = nullptr;

static void hk_GameTick(void* self) {
    orig_GameTick(self);
    esp::scan();
}

// ── eglSwapBuffers hook (render) ──────────────────────────────

using eglSwapBuffers_t = EGLBoolean(*)(EGLDisplay, EGLSurface);
static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;

static EGLBoolean hk_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    overlay::render();
    return orig_eglSwapBuffers(dpy, surface);
}

// ── Install ───────────────────────────────────────────────────

bool install() {
    uintptr_t base = mem::getBase();
    if (!base) {
        LOGE("hooks::install — could not find libminecraftpe.so base");
        return false;
    }

    // Hook game tick
    // FILL: offsets::GameTick_fn must be set
    if (offsets::GameTick_fn != 0) {
        uintptr_t target = base + offsets::GameTick_fn;
        DobbyHook(
            reinterpret_cast<void*>(target),
            reinterpret_cast<void*>(hk_GameTick),
            reinterpret_cast<void**>(&orig_GameTick)
        );
        LOGI("GameTick hook installed at 0x%lx", target);
    } else {
        LOGW("GameTick_fn offset is 0 — fill offsets.h");
    }

    // Hook eglSwapBuffers via dlsym (no offset needed)
    void* egl_sym = dlsym(RTLD_DEFAULT, "eglSwapBuffers");
    if (egl_sym) {
        DobbyHook(
            egl_sym,
            reinterpret_cast<void*>(hk_eglSwapBuffers),
            reinterpret_cast<void**>(&orig_eglSwapBuffers)
        );
        LOGI("eglSwapBuffers hook installed");
    } else {
        LOGE("eglSwapBuffers not found via dlsym");
    }

    return true;
}

// ── Remove ────────────────────────────────────────────────────

void remove() {
    uintptr_t base = mem::getBase();
    if (base && offsets::GameTick_fn) {
        DobbyDestroy(reinterpret_cast<void*>(base + offsets::GameTick_fn));
    }
    void* egl_sym = dlsym(RTLD_DEFAULT, "eglSwapBuffers");
    if (egl_sym) DobbyDestroy(egl_sym);
}

} // namespace hooks
