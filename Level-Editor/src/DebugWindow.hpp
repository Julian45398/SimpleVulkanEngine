#pragma once

#include <string>
#include <vector>

namespace SGF {
    class DebugWindow {
    public:
        DebugWindow(const std::string& name);
        DebugWindow();
        ~DebugWindow();
    
        void AddMessage(std::string &&message);
        void AddMessage(const std::string& message);
        void Draw();
    private:
        std::string windowName;
        std::vector<std::string> messages;
    };
}