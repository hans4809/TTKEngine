#include "ImGUI/imgui.h"
#include "ImGUI/imgui_impl_dx11.h"
#include "ImGUI/imgui_impl_win32.h"
#include "ImGuiManager.h"
#include "Font/RawFonts.h"
#include "Font/IconDefs.h"

ImGuiManager& ImGuiManager::Get()
{
    static ImGuiManager Instance;
    return Instance;
}

void ImGuiManager::AddWindow(HWND hWnd, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext)
{
    if (WindowContextMap.Contains(hWnd))
    {
        return;
    }

    ImGuiContext* OriginalContext = ImGui::GetCurrentContext();

    // ImGui Context 생성
    ImGuiContext* Context = ImGui::CreateContext();
    ImGui::SetCurrentContext(Context);

    // Win32 백엔드 초기화 (핸들 마다)
    ImGui_ImplWin32_Init(hWnd);
    // DX11 백엔드 초기화 (전역에 한 번만)
    ImGuiIO& io = ImGui::GetIO();
    if (io.BackendRendererUserData == nullptr)
    {
        ImGui_ImplDX11_Init(Device, DeviceContext);
    }

    WindowContextMap.Add(hWnd, Context);

    InitializeWindow();

    ImGui::SetCurrentContext(OriginalContext);
}

void ImGuiManager::RemoveWindow(HWND hWnd)
{
    if (!WindowContextMap.Contains(hWnd))
    {
        return;
    }

    ImGui::SetCurrentContext(WindowContextMap[hWnd]);
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext(WindowContextMap[hWnd]);

    WindowContextMap.Remove(hWnd);
}

void ImGuiManager::InitializeWindow()
{
    IMGUI_CHECKVERSION();
    ImGuiIO& io = ImGui::GetIO();
    
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\malgun.ttf", 17.0f, NULL, io.Fonts->GetGlyphRangesKorean());
    ImGui::GetStyle().ScaleAllSizes(0.8f);

    ImFontConfig FeatherFontConfig;
    FeatherFontConfig.PixelSnapH = true;
    FeatherFontConfig.FontDataOwnedByAtlas = false;
    FeatherFontConfig.GlyphOffset = ImVec2(0, 0);
    static const ImWchar IconRanges[] = {
        FEATHER_MOVE,          FEATHER_MOVE + 1,
        FEATHER_ROTATE,        FEATHER_ROTATE + 1,
        FEATHER_SCALE,         FEATHER_SCALE + 1,
        FEATHER_MONITOR,       FEATHER_MONITOR + 1,
        FEATHER_BAR_GRAPH,     FEATHER_BAR_GRAPH + 1,

        FEATHER_NEW,           FEATHER_NEW + 1,
        FEATHER_SAVE,          FEATHER_SAVE + 1,
        FEATHER_LOAD,          FEATHER_LOAD + 1,

        FEATHER_MENU,          FEATHER_MENU + 1,
        FEATHER_SLIDER,        FEATHER_SLIDER + 1,
        FEATHER_PLUS,          FEATHER_PLUS + 1,

        FEATHER_ZOOM_IN,       FEATHER_ZOOM_IN + 1,
        FEATHER_ZOOM_OUT,      FEATHER_ZOOM_OUT + 1,

        FEATHER_LOCK,          FEATHER_LOCK + 1,
        FEATHER_UNLOCK,        FEATHER_UNLOCK + 1,

        FEATHER_PLAY,          FEATHER_PLAY + 1,
        FEATHER_REVERSE,       FEATHER_REVERSE + 1,
        FEATHER_PAUSE,         FEATHER_PAUSE + 1,
        FEATHER_STOP,          FEATHER_STOP + 1,
        FEATHER_PREV_FRAME,    FEATHER_PREV_FRAME + 1,
        FEATHER_NEXT_FRAME,    FEATHER_NEXT_FRAME + 1,
        FEATHER_SKIP_BACK,     FEATHER_SKIP_BACK + 1,
        FEATHER_SKIP_FORWARD,  FEATHER_SKIP_FORWARD + 1,
        FEATHER_REPEAT,        FEATHER_REPEAT + 1,
        FEATHER_ONCE,          FEATHER_ONCE + 1,
        FEATHER_RECORD,        FEATHER_RECORD + 1,
        
        FEATHER_REWIND,        FEATHER_REWIND + 1,
        FEATHER_FAST_FORWARD,  FEATHER_FAST_FORWARD + 1,
        0 };

    io.Fonts->AddFontFromMemoryTTF(FeatherExRawData, FontSizeOfFeatherEx, 22.0f, &FeatherFontConfig, IconRanges);
    PreferenceStyle();
}

