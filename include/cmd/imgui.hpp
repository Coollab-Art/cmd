#pragma once

#include <chrono>
#include <sstream>
#include <string>
#include "cmd.hpp"

namespace cmd {

namespace internal {

template<typename T>
auto size_as_string(float multiplier) -> std::string
{
    const auto total_size_in_megabytes = static_cast<float>(sizeof(T))
                                         * multiplier
                                         / 1'000'000.f;
    // return std::format("{:.2f} Mb", total_size_in_megabytes); // Compilers don't support std::format() just yet :(
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2) << total_size_in_megabytes << " Mb";
    return stream.str();
}

} // namespace internal

struct UiForHistory {
    std::chrono::steady_clock::time_point _last_push_date{std::chrono::steady_clock::now()};
    size_t                                _uncommited_max_size;

    template<Command CommandT>
    void push(History<CommandT>& history, const CommandT& command)
    {
        _last_push_date = std::chrono::steady_clock::now();
        history.push(command);
    }

    template<Command CommandT>
    void push(History<CommandT>& history, CommandT&& command)
    {
        _last_push_date = std::chrono::steady_clock::now();
        history.push(std::move(command));
    }

    template<typename CommandToString, Command CommandT>
    void imgui_show(const History<CommandT>& history, CommandToString&& command_to_string)
    {
        const auto& commands = history.underlying_container();
        bool        drawn    = false;
        for (auto it = commands.cbegin(); it != commands.cend(); ++it)
        {
            if (it == history.current_command_iterator())
            {
                drawn = true;
                ImGui::Separator();
                using namespace std::chrono_literals;
                // if (time_since_last_push() < 500ms)
                {
                    ImGui::SetScrollHereY(1.0f);
                }
            }
            ImGui::Text("%s", command_to_string(*it).c_str());
        }
        if (!drawn)
        {
            ImGui::Separator();
        }
    }

    template<Command CommandT>
    auto imgui_max_size(History<CommandT>& history) -> bool
    {
        static_assert(sizeof(_uncommited_max_size) == 8, "The ImGui widget expects a u64 integer");
        ImGui::Text("Maximum history size");
        ImGui::PushID(1354321);
        ImGui::SetNextItemWidth(12.f + ImGui::CalcTextSize(std::to_string(_uncommited_max_size).c_str()).x); // Adapt the widget size to exactly fit the text input
        ImGui::InputScalar("", ImGuiDataType_U64, &_uncommited_max_size);
        const bool has_changed_max_size = [&]() {
            if (ImGui::IsItemDeactivatedAfterEdit())
            {
                history.set_max_size(_uncommited_max_size);
                return true;
            }
            else
            {
                return false;
            }
        }();
        if (!ImGui::IsItemActive()) // Sync with the current max_size if we are not editing // Must be after the check for IsItemDeactivatedAfterEdit() otherwise the value can't be set properly when we finish editing
        {
            _uncommited_max_size = history.max_size();
        }
        ImGui::PopID();
        ImGui::SameLine();
        ImGui::Text("commits (%s)", internal::size_as_string<CommandT>(static_cast<float>(_uncommited_max_size)).c_str());
        if (_uncommited_max_size != history.max_size())
        {
            ImGui::TextDisabled("Previously: %lld", history.max_size());
        }
        if (_uncommited_max_size < history.size())
        {
            ImGui::TextColored({1.f, 1.f, 0.f, 1.f},
                               "Some commits will be erased because you are reducing the size of the history!\nThe current size is %lld.",
                               history.size());
        }
        return has_changed_max_size;
    }

    auto time_since_last_push() const { return std::chrono::steady_clock::now() - _last_push_date; }
};

template<Command CommandT>
class HistoryWithUi {
public:
    template<typename CommandToString>
    void imgui_show(CommandToString&& command_to_string)
    {
        _ui.imgui_show(_history, std::forward<CommandToString>(command_to_string));
    }

    auto imgui_max_size() -> bool
    {
        return _ui.imgui_max_size(_history);
    }

    // auto max_saved_size() const -> size_t { return _max_saved_size; }

    // Exposed for serialization purposes. Don't use this unless you have a really good reason to.
    // void unsafe_set_uncommited_max_size(size_t size) { _uncommited_max_size = size; }

    // ---Boilerplate to replicate the API of an History---
    explicit HistoryWithUi(size_t max_size = 1000)
        : _history{max_size}
    {}
    void push(const CommandT& command) { _ui.push(_history, command); }
    void push(CommandT&& command) { _ui.push(_history, std::move(command)); }
    template<typename ExecutorT>
    requires Executor<ExecutorT, CommandT>
    void move_forward(ExecutorT& executor)
    {
        _history.move_forward(executor);
    }
    template<typename ReverterT>
    requires Reverter<ReverterT, CommandT>
    void move_backward(ReverterT& reverter)
    {
        _history.move_backward(reverter);
    }
    // ---End of boilerplate---

private:
    // size_t       _max_saved_size{5};
    History<CommandT> _history;
    UiForHistory      _ui{};
};

} // namespace cmd