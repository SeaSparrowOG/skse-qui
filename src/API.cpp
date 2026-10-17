#define QUI_BUILD_API
#include "QUIAPI.h"
#include "API.hpp"
#include "Core/Menu/PluginExplorerMenu.hpp"

#include <atomic>
#include <cstdint>
#include <mutex>

namespace Core::API
{

	namespace
	{

		std::atomic<bool> menuReady{ false };
		std::atomic<bool> menuOpen{ false };
		std::atomic<bool> hotkeyEnabled{ true };
		std::mutex        requestMutex;
		std::uint64_t     requestSequence{ 0 };

		bool RequestMenuState(bool a_open)
		{
			if (!menuReady.load(std::memory_order_acquire))
				return false;
			const auto tasks = SKSE::GetTaskInterface();
			if (!tasks)
				return false;

			std::scoped_lock lock(requestMutex);
			const auto       sequence = ++requestSequence;
			tasks->AddUITask([a_open, sequence] {
				std::scoped_lock taskLock(requestMutex);
				if (sequence != requestSequence)
					return;
				if (!RE::UI::GetSingleton() || !RE::UIMessageQueue::GetSingleton())
					return;

				// Always enqueue the requested state: a previous opposite message
				// can still be pending even when live visibility has not changed yet.
				if (a_open) {
					Menu::PluginExplorerMenu::Open();
				} else {
					// A forced close must not trigger the Container handoff in the
					// menu-close event handler after the player selected a form type.
					Menu::PluginExplorerMenu::SetFocus(Menu::PluginExplorerMenu::Focus::Plugin);
					Menu::PluginExplorerMenu::Close();
				}
			});
			return true;
		}
	}

	void SetMenuReady()
	{
		menuReady.store(true, std::memory_order_release);
	}

	void SetMenuOpen(bool a_open)
	{
		menuOpen.store(a_open, std::memory_order_release);
	}

	bool IsHotkeyEnabled()
	{
		return hotkeyEnabled.load(std::memory_order_acquire);
	}
}

bool QUI_Open()
{
	return Core::API::RequestMenuState(true);
}

void QUI_Close()
{
	Core::API::RequestMenuState(false);
}

bool QUI_IsMenuOpen()
{
	return Core::API::menuOpen.load(std::memory_order_acquire);
}

void QUI_SetHotkeyEnabled(bool a_enabled)
{
	Core::API::hotkeyEnabled.store(a_enabled, std::memory_order_release);
}
