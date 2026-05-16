#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS

#include <SDL3/SDL.h>

#include "../shared/SharedPCH.h"
#include "../shared/Utilities.h"
#include "./Types.h"
#include "./ImGui/imgui.h"
#include "./ImGui/imgui_impl_sdl3.h"
#include "./ImGui/imgui_impl_sdlrenderer3.h"
#include "./ImGui/imgui_internal.h"

namespace NS_Proteus {
    static SDL_GPUTextureCreateInfo* CreateTextureInfo(u32 w, u32 h) {
        return new SDL_GPUTextureCreateInfo{
            .type = SDL_GPU_TEXTURETYPE_2D,
            .format = SDL_GetGPUTextureFormatFromPixelFormat(SDL_PIXELFORMAT_ABGR8888),
            .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
            .width = w,
            .height = h,
            .layer_count_or_depth = 1,
            .num_levels = 1,
            .sample_count = SDL_GPU_SAMPLECOUNT_1
        };
    }
}

namespace ImGui {
    static vector<string> SplitTextToWrappedLines(ImFont* font, float font_size, const char* text, float wrap_width) {
        vector<string> ret = {};
        const char* text_end = text + ImStrlen(text);
        const char* sol = text;
        const char* eol = nullptr;
        while (sol < text_end) {
            eol = ImFontCalcWordWrapPositionEx(font, font_size, sol, text_end, wrap_width);
            ret.push_back(string(sol, (eol - sol)));
            sol = eol;
            eol = nullptr;
            sol = ImTextCalcWordWrapNextLineStart(sol, text_end);
        }
        return ret;
    }

    static void TextWrappedCentered(const char* text, float wrap = 0.33333333f) {
        ImGuiContext& g = *GImGui;
        ImFont* font = g.Font;
        float font_size = g.FontSize;
        const char* text_end = text + ImStrlen(text);

        // availabe space in ImGui window
        ImVec2 region = GetContentRegionAvail();
        float wrap_width = region.x * wrap;
        // split text into lines based on wrap_width
        vector<string> lines = SplitTextToWrappedLines(font, font_size, text, wrap_width);

        // initial cursor position
        ImVec2 cPos = {};
        cPos.y = (region.y - (font_size * lines.size())) / 2.0f;
        // print each line
        for (const string& line : lines) {
            // get c-string
            const char* cstr = line.c_str();
            size_t len = line.size();

            // calculate render pos
            ImVec2 tSize = CalcTextSize(cstr);
            
            // set cursor x position
            cPos.x = (region.x - tSize.x) * 0.5f;
            ImGui::SetCursorPos(cPos);
            ImGui::Text(cstr);
            cPos.y += font_size;
        }
    }

    static bool ButtonCentered(const char* label, ImVec2 size_arg, ImGuiButtonFlags flags = ImGuiButtonFlags_None) {
        #pragma region UGH
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        if (window->SkipItems) return false;

        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);
        const char* label_end = FindRenderedTextEnd(label);
        ImFont* font = g.Font;
        float font_size = g.FontSize;
        float wrap_width = size_arg.x * 0.85f;
        const ImVec2 label_size = CalcTextSize(label, label_end, false, wrap_width);
        
        ImVec2 pos = window->DC.CursorPos;
        if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset)
            pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
        ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

        const ImRect bb(pos, pos + size);
        ItemSize(size, style.FramePadding.y);
        if (!ItemAdd(bb, id))
            return false;

        bool hovered, held;
        bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

        // Render
        const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
        RenderNavCursor(bb, id);
        RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);

        if (g.LogEnabled)
            LogSetNextTextDecoration("[", "]");
        const char* text_display_end = FindRenderedTextEnd(label, label_end);
        const int text_len = (int)(text_display_end - label);
        #pragma endregion
        if (text_len != 0) {
            // split label into lines
            vector<string> lines = SplitTextToWrappedLines(font, font_size, label, wrap_width);

            ImVec2 cPos = {};
            cPos.y = pos.y + ((size_arg.y - (font_size * lines.size())) / 2.0f);
            for (const string& line : lines) {
                const char* cstr = line.c_str();
                size_t len = line.size();

                ImVec2 tSize = CalcTextSize(cstr);

                cPos.x = pos.x + ((size_arg.x - tSize.x) * 0.5f);
                RenderText(cPos, cstr, cstr + len);
                cPos.y += font_size;
            }

            if (g.LogEnabled)
                LogRenderedText(&bb.Min, label, text_display_end);
        }

        IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
        return pressed;
    }
}