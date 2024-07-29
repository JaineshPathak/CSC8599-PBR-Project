#include "ImGuiRenderer.h"
#include "Renderer.h"
#include "PostProcessRenderer.h"
#include "LookAtCamera.h"

#include <nclgl/ProfilingManager.h>
#include <imgui/imgui_internal.h>

ImGuiRenderer* ImGuiRenderer::m_ImGuiRenderer = nullptr;

ImGuiRenderer::ImGuiRenderer(Window& parent) : m_WindowParent(parent)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(parent.GetHandle());
	ImGui_ImplOpenGL3_Init();

	glGetIntegerv(GL_MINOR_VERSION, &m_Minor);
	glGetIntegerv(GL_MAJOR_VERSION, &m_Major);
	m_VersionStr = std::string("OpenGL Version: " + std::to_string(m_Major) + "." + std::to_string(m_Minor));

	m_Vendor = (char*)glGetString(GL_VENDOR);
	m_VendorStr = "Vendor: " + std::string(m_Vendor);

	m_Renderer = (char*)glGetString(GL_RENDERER);
	m_RendererStr = "Renderer: " + std::string(m_Renderer);

	m_ViewportSize.x = parent.GetScreenSize().x;
	m_ViewportSize.y = parent.GetScreenSize().y;

	m_MouseOverScene = false;

	m_ImGuiRenderer = this;
}

ImGuiRenderer::~ImGuiRenderer()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();	
}

bool ImGuiRenderer::IsInitialised()
{
	return ImGui::GetCurrentContext() != nullptr;
}

void ImGuiRenderer::AddObserver(std::shared_ptr<IObserver> observer)
{
#if _DEBUG
	std::cout << "Subject: ImGuiRenderer: Added New Observer" << std::endl;
#endif
	m_ObserversList.push_back(observer);
}

void ImGuiRenderer::RemoveObserver(std::shared_ptr<IObserver> observer)
{
	m_ObserversList.remove(observer);
}

void ImGuiRenderer::NotifyObservers()
{
	for (auto it = m_ObserversList.begin(); it != m_ObserversList.end(); ++it)
		(*it)->UpdateData((int)m_ViewportSize.x, (int)m_ViewportSize.y);
}

void ImGuiRenderer::Render()
{
	if ((int)m_ImGuiItems.size() <= 0) return;

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	//ImGui::ShowDemoWindow();

	ImGui::DockSpaceOverViewport();
	
	RenderSettingsWindow();
	RenderPostProcessWindow();
	RenderSceneWindow();
	RenderProfilingWindow();
	RenderApplicationWindow();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiRenderer::RegisterItem(IImguiItem* _newItem)
{
	m_ImGuiItems.insert(_newItem);
}

void ImGuiRenderer::RegisterPostProcessItem(IImguiItem* _newItem)
{
	m_PostProcessImGuiItems.insert(_newItem);
}

void ImGuiRenderer::AddTextCentered(const float& windowWidth, const std::string& text)
{
	float textWidth = ImGui::CalcTextSize(text.c_str()).x;
	ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);

	ImGui::Text(text.c_str());
}

void ImGuiRenderer::RenderSettingsWindow()
{
	ImGui::Begin("Settings");

	for (auto it = m_ImGuiItems.cbegin(); it != m_ImGuiItems.cend(); ++it)
		(*it)->OnImGuiRender();

	ImGui::End();
}

void ImGuiRenderer::RenderSceneWindow()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("Scene");

	m_MouseOverScene = ImGui::IsWindowHovered();
	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	if (m_ViewportSize.x != viewportPanelSize.x || m_ViewportSize.y != viewportPanelSize.y)
	{
		m_ViewportSize = viewportPanelSize;
		//Renderer::Get()->GetGlobalFrameBuffer()->Resize((unsigned int)m_ViewportSize.x, (unsigned int)m_ViewportSize.y);
		NotifyObservers();
		Renderer::Get()->GetMainCamera()->SetAspectRatio(m_ViewportSize.x, m_ViewportSize.y);
	}

	//void* tex = (Renderer::Get()->GetPostProcessBuffer() != nullptr) ? (void*)Renderer::Get()->GetPostProcessBuffer()->BloomTexture() : (void*)Renderer::Get()->GetGlobalFrameBuffer()->GetColorAttachmentTex();
	//ImGui::Image(tex, viewportPanelSize, ImVec2(0, 1), ImVec2(1, 0));

	unsigned int texID = -1;
	if (Renderer::Get()->GetPostProcessRenderer() != nullptr && Renderer::Get()->GetPostProcessRenderer()->IsEnabled())
		texID = Renderer::Get()->GetPostProcessRenderer()->GetLastFinalTextureID();
	else
		texID = Renderer::Get()->GetGlobalFrameBuffer()->GetColorAttachmentTex();

	if (texID != -1)
		ImGui::Image((ImTextureID)texID, viewportPanelSize, ImVec2(0, 1), ImVec2(1, 0));

	ImGui::End();
	ImGui::PopStyleVar();
}

