#include <algorithm>

#include <GLFW/glfw3.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/platform.h>

#include <bc7enc_wrapper.hpp>

// For native window handles via GLFW
#if BX_PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#elif BX_PLATFORM_LINUX || BX_PLATFORM_BSD
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_WAYLAND
#include <GLFW/glfw3native.h>
#elif BX_PLATFORM_OSX
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>
#endif

#include <STB/stb_image.h>

#include "Internal/Core/PMMA_Core.hpp"
#include "Internal/Core/PMMA_Registry.hpp"

#include "Internal/AnimationManager.hpp"
#include "Internal/LoggingManager.hpp"

#include "Internal/Events/EventsManager.hpp"

#include "Internal/Rendering/Core2D/RenderPipelineManager.hpp"

#include "Graphics/Shader.hpp"

#include "Events/KeyEvents.hpp"

#include "Types/Color.hpp"

#include "Display.hpp"
#include "General.hpp"

void PMMA::Display::PMMA_Update(GLFWwindow *Window) {
    int size[2];
    glfwGetWindowSize(Window, &size[0], &size[1]);
    CurrentSize[0] = static_cast<uint16_t>(size[0]);
    CurrentSize[1] = static_cast<uint16_t>(size[1]);

    if (KeyManagerInstance == nullptr) {
        if (PMMA::Core::Registry::KeyboardEventInstanceCount > 0) {
            KeyManagerInstance = new PMMA::Internal::Events::InternalKeyManager();
            glfwSetKeyCallback(Window, PMMA::Internal::Events::InternalKeyManager::KeyCallback);
        }
    } else {
        if (PMMA::Core::Registry::KeyboardEventInstanceCount <= 0) {
            glfwSetKeyCallback(Window, nullptr);
            delete KeyManagerInstance;
            KeyManagerInstance = nullptr;
            PMMA::Core::Registry::KeyboardEventInstanceCount = 0;
        } else {
            KeyManagerInstance->Update(Window);
        }
    }

    if (TextManagerInstance == nullptr) {
        if (PMMA::Core::Registry::TextEventInstanceCount > 0) {
            TextManagerInstance = new PMMA::Internal::Events::InternalTextManager();
            glfwSetCharCallback(Window, PMMA::Internal::Events::InternalTextManager::TextCallback);
        }
    } else {
        if (PMMA::Core::Registry::TextEventInstanceCount <= 0) {
            glfwSetCharCallback(Window, nullptr);
            delete TextManagerInstance;
            TextManagerInstance = nullptr;
            PMMA::Core::Registry::TextEventInstanceCount = 0;
        } else {
            TextManagerInstance->Update(Window);
        }
    }

    if (MousePositionManagerInstance == nullptr) {
        if (PMMA::Core::Registry::MousePositionEventInstanceCount > 0) {
            MousePositionManagerInstance = new PMMA::Internal::Events::InternalMousePositionManager();
            glfwSetCursorPosCallback(Window, PMMA::Internal::Events::InternalMousePositionManager::CursorPositionCallback);
        }
    } else {
        if (PMMA::Core::Registry::MousePositionEventInstanceCount <= 0) {
            glfwSetCursorPosCallback(Window, nullptr);
            delete MousePositionManagerInstance;
            MousePositionManagerInstance = nullptr;
            PMMA::Core::Registry::MousePositionEventInstanceCount = 0;
        } else {
            MousePositionManagerInstance->Update(Window);
        }
    }

    if (MouseEnterWindowManagerInstance == nullptr) {
        if (PMMA::Core::Registry::MouseEnterWindowEventInstanceCount > 0) {
            MouseEnterWindowManagerInstance = new PMMA::Internal::Events::InternalMouseEnterWindowManager();
            glfwSetCursorEnterCallback(Window, PMMA::Internal::Events::InternalMouseEnterWindowManager::CursorEnterCallback);
        }
    } else {
        if (PMMA::Core::Registry::MouseEnterWindowEventInstanceCount <= 0) {
            glfwSetCursorEnterCallback(Window, nullptr);
            delete MouseEnterWindowManagerInstance;
            MouseEnterWindowManagerInstance = nullptr;
            PMMA::Core::Registry::MouseEnterWindowEventInstanceCount = 0;
        } else {
            MouseEnterWindowManagerInstance->Update(Window);
        }
    }

    if (MouseButtonManagerInstance == nullptr) {
        if (PMMA::Core::Registry::MouseButtonEventInstanceCount > 0) {
            MouseButtonManagerInstance = new PMMA::Internal::Events::InternalMouseButtonManager();
            glfwSetMouseButtonCallback(Window, PMMA::Internal::Events::InternalMouseButtonManager::MouseButtonCallback);
        }
    } else {
        if (PMMA::Core::Registry::MouseButtonEventInstanceCount <= 0) {
            glfwSetMouseButtonCallback(Window, nullptr);
            delete MouseButtonManagerInstance;
            MouseButtonManagerInstance = nullptr;
            PMMA::Core::Registry::MouseButtonEventInstanceCount = 0;
        } else {
            MouseButtonManagerInstance->Update(Window);
        }
    }

    if (MouseScrollManagerInstance == nullptr) {
        if (PMMA::Core::Registry::MouseScrollEventInstanceCount > 0) {
            MouseScrollManagerInstance = new PMMA::Internal::Events::InternalMouseScrollManager();
            glfwSetScrollCallback(Window, PMMA::Internal::Events::InternalMouseScrollManager::ScrollCallback);
        }
    } else {
        if (PMMA::Core::Registry::MouseScrollEventInstanceCount <= 0) {
            glfwSetScrollCallback(Window, nullptr);
            delete MouseScrollManagerInstance;
            MouseScrollManagerInstance = nullptr;
            PMMA::Core::Registry::MouseScrollEventInstanceCount = 0;
        } else {
            MouseScrollManagerInstance->Update(Window);
        }
    }

    if (PMMA::Core::ControllerManagerInstance == nullptr) {
        if (PMMA::Core::Registry::ControllerEventInstanceCount > 0) {
            PMMA::Core::ControllerManagerInstance = new PMMA::Internal::Events::InternalControllerManager();
            glfwSetJoystickCallback(PMMA::Internal::Events::InternalControllerManager::JoystickCallback);
        }
    } else {
        if (PMMA::Core::Registry::ControllerEventInstanceCount <= 0) {
            glfwSetJoystickCallback(nullptr);
            delete PMMA::Core::ControllerManagerInstance;
            PMMA::Core::ControllerManagerInstance = nullptr;
            PMMA::Core::Registry::ControllerEventInstanceCount = 0;
        } else {
            PMMA::Core::ControllerManagerInstance->Update(Window);
        }
    }

    if (DropManagerInstance == nullptr) {
        if (PMMA::Core::Registry::DropEventInstanceCount > 0) {
            DropManagerInstance = new PMMA::Internal::Events::InternalDropManager();
            glfwSetDropCallback(Window, PMMA::Internal::Events::InternalDropManager::DropCallback);
        }
    } else {
        if (PMMA::Core::Registry::DropEventInstanceCount <= 0) {
            glfwSetDropCallback(Window, nullptr);
            delete DropManagerInstance;
            DropManagerInstance = nullptr;
            PMMA::Core::Registry::DropEventInstanceCount = 0;
        } else {
            DropManagerInstance->Update(Window);
        }
    }

    if (glfwWindowShouldClose(Window)) {
        if (!IsSecondaryDisplay) {
            PMMA::Core::Registry::IsApplicationRunning = false;
        }
        DisplayShouldClose = true;
    }

    if (!PMMA::Core::Registry::UserSetEscapeKeyShouldCloseWindow) {
        PMMA::Core::Registry::EscapeKeyShouldCloseWindow = FullScreen;
    }

    if (PMMA::Core::Registry::EscapeKeyShouldCloseWindow && Escape_KeyEvent->GetPressed()) {
        if (!IsSecondaryDisplay) {
            PMMA::Core::Registry::IsApplicationRunning = false;
        }
        DisplayShouldClose = true;
    }

    if (PMMA::Core::Registry::F11KeyShouldToggleFullScreen && F11_KeyEvent->GetPressed()) {
        ToggleFullScreen();
    }

    uint16_t DisplaySize[2];
    GetSize(DisplaySize);

    DisplaySizeChanged = false;

    if (DisplaySize[0] != PreviousDisplaySize[0] || DisplaySize[1] != PreviousDisplaySize[1]) {
        OrthographicProjectionSet = false;
        DisplaySizeChanged = true;
        bgfx::reset(DisplaySize[0], DisplaySize[1], BGFX_RESET_NONE);
        bgfx::setViewRect(DisplayID, 0, 0, DisplaySize[0], DisplaySize[1]);

        PreviousDisplaySize[0] = DisplaySize[0];
        PreviousDisplaySize[1] = DisplaySize[1];
    }
}

