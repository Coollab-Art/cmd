#pragma once

#include <iomanip>
#include <sstream>
#include <string>
#include "cmd.hpp"

namespace cmd {

namespace internal {

template<typename T>
auto size_as_string(float multiplier) -> std::string
{
    const auto total_size_in_kilobytes = static_cast<float>(sizeof(T))
                                         * multiplier
                                         / 1000.f;
    // return std::format("{:.2f} Mb", total_size_in_megabytes); // Compilers don't support std::format() just yet :(
    std::stringstream stream;
    stream << std::fixed << std::setprecision(1) << total_size_in_kilobytes << " Kb";
    return stream.str();
}

inline void imgui_help_marker(const char* text)
{
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(text);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

struct InputResult {
    bool is_item_deactivated_after_edit;
    bool is_item_active;
};

template<Command CommandT>
inline auto imgui_input_history_size(size_t* value, size_t previous_value, int uuid)
{
    static_assert(sizeof(size_t) == 8, "The ImGui widget expects a u64 integer");
    ImGui::PushID(uuid);
    ImGui::SetNextItemWidth(12.f + ImGui::CalcTextSize(std::to_string(*value).c_str()).x); // Adapt the widget size to exactly fit the text input
    ImGui::InputScalar("", ImGuiDataType_U64, value);
    ImGui::PopID();
    const auto ret = InputResult{
        .is_item_deactivated_after_edit = ImGui::IsItemDeactivatedAfterEdit(),
        .is_item_active                 = ImGui::IsItemActive(),
    };
    ImGui::SameLine();
    ImGui::Text("commits (%s)", internal::size_as_string<CommandT>(static_cast<float>(*value)).c_str());
    if (*value != previous_value)
    {
        ImGui::TextDisabled("Previously: %zu", previous_value);
    }
    return ret;
}

} // namespace internal

struct UiForHistory {
    bool   should_scroll_to_current_commit{true};
    size_t uncommited_max_size{};

    template<Command CommandT, typename MergerT>
        requires Merger<MergerT, CommandT>
    void push(History<CommandT>& history, const CommandT& command, const MergerT& merger)
    {
        should_scroll_to_current_commit = true;
        history.push(command, merger);
    }

    template<Command CommandT, typename MergerT>
        requires Merger<MergerT, CommandT>
    void push(History<CommandT>& history, CommandT&& command, const MergerT& merger)
    {
        should_scroll_to_current_commit = true;
        history.push(std::move(command), merger);
    }

    template<Command CommandT, typename ExecutorT>
        requires Executor<ExecutorT, CommandT>
    void move_forward(History<CommandT>& history, ExecutorT& executor)
    {
        should_scroll_to_current_commit = true;
        history.move_forward(executor);
    }

    template<Command CommandT, typename ReverterT>
        requires Reverter<ReverterT, CommandT>
    void move_backward(History<CommandT>& history, ReverterT& reverter)
    {
        should_scroll_to_current_commit = true;
        history.move_backward(reverter);
    }

    template<Command CommandT, typename CommandToString>
    void imgui_show(const History<CommandT>& history, CommandToString&& command_to_string)
    {
        const auto& commands                 = history.underlying_container();
        bool        drawn                    = false;
        const auto  draw_position_in_history = [&]() {
            ImGui::Separator();
            if (should_scroll_to_current_commit)
            {
                ImGui::SetScrollHereY(1.0f);
                should_scroll_to_current_commit = false;
            }
        };
        for (auto it = commands.cbegin(); it != commands.cend(); ++it)
        {
            if (it == history.current_command_iterator())
            {
                draw_position_in_history();
                drawn = true;
            }
            ImGui::Text("%s", command_to_string(*it).c_str());
        }
        if (!drawn)
        {
            draw_position_in_history();
        }
    }

    template<Command CommandT>
    auto imgui_max_size(History<CommandT>& history, std::function<void(const char*)> help_marker) -> bool
    {
        ImGui::Text("History maximum size");
        help_marker(
            "This is how far you can go back in the history, "
            "i.e. the number of undo you can perform."
        );
        const auto res = internal::imgui_input_history_size<CommandT>(
            &uncommited_max_size,
            history.max_size(),
            1354321
        );
        if (res.is_item_deactivated_after_edit)
        {
            history.set_max_size(uncommited_max_size);
        }
        if (!res.is_item_active) // Sync with the current max_size if we are not editing // Must be after the check for IsItemDeactivatedAfterEdit() otherwise the value can't be set properly when we finish editing
        {
            uncommited_max_size = history.max_size();
        }
        if (uncommited_max_size < history.size())
        {
            ImGui::TextColored(
                {1.f, 1.f, 0.f, 1.f},
                "Some commits will be erased because you are reducing the size of the history!\nThe current size is %zu.",
                history.size()
            );
        }
        return res.is_item_deactivated_after_edit;
    }
};

template<Command CommandT>
class HistoryWithUi {
public:
    template<typename CommandToString>
    void imgui_show(CommandToString&& command_to_string)
    {
        _ui.imgui_show(_history, std::forward<CommandToString>(command_to_string));
    }

    auto imgui_max_size() -> bool { return _ui.imgui_max_size(_history); }

    // ---Boilerplate to replicate the API of an History---
    explicit HistoryWithUi(size_t max_size = 1000)
        : _history{max_size} {}
    template<typename MergerT>
        requires Merger<MergerT, CommandT>
    void push(const CommandT& command, const MergerT& merger)
    {
        _ui.push(_history, command, merger);
    }
    template<typename MergerT>
        requires Merger<MergerT, CommandT>
    void push(CommandT&& command, const MergerT& merger)
    {
        _ui.push(_history, std::move(command), merger);
    }
    template<typename ExecutorT>
        requires Executor<ExecutorT, CommandT>
    void move_forward(ExecutorT& executor)
    {
        _ui.move_forward(_history, executor);
    }
    template<typename ReverterT>
        requires Reverter<ReverterT, CommandT>
    void move_backward(ReverterT& reverter)
    {
        _ui.move_backward(_history, reverter);
    }
    void dont_merge_next_command() const { _history.dont_merge_next_command(); }
    // ---End of boilerplate---

private:
    History<CommandT> _history;
    UiForHistory      _ui{};
};

} // namespace cmd