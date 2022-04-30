#pragma once

#include <optional>
#include <vector>
#include "Command.hpp"
#include "Executor.hpp"
#include "internal/CircularBuffer.hpp"

namespace cmd {

template<Command CommandT>
class History {
public:
    explicit History(size_t max_size = 1000)
        : _commands{max_size}
    {}

    History(History&&) noexcept = default;
    History& operator=(History&&) noexcept = default;

    History clone() const
    {
        History copy{*this};
        copy.unsafe_set_next_command_to_execute( // To properly "copy" the iterator
            this->unsafe_get_next_command_to_execute());
        return copy;
    }

    template<typename ExecutorT>
    requires Executor<ExecutorT, CommandT>
    void move_forward(ExecutorT& executor)
    {
        if (_next_command_to_execute)
        {
            if (*_next_command_to_execute != _commands.end())
            {
                executor.execute(**_next_command_to_execute); // If execute throws, then _next_command_to_execute won't be modified and that's what we want because the exception means that the command couldn't be applied
                (*_next_command_to_execute)++;
            }
        }
    }

    template<typename ReverterT>
    requires Reverter<ReverterT, CommandT>
    void move_backward(ReverterT& reverter)
    {
        if (_next_command_to_execute)
        {
            if (*_next_command_to_execute != _commands.begin())
            {
                reverter.revert(*std::prev(*_next_command_to_execute)); // If revert throws, then _next_command_to_execute won't be modified and that's what we want because the exception means that the command couldn't be reverted
                (*_next_command_to_execute)--;
            }
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

    auto size() const -> size_t { return _commands.size(); }

    auto max_size() const -> size_t { return _commands.max_size(); }

    /// If you reduce max_size, we will have to delete some commits from the history.
    /// We start deleting commits that are furthest away in the future, until we reach one commit before the current one.
    /// This means that you will be able to move forward at least once after setting max_size (unless you set it to 0, or you were already at the most forward point in your history)
    /// If there is still a need to delete commits, we will then start deleting the commits that are furthest away in the past.
    /// NB: this choice was done because it was the simplest to implement, but we could consider adding other policies of which commits to keep.
    void set_max_size(size_t new_max_size)
    {
        if (_next_command_to_execute)
        {
            _commands.set_max_size_and_preserve_given_iterator(new_max_size,
                                                               *_next_command_to_execute);
            if (new_max_size == 0)
            {
                _next_command_to_execute.reset();
            }
        }
        else
        {
            _commands.set_max_size(new_max_size);
        }
    }

    /// Removes commits until the size of the history is <= max_size
    void shrink(size_t max_size)
    {
        if (_next_command_to_execute)
        {
            _commands.shrink_and_preserve_given_iterator(max_size,
                                                         *_next_command_to_execute);
            if (max_size == 0)
            {
                _next_command_to_execute.reset();
            }
        }
        else
        {
            // No need to do anything since _commands is already empty
            assert(_commands.is_empty());
        }
    }

    auto underlying_container() const -> const std::list<CommandT>& { return _commands.underlying_container(); }
    auto underlying_container() -> std::list<CommandT>& { return _commands.underlying_container(); }

    auto current_command_iterator() const { return _next_command_to_execute; }

    // Exposed for serialization purposes. Don't use this unless you have a really good reason to.
    void unsafe_set_next_command_to_execute(std::optional<size_t> index)
    {
        if (index)
        {
            _next_command_to_execute = _commands.begin();
            for (size_t i = 0; i < *index; ++i)
            {
                (*_next_command_to_execute)++;
            }
        }
        else
        {
            _next_command_to_execute.reset();
        }
    }

    // Exposed for serialization purposes. Don't use this unless you have a really good reason to.
    auto unsafe_get_next_command_to_execute() const -> std::optional<size_t>
    {
        if (_next_command_to_execute)
        {
            size_t dist{0};
            for (auto it = _commands.begin(); it != *_next_command_to_execute; ++it)
            {
                ++dist;
            }
            return dist;
        }
        else
        {
            return std::nullopt;
        }
    }

private:
    template<typename T>
    void push_impl(T&& command)
    {
        if (_next_command_to_execute)
        {
            _commands.erase_all_starting_at(*_next_command_to_execute);
        }
        _commands.push_back(std::forward<T>(command));
        _next_command_to_execute = _commands.end();
    }

    History(const History&) = default;            // Use `clone()` instead
    History& operator=(const History&) = default; // if you want a copy of your history

private:
    internal::CircularBuffer<CommandT>                                   _commands;
    std::optional<typename internal::CircularBuffer<CommandT>::iterator> _next_command_to_execute{};
};

} // namespace cmd
