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

template<Command CommandT>
class HistoryWithUi : public History<CommandT> {
public:
    explicit HistoryWithUi(size_t max_size)
        : History<CommandT>{max_size}
        , _uncommited_max_size{max_size}
    {}

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
        for (auto it = commands.cbegin(); it != commands.cend(); ++it)
        {
            if (it == History<CommandT>::current_command_iterator())
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

    auto imgui_max_size(const char* label = "Max size") -> bool
    {
        static_assert(sizeof(_uncommited_max_size) == 8, "The ImGui widget expects a u64 integer");
        ImGui::Text("%s", label);
        ImGui::PushID(1354321);
        ImGui::SetNextItemWidth(12.f + ImGui::CalcTextSize(std::to_string(_uncommited_max_size).c_str()).x); // Adapt the widget size to exactly fit the text input
        ImGui::InputScalar("", ImGuiDataType_U64, &_uncommited_max_size);
        const auto has_changed_max_size = [&]() {
            if (ImGui::IsItemDeactivatedAfterEdit())
            {
                History<CommandT>::set_max_size(_uncommited_max_size);
                return true;
            }
            else
            {
                return false;
            }
        }();
        ImGui::PopID();
        ImGui::SameLine();
        ImGui::Text("commits (%s)", internal::size_as_string<CommandT>(static_cast<float>(_uncommited_max_size)).c_str());
        if (_uncommited_max_size != History<CommandT>::max_size())
        {
            ImGui::TextDisabled("Previously: %lld", History<CommandT>::max_size());
        }
        if (_uncommited_max_size < History<CommandT>::size())
        {
            ImGui::TextColored({1.f, 1.f, 0.f, 1.f},
                               "Some commits will be erased because you are reducing the size of the history!\nThe current size is %lld.",
                               History<CommandT>::size());
        }
        return has_changed_max_size;
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
    size_t                                _uncommited_max_size;
};

} // namespace cmd