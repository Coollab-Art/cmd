#pragma once

#include <vector>
#include "Command.hpp"
#include "Executor.hpp"

namespace cmd {

template<Command CommandT>
class History {
public:
    explicit History(size_t max_size)
        : _max_size{max_size}
    {
    }

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

    virtual void push(const CommandT& command)
    {
        push_impl(command);
    }

    virtual void push(CommandT&& command)
    {
        push_impl(std::move(command));
    }

    auto max_size() const -> size_t
    {
        return _max_size;
    }

    void set_max_size(size_t new_size)
    {
        _max_size = new_size;
    }

    auto underlying_container() const -> const std::vector<CommandT>& { return _commands; }
    auto underlying_container() -> std::vector<CommandT>& { return _commands; }

    auto current_command_iterator() const { return _commands.cbegin() + _current_index; }

    // Exposed for serialization purposes. Don't use this unless you have a really good reason to.
    auto unsafe_current_command_index_ref() -> size_t& { return _current_index; }

private:
    template<typename T>
    void push_impl(T&& command)
    {
        _commands.resize(_current_index);
        _commands.push_back(std::forward<T>(command));
        _current_index++;
    }

private:
    size_t                _current_index{0};
    size_t                _max_size;
    std::vector<CommandT> _commands;
};

} // namespace cmd