PMMA::Display::Display() {
    WindowFillColor = new PMMA::Types::Color();
    WindowFillColor->LinkedToDisplayBackground = true;

    glfwInit();

    DefaultIconPath = PMMA::Core::Registry::PMMA_Location + PMMA::Core::Registry::PathSeparator + "resources" + PMMA::Core::Registry::PathSeparator + "Icon.png";
}

GLFWmonitor *PMMA::Display::GetMonitorAtPoint(unsigned int *Point) {
    int count;

    GLFWmonitor **monitors = glfwGetMonitors(&count);

    for (int i = 0; i < count; i++) {
        int mx, my;
        glfwGetMonitorPos(monitors[i], &mx, &my);

        unsigned int Monitor_X_Position = (unsigned int)mx;
        unsigned int Monitor_Y_Position = (unsigned int)my;

        const GLFWvidmode *mode = glfwGetVideoMode(monitors[i]);

        if (Point[0] >= Monitor_X_Position && Point[0] < Monitor_X_Position + mode->width &&
            Point[1] >= Monitor_Y_Position && Point[1] < Monitor_Y_Position + mode->height) {
            // Found the monitor where the mouse cursor is
            return monitors[i];
        }
    }

    // Fallback
    return glfwGetPrimaryMonitor();
}

GLFWmonitor *PMMA::Display::GetTargetMonitor(GLFWwindow *window) {
    int Window_X_Position, Window_Y_Position;
    glfwGetWindowPos(window, &Window_X_Position, &Window_Y_Position);

    double Mouse_X_Position, Mouse_Y_Position;
    glfwGetCursorPos(window, &Mouse_X_Position, &Mouse_Y_Position);

    unsigned int Point[2] = {
        (unsigned int)(Mouse_X_Position + Window_X_Position),
        (unsigned int)(Mouse_Y_Position + Window_Y_Position)};

    return GetMonitorAtPoint(Point);
}

