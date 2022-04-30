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

    bool imgui_max_size(const char* label = "Max size")
    {
        static_assert(sizeof(_uncommited_max_size) == 8, "The ImGui widget expects a u64 integer");
        ImGui::InputScalar(label, ImGuiDataType_U64, &_uncommited_max_size);
        ImGui::Text("(%s)", internal::size_as_string<CommandT>(static_cast<float>(_uncommited_max_size)).c_str());
        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            History<CommandT>::set_max_size(_uncommited_max_size);
            return true;
        }
        return false;
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