#pragma once
#include <imgui/imgui_internal.h>

#include <Windows.h>
#include <psapi.h>
#include <chrono>
#include <string>

class ProfilingManager
{
public:
	static void RecordStartupTimeStart();
	static void RecordStartupTimeEnd();

	static void RecordTextureTimeStart();
	static void RecordTextureTimeEnd();

	static void RecordFrameTimeStart();
	static void RecordFrameTimeEnd();

	static void RecordSkyboxCaptureTimeStart();
	static void RecordSkyboxCaptureTimeEnd();

	static void RecordPostProcessTimeStart();
	static void RecordPostProcessTimeEnd();

	static void RecordGUITimeStart();
	static void RecordGUITimeEnd();

	static const float GetFramerate();
	static const double GetStartupTime() { return s_StartupTimeSec; }
	static const double GetTextureLoadTime() { return s_TextureLoadTimeSec; }
	static const double GetFrameTime() { return s_FrameTimeSec; }
	static const double GetSkyboxCaptureTime() { return s_SkyboxCaptureTimeSec; }
	static const double GetPostProcessTime() { return s_PostProcessTimeSec; }
	static const double GetGUITime() { return s_GUITimeSec; }

	static const std::string GetVirtualMemoryUsage() { return ConvertMemoryUsage(s_UsedVirtualMem); }
	static const std::string GetVirutalUsageByProgram() { return ConvertMemoryUsage(s_VirtualMemUsedByProgram); }
	static const std::string GetTotalVirtualMemory() { return ConvertMemoryUsage(s_TotalVirtualMem); }
	static const std::string GetPhysicalMemoryUsage() { return ConvertMemoryUsage(s_UsedPhysMem); }
	static const std::string GetPhysicalUsagebyProgram() { return ConvertMemoryUsage(s_PhysMemUsedByProgram); }
	static const std::string GetTotalPhysicalMemory() { return ConvertMemoryUsage(s_TotalPhysMem); }
	
	static void Update();

	static long DrawCalls;
	static long long TrianglesCount;
	static long long VerticesCount;

	static long long TrianglesCountCurrent;
	static long long VerticesCountCurrent;

private:
	static double s_StartupTimeSec;
	static double s_TextureLoadTimeSec;
	static double s_FrameTimeSec;
	static double s_SkyboxCaptureTimeSec;
	static double s_PostProcessTimeSec;
	static double s_GUITimeSec;

	static const int BYTE_TO_MB = 1048576;
	static std::string ConvertMemoryUsage(DWORDLONG a) { return std::to_string(a / BYTE_TO_MB) + " MB"; }

	static std::chrono::high_resolution_clock::time_point s_StartupStartTime, s_StartupEndTime;
	static std::chrono::high_resolution_clock::time_point s_TextureStartTime, s_TextureEndTime;
	static std::chrono::high_resolution_clock::time_point s_FrameStartTime, s_FrameEndTime;
	static std::chrono::high_resolution_clock::time_point s_SkyboxCaptureStartTime, s_SkyboxCaptureEndTime;
	static std::chrono::high_resolution_clock::time_point s_PostProcessStartTime, s_PostProcessEndTime;
	static std::chrono::high_resolution_clock::time_point s_GUIStartTime, s_GUIEndTime;
	
	static void CalculateMemoryUsage();
	static void CalculateMemoryUsageByProgram();

	static DWORDLONG	s_TotalVirtualMem;
	static DWORDLONG	s_UsedVirtualMem;
	static SIZE_T		s_VirtualMemUsedByProgram;

	static DWORDLONG	s_TotalPhysMem;
	static DWORDLONG	s_UsedPhysMem;
	static SIZE_T		s_PhysMemUsedByProgram;
};