GLFWmonitor *PMMA::Display::GetCurrentMonitor(GLFWwindow *window) {
    int Window_X_Position, Window_Y_Position;
    glfwGetWindowPos(window, &Window_X_Position, &Window_Y_Position);

    int count;
    GLFWmonitor **monitors = glfwGetMonitors(&count);

    for (int i = 0; i < count; i++) {
        int mx, my;
        glfwGetMonitorPos(monitors[i], &mx, &my);
        const GLFWvidmode *mode = glfwGetVideoMode(monitors[i]);

        if (Window_X_Position >= mx && Window_X_Position < mx + mode->width &&
            Window_Y_Position >= my && Window_Y_Position < my + mode->height) {
            // Found the monitor where the window is
            return monitors[i];
        }
    }

    // Fallback
    return glfwGetPrimaryMonitor();
}

void PMMA::Display::Create(
    uint16_t *NewSize,
    Display_Create_Kwargs kwargs) {

    if (!kwargs.OptionalFullScreen.has_value()) {

        if (NewSize[0] == 0 && NewSize[1] == 0) {
            FullScreen = true;
        } else {
            FullScreen = false;
        }
    } else {
        FullScreen = kwargs.OptionalFullScreen.value();
    }

    Caption = kwargs.Caption;
    Resizable = kwargs.Resizable;
    NoFrame = kwargs.NoFrame;
    Vsync = kwargs.Vsync;
    Centered = kwargs.Centered;
    Maximized = kwargs.Maximized;

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow *TemporaryWindow = glfwCreateWindow(
        1,
        1,
        Caption.c_str(),
        NULL,
        NULL);
    if (!TemporaryWindow) {
        PMMA::Core::LoggingManagerInstance->InternalLogError(
            55,
            "Failed to create GLFW window. Please ensure you installed PMMA \
correctly. If the problem persists, please report this issue on our GitHub page.");

        throw std::runtime_error("Failed to create GLFW window");

        return;
    }

    int MinimumOperatingSystemApprovedWidth, MinimumOperatingSystemApprovedHeight;
    glfwGetWindowSize(TemporaryWindow, &MinimumOperatingSystemApprovedWidth, &MinimumOperatingSystemApprovedHeight);

    PMMA::Core::LoggingManagerInstance->InternalLogDebug(
        63,
        "Your Operating System has a minimum approved window size of " +
            std::to_string(MinimumOperatingSystemApprovedWidth) + "x" +
            std::to_string(MinimumOperatingSystemApprovedHeight) +
            ". If you attempt to create a window smaller than this, PMMA \
will automatically resize it to this minimum approved size.");

    int TemporaryWindow_X_Position, TemporaryWindow_Y_Position;
    glfwGetWindowPos(
        TemporaryWindow,
        &TemporaryWindow_X_Position,
        &TemporaryWindow_Y_Position);

    GLFWmonitor *CurrentMonitor = GetCurrentMonitor(TemporaryWindow);
    int CurrentMonitor_X_Position, CurrentMonitor_Y_Position;
    glfwGetMonitorPos(
        CurrentMonitor,
        &CurrentMonitor_X_Position,
        &CurrentMonitor_Y_Position);

    unsigned int RelativeWindow_X_Position, RelativeWindow_Y_Position;
    RelativeWindow_X_Position = TemporaryWindow_X_Position - CurrentMonitor_X_Position;
    RelativeWindow_Y_Position = TemporaryWindow_Y_Position - CurrentMonitor_Y_Position;

    GLFWmonitor *TargetMonitor = GetTargetMonitor(TemporaryWindow);
    glfwDestroyWindow(TemporaryWindow);

    int TargetMonitor_X_Position, TargetMonitor_Y_Position;
    glfwGetMonitorPos(
        TargetMonitor,
        &TargetMonitor_X_Position,
        &TargetMonitor_Y_Position);

    const GLFWvidmode *Mode = glfwGetVideoMode(TargetMonitor);
    int Monitor_Width = Mode->width;
    int Monitor_Height = Mode->height;

    if (RelativeWindow_X_Position > Monitor_Width - Size[0]) {
        RelativeWindow_X_Position = Monitor_Width - Size[0];
    }
    if (RelativeWindow_Y_Position > Monitor_Height - Size[1]) {
        RelativeWindow_Y_Position = Monitor_Height - Size[1];
    }
    if (RelativeWindow_X_Position < 0) {
        RelativeWindow_X_Position = 0;
    }
    if (RelativeWindow_Y_Position < 0) {
        RelativeWindow_Y_Position = 0;
    }

    int Window_X_Offset = TargetMonitor_X_Position + RelativeWindow_X_Position;
    int Window_Y_Offset = TargetMonitor_Y_Position + RelativeWindow_Y_Position;

    if (NewSize[0] > 0) {
        Size[0] = NewSize[0];
    } else {
        Size[0] = Monitor_Width;
    }

    if (NewSize[1] > 0) {
        Size[1] = NewSize[1];
    } else {
        Size[1] = Monitor_Height;
    }

    CurrentSize[0] = Size[0];
    CurrentSize[1] = Size[1];

    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);

    if (Resizable) {
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    } else {
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    }

    if (NoFrame) {
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    } else {
        glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    if (FullScreen) {
        Window = glfwCreateWindow(
            Size[0],
            Size[1],
            Caption.c_str(),
            TargetMonitor,
            NULL);
    } else {
        Window = glfwCreateWindow(
            Size[0],
            Size[1],
            Caption.c_str(),
            NULL,
            NULL);

        if (Centered) {
            int Window_X_Offset = TargetMonitor_X_Position + (Monitor_Width - Size[0]) / 2;
            int Window_Y_Offset = TargetMonitor_Y_Position + (Monitor_Height - Size[1]) / 2;
        }
        Position[0] = Window_X_Offset;
        Position[1] = Window_Y_Offset;
        glfwSetWindowPos(Window, Window_X_Offset, Window_Y_Offset);
    }

    if (!Window) {
        PMMA::Core::LoggingManagerInstance->InternalLogError(
            55,
            "Failed to create GLFW window. Please ensure you installed PMMA \
correctly. If the problem persists, please report this issue on our GitHub page.");

        throw std::runtime_error("Failed to create GLFW window");

        return;
    }

    glfwSetWindowUserPointer(Window, this);

    if (PMMA::Core::MasterDisplayInstance == nullptr) {
        PMMA::Core::MasterDisplayInstance = this;

        PMMA::Core::LoggingManagerInstance->InternalLogDebug(
            61,
            "This is the first display created in this application. It \
is now set as the master display. All other displays created after this \
will be considered secondary displays. Note: You cannot change the master \
display once it is set, closing the master display will close all other \
displays");

        bgfx::PlatformData pd{};
        pd.ndt = nullptr;
        pd.nwh = nullptr;
        pd.context = nullptr;
        pd.backBuffer = nullptr;
        pd.backBufferDS = nullptr;

#if BX_PLATFORM_WINDOWS
        pd.nwh = glfwGetWin32Window(Window);
#elif BX_PLATFORM_OSX
        pd.nwh = glfwGetCocoaWindow(Window);
#elif BX_PLATFORM_LINUX || BX_PLATFORM_BSD
        if (glfwGetPlatform() == GLFW_PLATFORM_WAYLAND) {
            pd.ndt = glfwGetWaylandDisplay();
            pd.nwh = (void *)glfwGetWaylandWindow(Window);
        } else { // X11
            pd.ndt = glfwGetX11Display();
            pd.nwh = (void *)(uintptr_t)glfwGetX11Window(Window);
        }
#endif

        bgfx::Init init;
        init.type = bgfx::RendererType::Count; // auto-detect renderer
        init.resolution.width = Size[0];
        init.resolution.height = Size[1];
        init.resolution.reset = Vsync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE;
        init.platformData = pd;

        if (!bgfx::init(init)) {
            PMMA::Core::LoggingManagerInstance->InternalLogError(
                65,
                "Failed to initialize BGFX. Please ensure you have a \
graphical desktop envionment, graphics drivers installed and have correctly \
installed PMMA.");

            throw std::runtime_error("Failed to initialize BGFX");
        }

        bgfx::setDebug(BGFX_DEBUG_NONE);

        const bgfx::Caps *caps = bgfx::getCaps();
        PMMA::Core::Registry::MaxViewID = caps->limits.maxViews;

        std::string Renderer = PMMA::General::GetGraphicsBackend();
        PMMA::Core::LoggingManagerInstance->InternalLogInfo(
            34,
            "PMMA is using the '" + Renderer + "' backend for graphics.");

        PMMA::Core::Registry::IsApplicationRunning = true;

        std::string ShaderPath =
            PMMA::Core::Registry::PMMA_Location +
            PMMA::Core::Registry::PathSeparator +
            "shaders" +
            PMMA::Core::Registry::PathSeparator +
            "2D_Core" +
            PMMA::Core::Registry::PathSeparator +
            "ShapeDefinitions";

        PMMA::Core::Core2D_ShapeSDF_Program = new PMMA::Graphics::Shader();
        PMMA::Core::Core2D_ShapeSDF_Program->LoadShaderFromFolder(
            ShaderPath,
            true);

        bc7enc_init();

    } else {
        void *nwh = nullptr;

#if BX_PLATFORM_WINDOWS
        nwh = glfwGetWin32Window(Window);
#elif BX_PLATFORM_OSX
        nwh = glfwGetCocoaWindow(Window);
#elif BX_PLATFORM_LINUX || BX_PLATFORM_BSD
        if (glfwGetPlatform() == GLFW_PLATFORM_WAYLAND) {
            nwh = (void *)glfwGetWaylandWindow(Window);
        } else { // X11
            nwh = (void *)(uintptr_t)glfwGetX11Window(Window);
        }
#endif

        // Create a framebuffer using the native window handle
        DisplayFrameBufferHandle = bgfx::createFrameBuffer(nwh, Size[0], Size[1]);

        IsSecondaryDisplay = true;
        DisplayID = PMMA::Core::Registry::SecondaryDisplayIDs.front();
        PMMA::Core::Registry::SecondaryDisplayIDs.erase(PMMA::Core::Registry::SecondaryDisplayIDs.begin());
    }

    if (!Vsync) {
        PMMA::Core::LoggingManagerInstance->InternalLogDebug(
            33,
            "You are not using vsync. We recommend using \
vsync to reduce visual tearing and improve frame pacing.");
    }

    if (kwargs.IconPath == "") {
        kwargs.IconPath = DefaultIconPath;
    }
    SetIcon(kwargs.IconPath);

    RenderPipelineCore = new PMMA::Internal::Rendering::Core2D::RenderPipelineManager();

    PreviousDisplaySize[0] = Size[0];
    PreviousDisplaySize[1] = Size[1];

    // Sets default fill color
    if (!WindowFillColor->GetSet()) {
        uint8_t fill_color[4] = {0, 0, 0, 255};
        WindowFillColor->Set_RGBA(fill_color);
    }

    uint8_t out_color[4];
    WindowFillColor->Get_RGBA(out_color);

    uint32_t clearColor =
        (out_color[0]) << 24 | // R
        (out_color[1]) << 16 | // G
        (out_color[2]) << 8 |  // B
        (out_color[3]);        // A

    bgfx::setViewClear(
        DisplayID, // view ID (use 0 for your main screen)
        BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
        clearColor,
        1.0f, // depth clear value
        0     // stencil clear value
    );

    bgfx::setViewRect(DisplayID, 0, 0, Size[0], Size[1]);
    bgfx::setViewFrameBuffer(DisplayID, DisplayFrameBufferHandle);

    if (PMMA::Core::ActiveDisplayInstance == nullptr) {
        PMMA::Core::ActiveDisplayInstance = this;
    }

    F11_KeyEvent = new PMMA::Events::Key_F11();
    Escape_KeyEvent = new PMMA::Events::Key_Escape();

    bgfx::touch(DisplayID); // Ensure view DisplayID is cleared

    bgfx::frame();

    PMMA::Core::LoggingManagerInstance->InternalLogDebug(
        73,
        "The created display has been automatically hidden until the first refresh \
call is made. This is to prevent windows appearing and being unresponsive whilst \
the app loads. It will automatically show up once the first frame is ready!");
}

