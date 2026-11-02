#include "overlay.h"
#include "../esp/esp.h"
#include "../memory/memory.h"
#include "../offsets/offsets.h"
#include "../utils/vec.h"
#include "../utils/logger.h"

#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <vector>
#include <mutex>
#include <cmath>

namespace overlay {

// ── Shader sources ────────────────────────────────────────────

static const char* VERT_SRC = R"(
attribute vec2 aPos;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

static const char* FRAG_SRC = R"(
precision mediump float;
uniform vec4 uColor;
void main() {
    gl_FragColor = uColor;
}
)";

static GLuint g_prog  = 0;
static GLuint g_vbo   = 0;
static bool   g_ready = false;

static GLuint compileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    return s;
}

static void initGL() {
    if (g_ready) return;
    GLuint vs = compileShader(GL_VERTEX_SHADER,   VERT_SRC);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, FRAG_SRC);
    g_prog = glCreateProgram();
    glAttachShader(g_prog, vs);
    glAttachShader(g_prog, fs);
    glLinkProgram(g_prog);
    glDeleteShader(vs);
    glDeleteShader(fs);
    glGenBuffers(1, &g_vbo);
    g_ready = true;
    LOGI("overlay GL init done");
}

// ── World-to-screen ───────────────────────────────────────────

// FILL: read view+proj matrices from offsets::ViewMatrix / ProjMatrix
// multiply world pos through VP matrix, divide by w, map to NDC
// Returns false if behind camera
static bool worldToScreen(const Vec3& world, Vec2& out, int sw, int sh) {
    // FILL: implement matrix read and projection
    // Stub: always returns false until offsets are filled
    (void)world; (void)out; (void)sw; (void)sh;
    return false;
}

// ── Draw a 2D box in NDC coords ───────────────────────────────

static void drawBox(Vec2 center, float halfW, float halfH, float r, float g, float b) {
    // corners in NDC
    float x0 = center.x - halfW, x1 = center.x + halfW;
    float y0 = center.y - halfH, y1 = center.y + halfH;

    float verts[] = {
        x0,y0,  x1,y0,   // bottom
        x1,y0,  x1,y1,   // right
        x1,y1,  x0,y1,   // top
        x0,y1,  x0,y0,   // left
    };

    glUseProgram(g_prog);

    GLint colorLoc = glGetUniformLocation(g_prog, "uColor");
    glUniform4f(colorLoc, r, g, b, 1.0f);

    GLint posLoc = glGetAttribLocation(g_prog, "aPos");
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(posLoc);
    glVertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glDrawArrays(GL_LINES, 0, 8);
    glDisableVertexAttribArray(posLoc);
}

// ── Color by block type ───────────────────────────────────────

static void blockColor(const std::string& name, float& r, float& g, float& b) {
    if (name.find("diamond")        != std::string::npos) { r=0.0f; g=0.8f; b=1.0f; return; }
    if (name.find("emerald")        != std::string::npos) { r=0.0f; g=1.0f; b=0.2f; return; }
    if (name.find("ancient_debris") != std::string::npos) { r=1.0f; g=0.3f; b=0.0f; return; }
    if (name.find("netherite")      != std::string::npos) { r=0.6f; g=0.0f; b=0.6f; return; }
    if (name.find("beacon")         != std::string::npos) { r=0.0f; g=1.0f; b=1.0f; return; }
    if (name.find("chest")          != std::string::npos) { r=1.0f; g=0.8f; b=0.0f; return; }
    if (name.find("shulker")        != std::string::npos) { r=0.8f; g=0.0f; b=0.8f; return; }
    if (name.find("spawner")        != std::string::npos) { r=1.0f; g=0.0f; b=0.0f; return; }
    // default
    r=1.0f; g=1.0f; b=1.0f;
}

// ── Main render ───────────────────────────────────────────────

void render() {
    if (!esp::g_enabled) return;

    initGL();

    // FILL: get screen dimensions from offsets or EGL query
    int sw = 1080, sh = 1920; // placeholder — replace with real screen size

    std::vector<esp::BlockEntry> snapshot;
    {
        std::lock_guard<std::mutex> lk(esp::g_blocksMtx);
        snapshot = esp::g_blocks;
    }

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (auto& entry : snapshot) {
        Vec2 screen;
        if (!worldToScreen(entry.worldPos, screen, sw, sh)) continue;

        float r, g, b;
        blockColor(entry.name, r, g, b);

        // Box size scales with distance — closer = bigger box
        float scale = std::max(0.01f, 40.0f / (entry.distance + 1.0f) / (float)sh);
        drawBox(screen, scale, scale * 1.5f, r, g, b);
    }

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

} // namespace overlay
