#include "menu.h"

#pragma comment (lib, "d3dx9.lib")
#pragma comment (lib, "d3d9.lib")

#define IM_USE using namespace ImGui;
int tab = 0;
int subtab = 0;

enum AnimationTypes { STATIC, DYNAMIC, INTERP };

std::string pChar(std::string first_, std::string second_) 
{
    return (first_ + second_);
}

float Animate(const char* label, const char* second_label, bool if_, float Maximal_, float Speed_, int type) {

    auto ID = ImGui::GetID(pChar(label, second_label).c_str());

    static std::map<ImGuiID, float> pValue;

    auto this_e = pValue.find(ID);

    if (this_e == pValue.end())
    {
        pValue.insert({ ID, 0.f });
        this_e = pValue.find(ID);
    }

    switch (type)
    {

    case DYNAMIC:
    {
        if (if_) //do
            this_e->second += abs(Maximal_ - this_e->second) / Speed_;
        else
            this_e->second -= (0 + this_e->second) / Speed_;
    }
    break;

    case INTERP:
    {
        if (if_) //do
            this_e->second += (Maximal_ - this_e->second) / Speed_;
        else
            this_e->second -= (0 + this_e->second) / Speed_;
    }
    break;

    case STATIC: {
        if (if_) //do
            this_e->second += Speed_;
        else
            this_e->second -= Speed_;
    }
               break;
    }

    if (this_e->second < 0.f)
        this_e->second = 0.f;
    else if (this_e->second > Maximal_)
        this_e->second = Maximal_;

    return this_e->second;

}

void c_menu::on_paint()
{
    if (!(input::m_blocked = input::get_key<TOGGLE>(VK_INSERT)))
        return;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    auto& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    int w = GetSystemMetrics(SM_CXSCREEN), h = GetSystemMetrics(SM_CYSCREEN);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(-1, -1));
    ImGui::SetNextWindowSizeConstraints(ImVec2(w, h), ImVec2(w, h));
    ImGui::SetNextWindowPos({ 0,0 });
    ImGui::Begin("bg", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_::ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_::ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
    {

        auto Render = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetWindowPos();

    }
    ImGui::End();
    ImGui::PopStyleVar();
    int alpha = Animate("GUI", "Main alpha", GetKeyState(VK_INSERT), 270, 7.5f, INTERP);
    alpha = ImClamp(alpha, 0, 255);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha / 255.f);
    ImGui::SetNextWindowSize(ImVec2(690, 470));
    ImGui::MainBegin("##GUI", nullptr, ImGuiWindowFlags_NoDecoration);
    {
        ImGui::GetIO().OriginalWindowPos = ImGui::GetWindowPos();
        ImGui::SetCursorPos({ 0, 40 });
        ImGui::BeginGroup();//SUBTABS
        {
            if (ImGui::subtab("Q", "Main", subtab == 0))subtab = 0;
            ImGui::SetCursorPosY(80);
            if (ImGui::subtab("F", "Weapons", subtab == 1))subtab = 1;
            ImGui::SetCursorPosY(120);
            if (ImGui::subtab("B", "Anti-aim", subtab == 2))subtab = 2;
            ImGui::SetCursorPosY(160);
            if (ImGui::subtab("D", "Misc", subtab == 3))subtab = 3;
        }
        ImGui::EndGroup();

        ImGui::SetCursorPos({ 25, 430 });
        ImGui::BeginGroup();
        {
            if (ImGui::tab("A", "RAGE", tab == 0))tab = 0; ImGui::SameLine(0, 2);
            if (ImGui::tab("H", "LEGIT", tab == 1))tab = 1; ImGui::SameLine(0, 2);
            if (ImGui::tab("K", "VISUALS", tab == 2))tab = 2; ImGui::SameLine(0, 2);
            if (ImGui::tab("M", "Players", tab == 3))tab = 3; ImGui::SameLine(0, 2);
            if (ImGui::tab("N", "SKINS", tab == 4))tab = 4; ImGui::SameLine(0, 2);
            if (ImGui::tab("Q", "MISC", tab == 5))tab = 5; ImGui::SameLine(0, 2);
            if (ImGui::tab("O", "LUA", tab == 6))tab = 7; ImGui::SameLine(0, 2);
        }
        ImGui::EndGroup();

        ImGui::SetCursorPos({ 128, 40 });
        ImGui::BeginChild("wok sdk v2", ImVec2(560, 370), true, ImGuiWindowFlags_NoDecoration);
        {
            ImGui::PushFont(fonts::mainfont);

            switch (tab)
            {
            case 0: //Rage

                break;
            case 1: //Legit

                break;
            case 2: //Visuals

                break;
            case 3: //Players

                break;
            case 4: //Skins

                break;
            case 5: //Misc
                ImGui::SetCursorPos({ 10, 42 });
                ImGui::MenuChild("General", ImVec2(266, 122));
                {
                    static bool one, two;
                    ImGui::Checkbox("Enable", &one);
                    ImGui::Checkbox("Enable ", &two);
                    const char* text[2] = { "Value", "value" }; static int combo_value = 0;
                    ImGui::Combo("Combobox", &combo_value, text, 2);
                }
                ImGui::EndMenuChild();
                ImGui::SameLine(0, 8);
                ImGui::MenuChild("Movement", ImVec2(266, 108));
                {
                    char texts[24] = "Enq Designed";
                    ImGui::InputText("Textfield", texts, IM_ARRAYSIZE(texts));
                    static int slider_value = 1;
                    ImGui::SliderInt("Slider", &slider_value, 1, 100, "%d%%");
                }
                ImGui::EndMenuChild();
                ImGui::SetCursorPos({ 10, 206 });
                ImGui::MenuChild("BuyBot", ImVec2(266, 98));
                {
                    static float color[4] = { 1.f, 1.f, 1.f, 1.f };
                    ImGui::SetCursorPosX(15); ImGui::Text("Colorpicker"); ImGui::SameLine();
                    ImGui::ColorEdit4("##coloredit", color);

                    ImGui::SetCursorPosX(15); ImGui::Text("Binder"); ImGui::SameLine();
                    ImGui::KeybindButton("Shift", "##eeeee", ImVec2(56, 23));
                }
                ImGui::EndMenuChild();
                ImGui::SetCursorPos({ 284, 192 });
                ImGui::MenuChild("Information", ImVec2(266, 140));
                {
                    static int v = 0;
                    if (ImGui::RadioButton("Default", v == 0)) v = 0;
                    if (ImGui::RadioButton("Skating", v == 1)) v = 1;
                    if (ImGui::RadioButton("Walking", v == 2)) v = 2;
                    if (ImGui::RadioButton("Opposite", v == 3)) v = 3;
                }
                ImGui::EndMenuChild();
                break;
            case 6: //Config

                break;
                //case 7: //Lua

                //    break;
            }


            ImGui::PopFont();
        }
        ImGui::EndChild();
    }
    ImGui::PopStyleVar();
    ImGui::End();
}