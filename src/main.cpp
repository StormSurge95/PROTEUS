#include "./frontend/Proteus.h"

using namespace NS_Proteus;

#define WINSUB 0

// TODO: switch to WINDOWS mode instead of CONSOLE
#if (WINSUB)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
#else
int main(int i, int args[]) {
#endif
    Proteus* proteus = new Proteus();

    proteus->Init();
    proteus->Run();

    delete proteus;

    exit(EXIT_SUCCESS);

    return 0;
}

//#include "frontend/ImGui/imgui.h"
//#include "frontend/ImGui/imgui_impl_sdl3.h"
//#include "frontend/ImGui/imgui_impl_sdlgpu3.h"
//#include <stdio.h>
//#include <stdlib.h>
//#include <SDL3/SDL.h>
//
//using namespace ImGui;
//
//int main(int, char**) {
//    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
//        printf("Error: SDL_Init(): %s\n", SDL_GetError());
//        return 1;
//    }
//
//    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
//    SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
//    SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL3+SDL_GPU example", (int)(1920 * main_scale), (int)(1080 * main_scale), window_flags);
//    if (window == nullptr) {
//        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
//        return 1;
//    }
//    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
//    SDL_ShowWindow(window);
//
//    SDL_GPUDevice* gpu_device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL | SDL_GPU_SHADERFORMAT_METALLIB, true, nullptr);
//    if (gpu_device == nullptr) {
//        printf("Error: SDL_CreateGPUDevice(): %s\n", SDL_GetError());
//        return 1;
//    }
//
//    if (!SDL_ClaimWindowForGPUDevice(gpu_device, window)) {
//        printf("Error: SDL_ClaimWindowForGPUDevice(): %s\n", SDL_GetError());
//        return 1;
//    }
//    SDL_SetGPUSwapchainParameters(gpu_device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC);
//
//    IMGUI_CHECKVERSION();
//    CreateContext();
//    ImGuiIO& io = GetIO(); (void)io;
//    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
//    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
//
//    StyleColorsDark();
//
//    ImGuiStyle& style = GetStyle();
//    style.ScaleAllSizes(main_scale);
//    style.FontScaleDpi = main_scale;
//
//    ImGui_ImplSDL3_InitForSDLGPU(window);
//    ImGui_ImplSDLGPU3_InitInfo init_info = {};
//    init_info.Device = gpu_device;
//    init_info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(gpu_device, window);
//    init_info.MSAASamples = SDL_GPU_SAMPLECOUNT_1;
//    init_info.SwapchainComposition = SDL_GPU_SWAPCHAINCOMPOSITION_SDR;
//    init_info.PresentMode = SDL_GPU_PRESENTMODE_VSYNC;
//    ImGui_ImplSDLGPU3_Init(&init_info);
//
//    // fonts?
//
//    bool show_demo_window = true;
//    bool show_another_window = false;
//    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
//
//    bool done = false;
//    while (!done) {
//        SDL_Event event;
//        while (SDL_PollEvent(&event)) {
//            ImGui_ImplSDL3_ProcessEvent(&event);
//            if (event.type == SDL_EVENT_QUIT)
//                done = true;
//            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
//                done = true;
//        }
//
//        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
//            SDL_Delay(10);
//            continue;
//        }
//
//        ImGui_ImplSDLGPU3_NewFrame();
//        ImGui_ImplSDL3_NewFrame();
//        NewFrame();
//
//        if (show_demo_window)
//            ShowDemoWindow(&show_demo_window);
//
//        {
//            static float f = 0.0f;
//            static int counter = 0;
//
//            Begin("Hellow, world!");
//
//            Text("This is some useful text.");
//            Checkbox("Demo Window", &show_demo_window);
//            Checkbox("Another window", &show_another_window);
//
//            SliderFloat("float", &f, 0.0f, 1.0f);
//            ColorEdit3("clear color", (float*)&clear_color);
//
//            if (Button("Button"))
//                counter++;
//            SameLine();
//            Text("counter = %d", counter);
//
//            Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
//            End();
//        }
//
//        if (show_another_window) {
//            Begin("Another Window", &show_another_window);
//            Text("Hello from another window!");
//            if (Button("Close Me"))
//                show_another_window = false;
//            End();
//        }
//
//        Render();
//        ImDrawData* draw_data = GetDrawData();
//        const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
//
//        SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(gpu_device);
//
//        SDL_GPUTexture* swapchain_texture;
//        SDL_WaitAndAcquireGPUSwapchainTexture(command_buffer, window, &swapchain_texture, nullptr, nullptr);
//
//        if (swapchain_texture != nullptr && !is_minimized) {
//            ImGui_ImplSDLGPU3_PrepareDrawData(draw_data, command_buffer);
//
//            SDL_GPUColorTargetInfo target_info = {};
//            target_info.texture = swapchain_texture;
//            target_info.clear_color = SDL_FColor{ clear_color.x, clear_color.y, clear_color.z, clear_color.w };
//            target_info.load_op = SDL_GPU_LOADOP_CLEAR;
//            target_info.store_op = SDL_GPU_STOREOP_STORE;
//            target_info.mip_level = 0;
//            target_info.layer_or_depth_plane = 0;
//            target_info.cycle = false;
//            SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(command_buffer, &target_info, 1, nullptr);
//
//            ImGui_ImplSDLGPU3_RenderDrawData(draw_data, command_buffer, render_pass);
//
//            SDL_EndGPURenderPass(render_pass);
//        }
//
//        SDL_SubmitGPUCommandBuffer(command_buffer);
//    }
//
//    SDL_WaitForGPUIdle(gpu_device);
//    ImGui_ImplSDL3_Shutdown();
//    ImGui_ImplSDLGPU3_Shutdown();
//    DestroyContext();
//
//    SDL_ReleaseWindowFromGPUDevice(gpu_device, window);
//    SDL_DestroyGPUDevice(gpu_device);
//    SDL_DestroyWindow(window);
//    SDL_Quit();
//
//    return 0;
//}