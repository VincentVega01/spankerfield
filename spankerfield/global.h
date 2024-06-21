#pragma once
#include "SDK/sdk.h"

namespace big
{
	class globals
	{
	public:
		HWND g_hwnd{};
		HANDLE g_main_thread{};
		DWORD g_main_thread_id{};
		HINSTANCE g_hmodule{};

		Matrix g_viewproj{};
		bool g_running{ true };
		int g_width{ 1920 };
		int g_height{ 1080 };
		
		bool g_should_draw;
		bool g_fairfight;
		bool g_punkbuster;
		int screenshots_ff;
		int screenshots_pb;

		Vector3 g_pred_aim_point{ 0.0f, 0.0f, 0.0f};
		bool g_hasPredictedAimPoint{ false };

		bool g_obs;
	};

	inline globals g_globals;
}