void PMMA::Display::Clear() {
    if (Window == nullptr) {
        PMMA::Core::LoggingManagerInstance->InternalLogError(
            18,
            "You need to create a display before using this function. \
You can do this using `Display.create`.");
        throw std::runtime_error("Display not created yet!");
    }

    uint8_t out_color[4];
    WindowFillColor->Get_RGBA(out_color); // R (0 - 255), G (0 - 255), B (0 - 255), A (0 - 255)

    uint32_t clearColor =
        (out_color[0]) << 24 | // R
        (out_color[1]) << 16 | // G
        (out_color[2]) << 8 |  // B
        (out_color[3]);        // A

    bgfx::setViewRect(DisplayID, 0, 0, GetWidth(), GetHeight());

    bgfx::setViewFrameBuffer(DisplayID, DisplayFrameBufferHandle);

    bgfx::setViewClear(
        DisplayID, // view ID (use 0 for your main screen)
        BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
        clearColor,
        1.0f, // depth clear value
        0     // stencil clear value
    );

    bgfx::touch(DisplayID); // Ensure view DisplayID is cleared

    PMMA::Core::Registry::RollingViewID = DisplayID;

    RenderPipelineCore->Reset();

    if (PMMA::Core::AnimationManagerInstance != nullptr && !IsSecondaryDisplay) {
        if (PMMA::Core::AnimationManagerInstance->Update()) { // returns true if no longer needed
            delete PMMA::Core::AnimationManagerInstance;
            PMMA::Core::AnimationManagerInstance = nullptr;
        }
    }
}