void ImGuiManager::BeginFrame(HWND hWnd)
{
    if (!WindowContextMap.Contains(hWnd))
    {
        return;
    }

    ImGui::SetCurrentContext(WindowContextMap[hWnd]);
    
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void ImGuiManager::EndFrame(HWND hWnd)
{
    if (!WindowContextMap.Contains(hWnd))
    {
        return;
    }
    
    ImGui::Render();
    
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

/* GUI Style Preference */
void ImGuiManager::PreferenceStyle()
{
    ImGuiStyle& Style = ImGui::GetStyle();

    /** Colors */
    ImVec4 AccentColor = ImVec4(0.8f, 0.0f, 0.16f, 1.0f);
    ImVec4 PrimaryTextColor = ImGui::ColorConvertU32ToFloat4(IM_COL32(200, 200, 200, 255));

    Style.WindowRounding = 5.0f;
    Style.FrameRounding = 5.0f;

    /** Text accent color */
    Style.Colors[ImGuiCol_Text] = PrimaryTextColor;

    // Window
    Style.Colors[ImGuiCol_WindowBg] = ImVec4(0.02f, 0.02f, 0.02f, 1);
    Style.Colors[ImGuiCol_Border] = ImVec4(0.098f, 0.098f, 0.098f, 1);

    Style.Colors[ImGuiCol_PopupBg] = ImVec4(0.0f, 0.0f, 0.0f, 1);

    // Menubar
    Style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 1.0f, 1.0f, 0.0f);

    // Title
    Style.Colors[ImGuiCol_TitleBg] = ImVec4{ 0.05f, 0.05f, 0.05f, 1.0f };
    Style.Colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.1f, 0.1f, 0.1f, 1.0f };
    Style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.05f, 0.05f, 0.05f, 1.0f };

    // Header
    Style.Colors[ImGuiCol_Header] = ImVec4(1, 1, 1, 0.0f);
    Style.Colors[ImGuiCol_HeaderActive] = ImVec4(1, 1, 1, 0.3f);
    Style.Colors[ImGuiCol_HeaderHovered] = ImVec4(1, 1, 1, 0.25f);

    // Button
    Style.Colors[ImGuiCol_Button] = ImVec4(0, 0, 0, 1.0f);
    Style.Colors[ImGuiCol_ButtonActive] = ImVec4(1, 1, 1, 0.3f);
    Style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1, 1, 1, 0.25f);

    // Frame
    Style.Colors[ImGuiCol_FrameBg] = ImVec4{ 0.039f, 0.039f, 0.039f, 1.0f };
    Style.Colors[ImGuiCol_FrameBgHovered] = ImVec4{ 1, 1, 1, 0.25f };
    Style.Colors[ImGuiCol_FrameBgActive] = ImVec4{ 1, 1, 1, 0.3f };

    // Check
    Style.Colors[ImGuiCol_CheckMark] = AccentColor;

    // Slider
    Style.Colors[ImGuiCol_SliderGrab] = AccentColor;
    Style.Colors[ImGuiCol_SliderGrabActive] = AccentColor;

    // ResizeGrip
    Style.Colors[ImGuiCol_ResizeGrip] = AccentColor;
    Style.Colors[ImGuiCol_ResizeGripHovered] = AccentColor;
    Style.Colors[ImGuiCol_ResizeGripActive] = AccentColor;

    // Tabs
    ImGui::GetStyle().Colors[ImGuiCol_Tab] = ImVec4{ 0.0f, 0.0f, 0.0f, 0.15f };
    ImGui::GetStyle().Colors[ImGuiCol_TabHovered] = ImVec4{ 0.0f, 0.0f, 0.0f, 0.15f };
    ImGui::GetStyle().Colors[ImGuiCol_TabActive] = ImVec4{ 0.0f, 0.0f, 0.0f, 0.15f };
    ImGui::GetStyle().Colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.0f, 0.0f, 0.0f, 1.0f };
    ImGui::GetStyle().Colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.0f, 0.0f, 0.0f, 0.15f };
}

void ImGuiManager::Release()
{
    TMap<HWND, ImGuiContext*> CopiedMap = WindowContextMap;
    for (auto& [HWnd, WindowContext] : CopiedMap)
    {
        RemoveWindow(HWnd);
    }

    WindowContextMap.Empty();
}

bool ImGuiManager::GetWantCaptureMouse(HWND hWnd)
{
    if (!WindowContextMap.Contains(hWnd))
    {
       return false;
    }

    bool bWantCaptureMouse;

    ImGuiContext* OriginalContext = ImGui::GetCurrentContext();
    ImGui::SetCurrentContext(WindowContextMap[hWnd]);
    {
        bWantCaptureMouse = ImGui::GetIO().WantCaptureMouse;
    }
    ImGui::SetCurrentContext(OriginalContext);

    return bWantCaptureMouse;
}

bool ImGuiManager::GetWantCaptureKeyboard(HWND hWnd)
{
    if (!WindowContextMap.Contains(hWnd))
    {
        return false;
    }

    bool bWantCaptureKeyboard;

    ImGuiContext* OriginalContext = ImGui::GetCurrentContext();
    ImGui::SetCurrentContext(WindowContextMap[hWnd]);
    {
        bWantCaptureKeyboard = ImGui::GetIO().WantCaptureKeyboard;
    }
    ImGui::SetCurrentContext(OriginalContext);

    return bWantCaptureKeyboard;
}

ImGuiContext* ImGuiManager::GetImGuiContext(HWND hWnd)
{
    if (!WindowContextMap.Contains(hWnd))
    {
        return nullptr;
    }

    return WindowContextMap[hWnd];
}

