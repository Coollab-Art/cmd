#pragma once

#include <utility>
#include <vector>
#include "Command.hpp"
#include "Executor.hpp"

namespace cmd {

template<Command CommandT>
class History {
public:
    template<typename ExecutorT>
    requires Executor<ExecutorT, CommandT>
    void move_forward(ExecutorT& executor)
    {
        if (_current_index < _commands.size()) {
            executor.execute(_commands[_current_index]); // If execute throws, then _current_index won't be modified and that's what we want because the exception means that the command couldn't be applied
            _current_index++;
        }
    }

    template<typename ReverterT>
    requires Reverter<ReverterT, CommandT>
    void move_backward(ReverterT& reverter)
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

    auto underlying_container() const -> const std::vector<CommandT>& { return _commands; }
    auto underlying_container() -> std::vector<CommandT>& { return _commands; }

    auto current_command_index() const { return _current_index; }

private:
    template<typename T>
    void push_impl(T&& command)
    {
        _commands.resize(_current_index + 1);
        _commands.push_back(std::forward<T>(command));
        _current_index++;
    }

private:
    size_t                _current_index = 0;
    std::vector<CommandT> _commands;
};

} // namespace cmd