void PMMA::Display::LimitRefreshRate(unsigned int RefreshRate) {
    float estimate = 0.001f;
    float average = 0.001f;
    unsigned int samples = 1;

    std::chrono::high_resolution_clock::time_point EndTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> FrameDuration = EndTime - StartTime;

    float TargetFrameTime = 1.0f / static_cast<float>(RefreshRate);
    float SleepTime = TargetFrameTime - FrameDuration.count();

    while (SleepTime > average) {
        std::chrono::high_resolution_clock::time_point s = std::chrono::high_resolution_clock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        std::chrono::high_resolution_clock::time_point e = std::chrono::high_resolution_clock::now();
        estimate = std::chrono::duration<float>(e - s).count();
        average = (average * samples + estimate) / (samples + 1);
        samples += 1;
        SleepTime -= average;
    }

    std::chrono::high_resolution_clock::time_point s = std::chrono::high_resolution_clock::now();
    while (std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - s).count() < SleepTime) {
    }
}

unsigned int PMMA::Display::CalculateRefreshRate(
    unsigned int RefreshRate, bool LowerRefreshRate_OnMinimize,
    bool LowerRefreshRate_OnFocusLoss,
    bool LowerRefreshRate_OnLowBattery) {

    bool Minimized = glfwGetWindowAttrib(Window, GLFW_ICONIFIED) == GLFW_TRUE;
    bool FocusLoss = glfwGetWindowAttrib(Window, GLFW_FOCUSED) == GLFW_FALSE;
    bool LowBattery = PMMA::Core::Registry::IsPowerSavingModeEnabled;

    unsigned int OriginalRefreshRate = RefreshRate;

    if (Minimized && LowerRefreshRate_OnMinimize) {
        RefreshRate /= 5;
    }

    if (FocusLoss && LowerRefreshRate_OnFocusLoss) {
        RefreshRate /= 2;
    }

    if (LowBattery && LowerRefreshRate_OnLowBattery) {
        RefreshRate /= 2;
    }

    if (Minimized) {
        RefreshRate = std::max(RefreshRate, 5u);
    } else {
        RefreshRate = std::max(RefreshRate, RefreshRate / 2);
    }

    if (RefreshRate > OriginalRefreshRate) {
        RefreshRate = OriginalRefreshRate;
    }

    return RefreshRate;
}

