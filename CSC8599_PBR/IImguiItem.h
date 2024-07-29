#pragma once
#include <imgui/imgui.h>

class IImguiItem
{
public:
	virtual void OnImGuiRender() = 0;
};