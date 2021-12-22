#pragma once

#include <vector>
#include "Command.hpp"
#include "Executor.hpp"

namespace cmd {

template<Command CommandT>
class History {
public:
    void move_forward(Executor const auto& executor)
    {
        if (_current_index < _commands.size()) {
            executor.execute(_commands[_current_index]); // If execute throws, then _current_index won't be modified and that's what we want because the exception means that the command couldn't be applied
            _current_index++;
        }
    }

    void move_backward(Reverter const auto& reverter)
    {
        if (_current_index > 0) {
            reverter.revert(_commands[_current_index - 1]); // If revert throws, then _current_index won't be modified and that's what we want because the exception means that the command couldn't be reverted
            _current_index--;
        }
    }

    void push(const CommandT& command)
    {
        push_impl(command);
    }

    void push(CommandT&& command)
    {
        push_impl(command);
    }

    size_t size() const { return _commands.size(); }

private:
    template<typename T>
    void push_impl(T&& command)
    {
        _commands.resize(_current_index + 1);
        _commands.push_back(std::forward(command));
        _current_index++;
    }

private:
    size_t                _current_index;
    std::vector<CommandT> _commands;
};

} // namespace cmd