void PMMA::Display::Refresh(Display_Refresh_Kwargs kwargs) {
    if (FirstFrame) {
        FirstFrame = false;
        glfwShowWindow(Window);
    }

    if (Window == nullptr) {
        PMMA::Core::LoggingManagerInstance->InternalLogError(
            18,
            "You need to create a display before using this function. \
You can do this using `Display.create`.");
        throw std::runtime_error("Display not created yet!");
    }

    bgfx::setViewRect(DisplayID, 0, 0, GetWidth(), GetHeight());
    bgfx::setViewFrameBuffer(DisplayID, DisplayFrameBufferHandle);

    PMMA::Core::Core2D_ShapeSDF_Program->CreateShader();
    RenderPipelineCore->Render();

    unsigned int MaxRefreshRate;

    if (IsSecondaryDisplay) {
        glfwPollEvents();
    } else {
        PMMA::Core::LoggingManagerInstance->InternalLogDebug(
            62,
            "Always ensure that the master display is refreshed last as \
this controls both frame timing and events. Also, refreshing the master \
display updates all secondary displays.");

        bgfx::frame();

        if (kwargs.LimitRefreshRate) {
            if (!kwargs.MaxRefreshRate.has_value()) {
                if (GetIsWindowUsingVsync()) {
                    MaxRefreshRate = 0;
                } else {
                    MaxRefreshRate = 60;
                }
            } else {
                MaxRefreshRate = kwargs.MaxRefreshRate.value();
            }

            if (kwargs.MinRefreshRate == 0) {
                glfwWaitEvents();
            } else {
                glfwWaitEventsTimeout(1.0f / kwargs.MinRefreshRate);
            }
        } else {
            glfwPollEvents();
        }
    }

    PMMA_Update(Window);

    if (!IsSecondaryDisplay) {
        if (kwargs.LimitRefreshRate) {
            MaxRefreshRate = PMMA::Display::CalculateRefreshRate(
                MaxRefreshRate, kwargs.LowerRefreshRate_OnMinimize,
                kwargs.LowerRefreshRate_OnFocusLoss,
                kwargs.LowerRefreshRate_OnLowBattery);

            if (MaxRefreshRate > 0) {
                LimitRefreshRate(MaxRefreshRate);
            }
        }

        std::chrono::high_resolution_clock::time_point EndTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> FrameDuration = EndTime - StartTime;
        RefreshTime = std::chrono::duration<float>(EndTime - StartTime).count();

        StartTime = std::chrono::high_resolution_clock::now();
    }
}

void PMMA::Display::SetIcon(std::string IconPath) {
    PMMA::Internal::DisplayExistsCheck(this);

    if (IconPath == "") {
        IconPath = DefaultIconPath;
    }

    int width, height, channels;
    unsigned char *pixels = stbi_load(
        IconPath.c_str(),
        &width,
        &height,
        &channels,
        4);

    if (pixels) {
        GLFWimage icon;
        icon.width = width;
        icon.height = height;
        icon.pixels = pixels;

        glfwSetWindowIcon(Window, 1, &icon);
        stbi_image_free(pixels); // Don’t forget to free the image
    } else {
        PMMA::Core::LoggingManagerInstance->InternalLogError(
            56,
            "Failed to load icon from path: " + IconPath + ". Please \
ensure the file exists and is a valid image file.");

        throw std::runtime_error("Failed to load icon: " + IconPath);
    }
}

void PMMA::Display::ToggleFullScreen() {
    PMMA::Internal::DisplayExistsCheck(this);

    FullScreen = !FullScreen;

    unsigned int new_width, new_height;

    if (FullScreen) {
        GLFWmonitor *CurrentMonitor = GetCurrentMonitor(Window);
        int CurrentMonitor_X_Position, CurrentMonitor_Y_Position;
        int TemporaryWindow_X_Position, TemporaryWindow_Y_Position;
        glfwGetWindowPos(
            Window,
            &TemporaryWindow_X_Position,
            &TemporaryWindow_Y_Position);

        glfwGetMonitorPos(
            CurrentMonitor,
            &CurrentMonitor_X_Position,
            &CurrentMonitor_Y_Position);

        unsigned int RelativeWindow_X_Position, RelativeWindow_Y_Position;
        RelativeWindow_X_Position = TemporaryWindow_X_Position - CurrentMonitor_X_Position;
        RelativeWindow_Y_Position = TemporaryWindow_Y_Position - CurrentMonitor_Y_Position;

        GLFWmonitor *TargetMonitor = GetTargetMonitor(Window);

        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

        int TargetMonitor_X_Position, TargetMonitor_Y_Position;
        glfwGetMonitorPos(
            TargetMonitor,
            &TargetMonitor_X_Position,
            &TargetMonitor_Y_Position);

        const GLFWvidmode *Mode = glfwGetVideoMode(TargetMonitor);
        int Monitor_Width = Mode->width;
        int Monitor_Height = Mode->height;

        if (RelativeWindow_X_Position > Monitor_Width - Size[0]) {
            RelativeWindow_X_Position = Monitor_Width - Size[0];
        }
        if (RelativeWindow_Y_Position > Monitor_Height - Size[1]) {
            RelativeWindow_Y_Position = Monitor_Height - Size[1];
        }
        if (RelativeWindow_X_Position < 0) {
            RelativeWindow_X_Position = 0;
        }
        if (RelativeWindow_Y_Position < 0) {
            RelativeWindow_Y_Position = 0;
        }

        int Window_X_Offset = TargetMonitor_X_Position + RelativeWindow_X_Position;
        int Window_Y_Offset = TargetMonitor_Y_Position + RelativeWindow_Y_Position;
        Position[0] = Window_X_Offset;
        Position[1] = Window_Y_Offset;
        Size[0] = GetWidth();
        Size[1] = GetHeight();
        const GLFWvidmode *mode = glfwGetVideoMode(CurrentMonitor);
        glfwSetWindowMonitor(Window, CurrentMonitor, 0, 0, mode->width, mode->height, 0);
        new_width = mode->width;
        new_height = mode->height;
    } else {
        GLFWmonitor *CurrentMonitor = GetCurrentMonitor(Window);
        glfwSetWindowMonitor(Window, NULL, Position[0], Position[1], Size[0], Size[1], 0);
        new_width = Size[0];
        new_height = Size[1];
    }
}

