#pragma once

// Resolve these C exports from the already-loaded qui.dll with GetProcAddress.
// Do not link qui.lib or call LoadLibrary: QUI remains an optional dependency.
#ifdef QUI_BUILD_API
#define QUI_API extern "C" __declspec(dllexport)
#else
#define QUI_API extern "C"
#endif

// True when the request was queued. False until Plugin Explorer is registered
// (kDataLoaded), when it is disabled in QUI's config, or if UI tasks are unavailable.
// Opening is asynchronous; IsMenuOpen reports actual, not requested, visibility.
QUI_API bool QUI_Open();

// Requests a full Plugin Explorer close, not Back to its parent list.
// Safe when already closed; supersedes an earlier pending API open request.
QUI_API void QUI_Close();

// Latest visibility reported by Skyrim's PluginExplorerMenu open/close events.
QUI_API bool QUI_IsMenuOpen();

// Controls only QUI's native opening/toggle hotkey. Does not consume the key,
// change TOML files, disable API control, or change Escape/Tab/B navigation.
// Runtime-only; defaults to enabled on each game-process start.
QUI_API void QUI_SetHotkeyEnabled(bool a_enabled);

#undef QUI_API
