#include "ProfilingManager.h"

long ProfilingManager::DrawCalls = 0;
long long ProfilingManager::TrianglesCount = 0;
long long ProfilingManager::VerticesCount = 0;
long long ProfilingManager::TrianglesCountCurrent = 0;
long long ProfilingManager::VerticesCountCurrent = 0;

double ProfilingManager::s_StartupTimeSec = 0.0;
double ProfilingManager::s_TextureLoadTimeSec = 0.0;
double ProfilingManager::s_FrameTimeSec = 0.0;
double ProfilingManager::s_SkyboxCaptureTimeSec = 0.0;
double ProfilingManager::s_PostProcessTimeSec = 0.0;
double ProfilingManager::s_GUITimeSec = 0.0;

std::chrono::high_resolution_clock::time_point ProfilingManager::s_StartupStartTime = std::chrono::high_resolution_clock::now();
std::chrono::high_resolution_clock::time_point ProfilingManager::s_StartupEndTime = std::chrono::high_resolution_clock::now();

std::chrono::high_resolution_clock::time_point ProfilingManager::s_TextureStartTime = std::chrono::high_resolution_clock::now();
std::chrono::high_resolution_clock::time_point ProfilingManager::s_TextureEndTime = std::chrono::high_resolution_clock::now();

std::chrono::high_resolution_clock::time_point ProfilingManager::s_FrameStartTime = std::chrono::high_resolution_clock::now();
std::chrono::high_resolution_clock::time_point ProfilingManager::s_FrameEndTime = std::chrono::high_resolution_clock::now();

std::chrono::high_resolution_clock::time_point ProfilingManager::s_SkyboxCaptureStartTime = std::chrono::high_resolution_clock::now();
std::chrono::high_resolution_clock::time_point ProfilingManager::s_SkyboxCaptureEndTime = std::chrono::high_resolution_clock::now();

std::chrono::high_resolution_clock::time_point ProfilingManager::s_PostProcessStartTime = std::chrono::high_resolution_clock::now();
std::chrono::high_resolution_clock::time_point ProfilingManager::s_PostProcessEndTime = std::chrono::high_resolution_clock::now();

std::chrono::high_resolution_clock::time_point ProfilingManager::s_GUIStartTime = std::chrono::high_resolution_clock::now();
std::chrono::high_resolution_clock::time_point ProfilingManager::s_GUIEndTime = std::chrono::high_resolution_clock::now();

DWORDLONG ProfilingManager::s_TotalVirtualMem = 0;
DWORDLONG ProfilingManager::s_UsedVirtualMem = 0;
SIZE_T ProfilingManager::s_VirtualMemUsedByProgram = 0;

DWORDLONG ProfilingManager::s_TotalPhysMem = 0;
DWORDLONG ProfilingManager::s_UsedPhysMem = 0;
SIZE_T ProfilingManager::s_PhysMemUsedByProgram = 0;

const float ProfilingManager::GetFramerate()
{
	if (ImGui::GetCurrentContext() == nullptr)
		return 0;

	return (const float)ImGui::GetIO().Framerate;
}

void ProfilingManager::Update()
{
	CalculateMemoryUsage();
	CalculateMemoryUsageByProgram();
}

void ProfilingManager::CalculateMemoryUsage()
{
	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);

	s_TotalVirtualMem = memInfo.ullTotalPageFile;
	s_UsedVirtualMem = memInfo.ullTotalPageFile - memInfo.ullAvailPageFile;

	s_TotalPhysMem = memInfo.ullTotalPhys;
	s_UsedPhysMem = memInfo.ullTotalPhys - memInfo.ullAvailPhys;
}

void ProfilingManager::CalculateMemoryUsageByProgram()
{
	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));

	s_VirtualMemUsedByProgram = pmc.PrivateUsage;
	s_PhysMemUsedByProgram = pmc.WorkingSetSize;
}

void ProfilingManager::RecordStartupTimeStart()
{
	s_StartupStartTime = std::chrono::high_resolution_clock::now();
}

void ProfilingManager::RecordStartupTimeEnd()
{
	s_StartupEndTime = std::chrono::high_resolution_clock::now();
	s_StartupTimeSec = std::chrono::duration_cast<std::chrono::milliseconds>(s_StartupEndTime - s_StartupStartTime).count() / 1000.0;
}

void ProfilingManager::RecordTextureTimeStart()
{
	s_TextureStartTime = std::chrono::high_resolution_clock::now();
}

void ProfilingManager::RecordTextureTimeEnd()
{
	s_TextureEndTime = std::chrono::high_resolution_clock::now();
	s_TextureLoadTimeSec = std::chrono::duration_cast<std::chrono::milliseconds>(s_TextureEndTime - s_TextureStartTime).count() / 1000.0;
}

void ProfilingManager::RecordFrameTimeStart()
{
	s_FrameStartTime = std::chrono::high_resolution_clock::now();
}

void ProfilingManager::RecordFrameTimeEnd()
{
	s_FrameEndTime = std::chrono::high_resolution_clock::now();
	s_FrameTimeSec = std::chrono::duration_cast<std::chrono::milliseconds>(s_FrameEndTime - s_FrameStartTime).count() / 1000.0;
}

void ProfilingManager::RecordSkyboxCaptureTimeStart()
{
	s_SkyboxCaptureStartTime = std::chrono::high_resolution_clock::now();
}

void ProfilingManager::RecordSkyboxCaptureTimeEnd()
{
	s_SkyboxCaptureEndTime = std::chrono::high_resolution_clock::now();
	s_SkyboxCaptureTimeSec = std::chrono::duration_cast<std::chrono::milliseconds>(s_SkyboxCaptureEndTime - s_SkyboxCaptureStartTime).count() / 1000.0;
}

void ProfilingManager::RecordPostProcessTimeStart()
{
	s_PostProcessStartTime = std::chrono::high_resolution_clock::now();
}

void ProfilingManager::RecordPostProcessTimeEnd()
{
	s_PostProcessEndTime = std::chrono::high_resolution_clock::now();
	s_PostProcessTimeSec = std::chrono::duration_cast<std::chrono::milliseconds>(s_PostProcessEndTime - s_PostProcessStartTime).count() / 1000.0;
}

void ProfilingManager::RecordGUITimeStart()
{
	s_GUIStartTime = std::chrono::high_resolution_clock::now();
}

void ProfilingManager::RecordGUITimeEnd()
{
	s_GUIEndTime = std::chrono::high_resolution_clock::now();
	s_GUITimeSec = std::chrono::duration_cast<std::chrono::milliseconds>(s_GUIEndTime - s_GUIStartTime).count() / 1000.0;
}
