#pragma once

#include <chrono>
#include "cmd.hpp"

namespace cmd {

template<Command CommandT>
class HistoryWithUi : public History<CommandT> {
public:
    explicit HistoryWithUi(size_t max_size)
        : History<CommandT>{max_size}
    {
    }

    void push(const CommandT& command) override
    {
        push_impl(command);
    }

    void push(CommandT&& command) override
    {
        push_impl(std::move(command));
    }

    template<typename CommandToString>
    void imgui_show(CommandToString&& command_to_string)
    {
        const auto& commands = History<CommandT>::underlying_container();
        bool        drawn    = false;
        for (auto it = commands.cbegin(); it != commands.cend(); ++it) {
            if (it == History<CommandT>::current_command_iterator()) {
                drawn = true;
                ImGui::Separator();
            }
            ImGui::Text("%s", command_to_string(*it).c_str());
        }
        if (!drawn) {
            ImGui::Separator();
        }
        using namespace std::chrono_literals;
        if (time_since_last_push() < 500ms) {
            ImGui::SetScrollHereY(1.0f);
        }
    }

private:
    template<typename T>
    void push_impl(T&& command)
    {
        History<CommandT>::push(std::forward<T>(command));

        _last_push_date = std::chrono::steady_clock::now();
    }

    auto time_since_last_push() const { return std::chrono::steady_clock::now() - _last_push_date; }

private:
    std::chrono::steady_clock::time_point _last_push_date{std::chrono::steady_clock::now()};
};
    }
}

} // namespace cmd