PMMA::Display::~Display() {
    PMMA::Core::Registry::SecondaryDisplayIDs.push_back(DisplayID);

    if (RenderPipelineCore != nullptr) {
        delete RenderPipelineCore;
        RenderPipelineCore = nullptr;
    }

    if (KeyManagerInstance != nullptr) {
        delete KeyManagerInstance;
        KeyManagerInstance = nullptr;
    }

    if (TextManagerInstance != nullptr) {
        delete TextManagerInstance;
        TextManagerInstance = nullptr;
    }

    if (MousePositionManagerInstance != nullptr) {
        delete MousePositionManagerInstance;
        MousePositionManagerInstance = nullptr;
    }

    if (MouseEnterWindowManagerInstance != nullptr) {
        delete MouseEnterWindowManagerInstance;
        MouseEnterWindowManagerInstance = nullptr;
    }

    if (MouseButtonManagerInstance != nullptr) {
        delete MouseButtonManagerInstance;
        MouseButtonManagerInstance = nullptr;
    }

    if (MouseScrollManagerInstance != nullptr) {
        delete MouseScrollManagerInstance;
        MouseScrollManagerInstance = nullptr;
    }

    if (PMMA::Core::ControllerManagerInstance != nullptr && !IsSecondaryDisplay) {
        delete PMMA::Core::ControllerManagerInstance;
        PMMA::Core::ControllerManagerInstance = nullptr;
    }

    if (DropManagerInstance != nullptr) {
        delete DropManagerInstance;
        DropManagerInstance = nullptr;
    }

    if (!IsSecondaryDisplay) {
        PMMA::Core::MasterDisplayInstance = nullptr;

        delete PMMA::Core::Core2D_ShapeSDF_Program;

        glfwTerminate();
        bgfx::shutdown();
    }

    glfwDestroyWindow(Window);
    Window = nullptr;

    delete WindowFillColor;
    WindowFillColor = nullptr;

    delete F11_KeyEvent;
    F11_KeyEvent = nullptr;
    delete Escape_KeyEvent;
    Escape_KeyEvent = nullptr;
}

void PMMA::Display::SetAsActiveDisplay() {
    PMMA::Core::ActiveDisplayInstance = this;
}

bool PMMA::Display::GetIsActiveDisplay() {
    return PMMA::Core::ActiveDisplayInstance == this;
}

bool PMMA::Display::GetIsWindowUsingVsync() {
    PMMA::Internal::DisplayExistsCheck(this);

    return Vsync;
}

unsigned int PMMA::Display::GetCurrentMonitorRefreshRate() {
    PMMA::Internal::DisplayExistsCheck(this);

    GLFWmonitor *CurrentMonitor = GetCurrentMonitor(Window);
    const GLFWvidmode *Mode = glfwGetVideoMode(CurrentMonitor);
    CurrentMonitorRefreshRate = Mode->refreshRate;
    return CurrentMonitorRefreshRate;
}

uint16_t PMMA::Display::GetWidth() {
    PMMA::Internal::DisplayExistsCheck(this);
    return CurrentSize[0];
};

uint16_t PMMA::Display::GetHeight() {
    PMMA::Internal::DisplayExistsCheck(this);

    return CurrentSize[1];
};

void PMMA::Display::GetSize(uint16_t *out) {
    PMMA::Internal::DisplayExistsCheck(this);

    out[0] = CurrentSize[0];
    out[1] = CurrentSize[1];
};

void PMMA::Display::SetRelativeWindowPosition(unsigned int *NewPosition) {
    PMMA::Internal::DisplayExistsCheck(this);

    glfwSetWindowPos(Window, NewPosition[0], NewPosition[1]);
}

void PMMA::Display::SetAbsoluteWindowPosition(unsigned int *NewPosition) {
    PMMA::Internal::DisplayExistsCheck(this);

    GLFWmonitor *PointMonitor = GetMonitorAtPoint(NewPosition);

    int Monitor_X_Position, Monitor_Y_Position;
    glfwGetMonitorPos(PointMonitor, &Monitor_X_Position, &Monitor_Y_Position);
    glfwSetWindowPos(Window, NewPosition[0] - Monitor_X_Position, NewPosition[1] - Monitor_Y_Position);
}

void PMMA::Display::CenterWindow() {
    PMMA::Internal::DisplayExistsCheck(this);

    GLFWmonitor *CurrentMonitor = GetCurrentMonitor(Window);

    int Monitor_Width, Monitor_Height;
    const GLFWvidmode *Mode = glfwGetVideoMode(CurrentMonitor);
    Monitor_Width = Mode->width;
    Monitor_Height = Mode->height;

    int Window_X_Offset = (Monitor_Width - Size[0]) / 2;
    int Window_Y_Offset = (Monitor_Height - Size[1]) / 2;

    glfwSetWindowPos(Window, Window_X_Offset, Window_Y_Offset);
}

void PMMA::Display::SetWindowInFocus() {
    PMMA::Internal::DisplayExistsCheck(this);

    glfwFocusWindow(Window);
}

void PMMA::Display::SetWindowMinimized(bool IsMinimized) {
    PMMA::Internal::DisplayExistsCheck(this);

    if (IsMinimized) {
        glfwIconifyWindow(Window);
    } else {
        glfwRestoreWindow(Window);
    }
}

void PMMA::Display::SetWindowMaximized(bool IsMaximized) {
    PMMA::Internal::DisplayExistsCheck(this);

    if (IsMaximized) {
        glfwMaximizeWindow(Window);
    } else {
        glfwRestoreWindow(Window);
    }
}

