#pragma once

namespace hooks {

// Install all hooks via Dobby. Called once from JNI_OnLoad.
bool install();

// Remove all hooks on unload.
void remove();

} // namespace hooks
