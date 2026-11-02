#include <jni.h>
#include <pthread.h>
#include "hook/hooks.h"
#include "utils/logger.h"

// ── Init thread ───────────────────────────────────────────────
// Runs after Minecraft has loaded its own .so files.
// Gives libminecraftpe.so time to map before we search for its base.

static void* initThread(void*) {
    LOGI("mcbe_esp: init thread started");

    // Wait for libminecraftpe.so to be mapped
    // Simple approach: sleep 2s — adjust if game crashes on startup
    struct timespec ts = {2, 0};
    nanosleep(&ts, nullptr);

    if (!hooks::install()) {
        LOGE("mcbe_esp: hook installation failed");
    } else {
        LOGI("mcbe_esp: hooks active");
    }
    return nullptr;
}

// ── JNI_OnLoad ────────────────────────────────────────────────
// Called by System.loadLibrary("mcbe_esp") injected via smali patch.

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    (void)vm; (void)reserved;
    LOGI("mcbe_esp: JNI_OnLoad");

    pthread_t t;
    pthread_create(&t, nullptr, initThread, nullptr);
    pthread_detach(t);

    return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* vm, void* reserved) {
    (void)vm; (void)reserved;
    hooks::remove();
    LOGI("mcbe_esp: unloaded");
}