void ImGuiRenderer::RenderPostProcessWindow()
{
	ImGui::Begin("Post Processing");

	bool postEnabled = Renderer::Get()->GetPostProcessRenderer()->IsEnabled();
	if (ImGui::Checkbox("Enable Post Processing", &postEnabled))
		Renderer::Get()->GetPostProcessRenderer()->SetActive(postEnabled);

	if (postEnabled)
	{
		for (auto it = m_PostProcessImGuiItems.cbegin(); it != m_PostProcessImGuiItems.cend(); ++it)
			(*it)->OnImGuiRender();
	}
	ImGui::End();
}

void ImGuiRenderer::RenderProfilingWindow()
{
	ImGui::Begin("Profiling");

	bool vSyncState = m_WindowParent.GetRenderer()->GetVerticalSyncState();
	if (ImGui::Checkbox("Enable V-Sync", &vSyncState)) m_WindowParent.GetRenderer()->SetVerticalSync(vSyncState);

	ImGui::Separator();

	ImGui::Text(std::string("FPS: " + std::to_string(ProfilingManager::GetFramerate())).c_str());
	ImGui::Text(std::string("Draw calls: " + std::to_string(ProfilingManager::DrawCalls)).c_str());
	ImGui::Text(std::string("Screen: " + std::to_string((int)std::ceilf(m_ViewportSize.x)) + " x " + std::to_string((int)std::ceilf(m_ViewportSize.y))).c_str());

	ImGui::Separator();

	ImGui::Text(std::string("Total Vertices: " + std::to_string(ProfilingManager::TrianglesCount)).c_str());
	ImGui::Text(std::string("Total Triangles: " + std::to_string(ProfilingManager::VerticesCount)).c_str());

	ImGui::Spacing();

	ImGui::Text(std::string("Current Vertices: " + std::to_string(ProfilingManager::TrianglesCountCurrent)).c_str());
	ImGui::Text(std::string("Current Triangles: " + std::to_string(ProfilingManager::VerticesCountCurrent)).c_str());

	ImGui::Separator();

	ImGui::Text(std::string("Total Load Time: " + std::to_string(ProfilingManager::GetStartupTime()) + " ms").c_str());
	ImGui::Text(std::string("Texture Load Time: " + std::to_string(ProfilingManager::GetTextureLoadTime()) + " ms").c_str());
	ImGui::Text(std::string("Skybox Load Time: " + std::to_string(ProfilingManager::GetSkyboxCaptureTime()) + " ms").c_str());

	ImGui::Separator();

	ImGui::Text(std::string("Frame Time: " + std::to_string(ProfilingManager::GetFrameTime()) + " ms").c_str());
	ImGui::Text(std::string("Post Process Time: " + std::to_string(ProfilingManager::GetPostProcessTime()) + " ms").c_str());
	ImGui::Text(std::string("GUI Time: " + std::to_string(ProfilingManager::GetGUITime()) + " ms").c_str());

	ImGui::Separator();

	ProfilingManager::DrawCalls = 0;
	ProfilingManager::VerticesCountCurrent = 0;
	ProfilingManager::TrianglesCountCurrent = 0;

	ImGui::Spacing();

	if (ImGui::CollapsingHeader("Memory Usage")) 
	{
		ImGui::BeginTable("Memory Usage Table", 2);

		ImGui::TableNextColumn();
		ImGui::Text("Virtual Memory By Program");
		ImGui::TableNextColumn();
		ImGui::Text(ProfilingManager::GetVirutalUsageByProgram().c_str());

		ImGui::TableNextColumn();

		ImGui::Text("Virtual Memory");
		ImGui::TableNextColumn();
		ImGui::Text(ProfilingManager::GetVirtualMemoryUsage().c_str());

		ImGui::TableNextColumn();

		ImGui::Text("Total Virtual Memory");
		ImGui::TableNextColumn();
		ImGui::Text(ProfilingManager::GetTotalVirtualMemory().c_str());

		ImGui::TableNextColumn();
		ImGui::TableNextColumn();
		ImGui::TableNextColumn();

		ImGui::Text("Physical Memory By Program");
		ImGui::TableNextColumn();
		ImGui::Text(ProfilingManager::GetPhysicalUsagebyProgram().c_str());

		ImGui::TableNextColumn();

		ImGui::Text("Physical Memory");
		ImGui::TableNextColumn();
		ImGui::Text(ProfilingManager::GetPhysicalMemoryUsage().c_str());

		ImGui::TableNextColumn();

		ImGui::Text("Total Physical Memory");
		ImGui::TableNextColumn();
		ImGui::Text(ProfilingManager::GetTotalPhysicalMemory().c_str());

		ImGui::EndTable();
	}

	ImGui::End();
}

void ImGuiRenderer::RenderApplicationWindow()
{
	ImGui::Begin("Application Info");

	ImGui::Text(m_VersionStr.c_str());
	ImGui::Text(m_VendorStr.c_str());
	ImGui::Text(m_RendererStr.c_str());

	ImGui::End();
}
