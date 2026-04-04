#include "DebugWindow.hpp"

#include <SGF.hpp>

namespace SGF {
    DebugWindow::DebugWindow(const std::string& name) : windowName(name) {}
    DebugWindow::DebugWindow() {}
    DebugWindow::~DebugWindow() {}
    
    void DebugWindow::AddMessage(std::string &&message) {
        messages.emplace_back(message);
    }
    void DebugWindow::AddMessage(const std::string& message) {
        messages.emplace_back(message);
    }
    void DebugWindow::Draw() {
        ImGui::Begin(windowName.c_str());
        for (const auto& message : messages) {
            ImGui::Text("%s", message.c_str());
        }
        ImGui::End();
        messages.clear();
    }
}