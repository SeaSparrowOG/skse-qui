#pragma once

namespace Core::API
{

	// Called only after Plugin Explorer's factory and event handler are installed.
	void SetMenuReady();
	void SetMenuOpen(bool a_open);
	bool IsHotkeyEnabled();
}
