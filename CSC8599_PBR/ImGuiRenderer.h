#pragma once

#include "IImguiItem.h"

#include <nclgl/Window.h>
#include <nclgl/FrameBuffer.h>
#include <nclgl/ISubject.h>

#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_win32.h>

#include <unordered_set>

class ImGuiRenderer : public ISubject
{
public:
	static ImGuiRenderer* Get() { return m_ImGuiRenderer; }
	
	ImGuiRenderer(Window& parent);
	~ImGuiRenderer();

	bool IsInitialised();

	//Observer Frame Buffers
	virtual void AddObserver(std::shared_ptr<IObserver> observer);
	virtual void RemoveObserver(std::shared_ptr<IObserver> observer);
	virtual void NotifyObservers();

	void Render();

	void RegisterItem(IImguiItem* _newItem);
	void RegisterPostProcessItem(IImguiItem* _newItem);

	const bool IsMouseOverScene() const { return m_MouseOverScene; }

	static void AddTextCentered(const float& windowWidth, const std::string& text);

private:
	void RenderSettingsWindow();
	void RenderSceneWindow();
	void RenderPostProcessWindow();
	void RenderProfilingWindow();
	void RenderApplicationWindow();	

protected:
	static ImGuiRenderer* m_ImGuiRenderer;
	std::unordered_set<IImguiItem*> m_ImGuiItems;
	std::unordered_set<IImguiItem*> m_PostProcessImGuiItems;

	std::list<std::shared_ptr<IObserver>> m_ObserversList;

	ImVec2 m_ViewportSize;
	bool m_MouseOverScene;

private:
	Window& m_WindowParent;

	int m_Minor, m_Major;
	std::string m_VersionStr;

	char* m_Vendor;
	char* m_Renderer;
	std::string m_VendorStr, m_RendererStr;
};