#pragma once

#include "cmd.hpp"

namespace cmd {

template<typename CommandToString, Command CommandT>
void imgui_show_history(const History<CommandT>& history, const CommandToString& command_to_string)
{
    const auto& commands = history.underlying_container();
    bool        drawn    = false;
    for (auto it = commands.cbegin(); it != commands.cend(); ++it) {
        if (it == history.current_command_iterator()) {
            drawn = true;
            ImGui::Separator();
        }
        ImGui::Text("%s", command_to_string(*it).c_str());
    }
    if (!drawn) {
        ImGui::Separator();
    }
    using namespace std::chrono_literals;
    if (history.time_since_last_push() < 100ms) {
        ImGui::SetScrollHereY(1.0f);
    }
}

} // namespace cmd