#pragma once

#include "cmd.hpp"

namespace cmd {

template<typename CommandToString, Command CommandT>
void imgui_show_history(History<CommandT> history, const CommandToString& command_to_string)
{
    const auto& commands = history.underlying_container();
    size_t      i        = 0u;
    for (const auto& command : commands) {
        if (i == history.current_command_index()) {
            ImGui::Separator();
        }
        ImGui::Text("%s", command_to_string(command).c_str());
        i++;
    }
    if (i == history.current_command_index()) {
        ImGui::Separator();
    }
}

} // namespace cmd