bool PMMA::Display::GetIsWindowInFocus() {
    PMMA::Internal::DisplayExistsCheck(this);

    return glfwGetWindowAttrib(Window, GLFW_FOCUSED) == GLFW_TRUE;
}

bool PMMA::Display::GetIsWindowMinimized() {
    PMMA::Internal::DisplayExistsCheck(this);

    return glfwGetWindowAttrib(Window, GLFW_ICONIFIED) == GLFW_TRUE;
}

bool PMMA::Display::GetIsWindowResizable() {
    PMMA::Internal::DisplayExistsCheck(this);

    return glfwGetWindowAttrib(Window, GLFW_RESIZABLE) == GLFW_TRUE;
}

bool PMMA::Display::GetIsWindowVisible() {
    PMMA::Internal::DisplayExistsCheck(this);

    return glfwGetWindowAttrib(Window, GLFW_VISIBLE) == GLFW_TRUE;
}

bool PMMA::Display::GetIsWindowAlwaysOnTop() {
    PMMA::Internal::DisplayExistsCheck(this);

    return glfwGetWindowAttrib(Window, GLFW_FLOATING) == GLFW_TRUE;
}

bool PMMA::Display::GetIsWindowAutoMinimize() {
    PMMA::Internal::DisplayExistsCheck(this);

    return glfwGetWindowAttrib(Window, GLFW_AUTO_ICONIFY) == GLFW_TRUE;
}

bool PMMA::Display::GetIsWindowMaximized() {
    PMMA::Internal::DisplayExistsCheck(this);

    return glfwGetWindowAttrib(Window, GLFW_MAXIMIZED) == GLFW_TRUE;
}

unsigned int PMMA::Display::GetWindow_MSAA_Samples() {
    PMMA::Internal::DisplayExistsCheck(this);

    return glfwGetWindowAttrib(Window, GLFW_SAMPLES);
}

void PMMA::Display::SetCaption(std::string new_caption) {
    PMMA::Internal::DisplayExistsCheck(this);

    glfwSetWindowTitle(Window, new_caption.c_str());
    Caption = new_caption;
}

void PMMA::Display::GetCenterPosition(uint16_t *out) {
    PMMA::Internal::DisplayExistsCheck(this);

    out[0] = Size[0] / 2;
    out[1] = Size[1] / 2;
}

uint16_t PMMA::Display::GetHorizontalCenterPosition() {
    PMMA::Internal::DisplayExistsCheck(this);

    return Size[0] / 2;
}

uint16_t PMMA::Display::GetVerticalCenterPosition() {
    PMMA::Internal::DisplayExistsCheck(this);

    return Size[1] / 2;
}

void PMMA::Display::GetCenterPosition(uint16_t *ObjectSize, uint16_t *out) {
    PMMA::Internal::DisplayExistsCheck(this);

    out[0] = (Size[0] - ObjectSize[0]) / 2;
    out[1] = (Size[1] - ObjectSize[1]) / 2;
}

uint16_t PMMA::Display::GetHorizontalCenterPosition(uint16_t ObjectSize) {
    PMMA::Internal::DisplayExistsCheck(this);

    return (Size[0] - ObjectSize) / 2;
}

uint16_t PMMA::Display::GetVerticalCenterPosition(uint16_t ObjectSize) {
    PMMA::Internal::DisplayExistsCheck(this);

    return (Size[1] - ObjectSize) / 2;
}

float PMMA::Display::GetAspectRatio() {
    PMMA::Internal::DisplayExistsCheck(this);
    uint16_t Size[2];
    GetSize(Size);
    return (float)Size[0] / (float)Size[1];
}

void PMMA::Display::TriggerEventRefresh() {
    PMMA::Internal::DisplayExistsCheck(this);
    glfwPostEmptyEvent();
}

unsigned int PMMA::Display::GetFrameRate() {
    PMMA::Internal::DisplayExistsCheck(this);
    return (unsigned int)(1 / RefreshTime);
}

float PMMA::Display::GetFrameTime() {
    PMMA::Internal::DisplayExistsCheck(this);
    return RefreshTime;
}

void PMMA::Display::GetOrthographicProjection(float *out) {
    PMMA::Internal::DisplayExistsCheck(this);

    if (OrthographicProjectionSet) {
        out[0] = OrthographicProjection[0];
        out[1] = OrthographicProjection[1];
        out[2] = OrthographicProjection[2];
        out[3] = OrthographicProjection[3];
        out[4] = OrthographicProjection[4];
        out[5] = OrthographicProjection[5];
        out[6] = OrthographicProjection[6];
        out[7] = OrthographicProjection[7];
        out[8] = OrthographicProjection[8];
        out[9] = OrthographicProjection[9];
        out[10] = OrthographicProjection[10];
        out[11] = OrthographicProjection[11];
        out[12] = OrthographicProjection[12];
        out[13] = OrthographicProjection[13];
        out[14] = OrthographicProjection[14];
        out[15] = OrthographicProjection[15];
        return;
    }

    uint16_t Size[2];
    GetSize(Size);

    OrthographicProjection[0] = 2.0f / (float)Size[0];
    OrthographicProjection[5] = -2.0f / (float)Size[1];
    OrthographicProjection[10] = 1.0f;
    OrthographicProjection[12] = -1.0f;
    OrthographicProjection[13] = 1.0f;
    OrthographicProjection[14] = 0.0f;
    OrthographicProjection[15] = 1.0f;

    OrthographicProjectionSet = true;
};