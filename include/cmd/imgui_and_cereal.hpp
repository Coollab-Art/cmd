#pragma once

#include <cmd/cereal.hpp>
#include <cmd/cmd.hpp>
#include <cmd/imgui.hpp>

namespace cmd {

template<Command CommandT>
class HistoryWithUiAndSerialization {
public:
    template<typename CommandToString>
    void imgui_show(CommandToString&& command_to_string)
    {
        _ui.imgui_show(_history, std::forward<CommandToString>(command_to_string));
    }

    auto imgui_max_size() -> bool { return _ui.imgui_max_size(_history); }

    // ---Boilerplate to replicate the API of an History---
    explicit HistoryWithUiAndSerialization(size_t max_size = 1000)
        : _history{max_size}
    {}
    void push(const CommandT& command) { _ui.push(_history, command); }
    void push(CommandT&& command) { _ui.push(_history, std::move(command)); }
    template<typename ExecutorT>
    requires Executor<ExecutorT, CommandT>
    void move_forward(ExecutorT& executor) { _history.move_forward(executor); }
    template<typename ReverterT>
    requires Reverter<ReverterT, CommandT>
    void move_backward(ReverterT& reverter) { _history.move_backward(reverter); }
    // ---End of boilerplate---

private:
    History<CommandT>       _history;
    UiForHistory            _ui{};
    SerializationForHistory _serialization{};

private:
    friend class cereal::access;

    template<class Archive>
    void save(Archive& archive) const
    {
        _serialization.save(archive, _history);
    }

    template<class Archive>
    void load(Archive& archive)
    {
        _serialization.load(archive, _history);
    }
};

} // namespace cmd