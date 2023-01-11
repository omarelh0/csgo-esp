// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs
// -------------- https://github.com/omarelh0 ---------------
#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <memory/memman.h>
#include "GL/glew.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "vectorDT.h"
#include "offsets.h"
#include "functions.h"

    using namespace hazedumper::netvars;
    using namespace hazedumper::signatures;

#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}


int main(int, char**)
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    if (!monitor)
    {
        return 0;
    }
    int Width = glfwGetVideoMode(monitor)->width;
    int Height = glfwGetVideoMode(monitor)->height;

    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, true);
    glfwWindowHint(GLFW_RESIZABLE, false);
    glfwWindowHint(GLFW_MAXIMIZED, true);
    glfwWindowHint(GLFW_FLOATING, true);
    glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, true);

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(Width, Height, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL)
        return 1;

    glfwMakeContextCurrent(window);
    //glfwSwapInterval(1); // Enable vsync
    glfwSetWindowAttrib(window, GLFW_DECORATED, false);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    //ImGui::StyleColorsDark();
    ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    bool bMenuVisible = false;
    glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH, true);
    
    MemMan Mem;

    uintptr_t ProcID = Mem.getProcess(L"csgo.exe");
    if (ProcID == NULL) 
    {
        return 0;
    }

    uintptr_t Client = Mem.getModule(ProcID, L"client.dll");
    if (Client == NULL) 
    {
        return 0;
    }

    Matrix4x4 ViewMatrix;
    Matrix3x4 Matrix3x4;
    Vec2 LineOrigin;
    LineOrigin.X = 0.0f;
    LineOrigin.Y = -1.0f;
    uintptr_t LocalPlayer;

    ColorA* Color = new ColorA();
    Color->G = 0.0f;
    Color->A = 1.0f;

    do 
    
    { LocalPlayer = Mem.readMem<uintptr_t>(Client + dwLocalPlayer);
    } while (LocalPlayer == NULL);
    

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        
        //DRAW HERE
        if (GetAsyncKeyState(VK_UP) & 1)
        {
            bMenuVisible = !bMenuVisible;
            if (bMenuVisible)
            {
                HideMenu(window);
            }
            else
            {
                ShowMenu(window);
            }
        }

        if (bMenuVisible)
        {
            if (ImGui::Button("Exit"))
            {
                return 0;
            }
        }

        do
        
        { LocalPlayer = Mem.readMem<uintptr_t>(Client + dwLocalPlayer);
        } while (LocalPlayer == NULL);

        ViewMatrix = Mem.readMem<Matrix4x4>(Client + dwViewMatrix);

        int LocalTeam = Mem.readMem<int>(LocalPlayer + m_iTeamNum);
       
        for (short int i = 0; i < 64; ++i)
        {
            uintptr_t Entity = Mem.readMem<uintptr_t>(Client + dwEntityList + i * 0x10);
            if (Entity == NULL) continue;
            if (Mem.readMem<bool>(Entity + m_bDormant)) continue;
            if (Mem.readMem<bool>(Entity + m_iHealth) == 0) continue;
            Vec3 EntityLocation = Mem.readMem<Vec3>(Entity + m_vecOrigin);
            Vec3 LocalLocation = Mem.readMem<Vec3>(LocalPlayer + m_vecOrigin);
            Vec3 ScreenCords;
            int TeamId = Mem.readMem<int>(Entity + m_iTeamNum);
            bool OnSameTeam = TeamId == LocalTeam;

            if (OnSameTeam)
            {
                Color->R = 0.0f;
                Color->B = 1.0f;
            }

            else
            {
                Color->R = 1.0f;
                Color->B = 0.0f;
            }

            if (!WorldToScreen(EntityLocation, ScreenCords, ViewMatrix.vmatrix, Height, Width)) continue;
            DrawLine(LineOrigin, ScreenCords, Color);
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate(); 
}