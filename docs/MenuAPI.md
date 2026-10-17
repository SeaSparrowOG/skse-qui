# Plugin Explorer Runtime API

QUI exposes four C exports from `qui.dll` for external launchers and hotkey
managers. The declarations are in [include/QUIAPI.h](../include/QUIAPI.h).
There is no dependency on Risa or any other consumer.

```cpp
extern "C" bool QUI_Open();
extern "C" void QUI_Close();
extern "C" bool QUI_IsMenuOpen();
extern "C" void QUI_SetHotkeyEnabled(bool enabled);
```

`QUI_Open` queues an opening request and returns `true` if accepted. It returns
`false` before QUI registers Plugin Explorer at SKSE `kDataLoaded`, when Plugin
Explorer is disabled in QUI's configuration, or when SKSE UI tasks are unavailable.
It does not inherit the native opener's gameplay/paused checks, so a launcher
can open it during its own menu handoff.

`QUI_Close` requests a full close, including from the form-type list. It is safe
when already closed and supersedes an earlier pending API open request. It does
not open the selected form-type container as a side effect.

Both requests run through SKSE's UI task queue and QUI's normal show/hide messages.
The latest API request wins when multiple requests are waiting. Visibility does
not necessarily change until a later UI frame.

`QUI_IsMenuOpen` reports the latest actual Plugin Explorer visibility from Skyrim's
menu events, not a pending request. It also reflects native opens and closes.
It refers to Plugin Explorer itself, not the item container opened after selection.

`QUI_SetHotkeyEnabled(false)` disables only QUI's native Plugin Explorer toggle
listener. The key still reaches Skyrim and other plugins normally, and API opening
remains available. Escape, Tab, right-click, and B continue navigating Back normally.
Call it with `true` to restore the player's configured toggle. The flag defaults
to `true` on each process start and never changes either TOML file or a save.

The exports are callable from external plugin threads: request submission is
serialized, visibility and hotkey queries use atomics, and Skyrim UI work is queued.
Keep calls short; do not wait synchronously for the menu to appear.

## Optional Consumer Example

Resolve the exports after SKSE has loaded the plugins, for example at `kPostLoad`.
Only start opening menus after `kDataLoaded`. Do not use `LoadLibrary` or link
`qui.lib`: SKSE owns the plugin lifetime. Missing exports on older QUI versions
should disable only the integration, not your mod.

```cpp
#include <Windows.h>

struct QUIFunctions
{
    bool (*open)(){};
    void (*close)(){};
    bool (*isOpen)(){};
    void (*setHotkeyEnabled)(bool){};
};

QUIFunctions ResolveQUI()
{
    const auto module = GetModuleHandleW(L"qui.dll");
    if (!module) return {};

    QUIFunctions api{
        reinterpret_cast<bool (*)()>(GetProcAddress(module, "QUI_Open")),
        reinterpret_cast<void (*)()>(GetProcAddress(module, "QUI_Close")),
        reinterpret_cast<bool (*)()>(GetProcAddress(module, "QUI_IsMenuOpen")),
        reinterpret_cast<void (*)(bool)>(GetProcAddress(module, "QUI_SetHotkeyEnabled"))
    };
    if (!api.open || !api.close || !api.isOpen || !api.setHotkeyEnabled)
        return {};
    return api;
}
```

Disable the native listener only after all required exports resolve, and restore
it when your launcher stops managing QUI. Do not treat a successful `open()` as
immediate visibility or call it repeatedly while waiting for `isOpen()`.

## Standalone Tests

The tests compile the production API implementation and native-toggle dispatch
against fake SKSE tasks/menu messages; they do not start Skyrim or require its SDK.

```text
cmake -S tests -B build-api-tests
cmake --build build-api-tests --config Release
ctest --test-dir build-api-tests -C Release --output-on-failure
```

In-game verification is still needed for the actual Scaleform/UI lifecycle.
