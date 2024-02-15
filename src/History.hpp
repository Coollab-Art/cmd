#pragma once
#include <optional>
#include <vector>
#include "Command.hpp"
#include "Executor.hpp"
#include "internal/CircularBuffer.hpp"

namespace cmd {

template<CommandC CommandT>
class History {
public:
    using CommandGroup = std::vector<CommandT>;

    explicit History(size_t max_size = 1000)
        : _command_groups{max_size}
    {}

    History(History&&) noexcept            = default;
    History& operator=(History&&) noexcept = default;

    History clone() const
    {
        History copy{*this};
        copy.unsafe_set_next_command_group_to_execute( // To properly "copy" the iterator
            this->unsafe_get_next_command_group_to_execute()
        );
        return copy;
    }

    template<typename ExecutorT>
        requires ExecutorC<ExecutorT, CommandT>
    void move_forward(ExecutorT& executor)
    {
        if (_next_command_group_to_execute
            && *_next_command_group_to_execute != _command_groups.end())
        {
            for (auto const& command : **_next_command_group_to_execute)
                executor.execute(command); // TODO if one of the commands throws, this can mess up the state. We should probably provide the strong guarantee.
            (*_next_command_group_to_execute)++;
        }
        _can_try_to_merge_next_command        = false;
        _should_put_next_command_in_new_group = true;
    }

    template<typename ReverterT>
        requires ReverterC<ReverterT, CommandT>
    void move_backward(ReverterT& reverter)
    {
        if (_next_command_group_to_execute
            && *_next_command_group_to_execute != _command_groups.begin())
        {
            // We want to undo in the reverse order compared to when we do
            CommandGroup const& group = *std::prev(*_next_command_group_to_execute);
            for (auto it = group.rbegin(); it != group.rend(); ++it)
                reverter.revert(*it); // TODO if one of the commands throws, this can mess up the state. We should probably provide the strong guarantee.
            (*_next_command_group_to_execute)--;
        }
        _can_try_to_merge_next_command        = false;
        _should_put_next_command_in_new_group = true;
    }

    template<typename MergerT>
        requires MergerC<MergerT, CommandT>
    void push(const CommandT& command, const MergerT& merger)
    {
        push_impl(command, merger);
    }

    template<typename MergerT>
        requires MergerC<MergerT, CommandT>
    void push(CommandT&& command, const MergerT& merger)
    {
        push_impl(std::move(command), merger);
    }

    /// The history is eager to merge commands: it will try to do it unless you explicitly tell it not to
    void dont_merge_next_command() const { _can_try_to_merge_next_command = false; }
    void start_new_commands_group() { _should_put_next_command_in_new_group = true; }

    auto size() const -> size_t { return _command_groups.size(); }
    auto max_size() const -> size_t { return _command_groups.max_size(); }

    /// If you reduce max_size, we will have to delete some commits from the history.
    /// We start deleting commits that are furthest away in the future, until we reach one commit before the current one.
    /// This means that you will be able to move forward at least once after setting max_size (unless you set it to 0, or you were already at the most forward point in your history)
    /// If there is still a need to delete commits, we will then start deleting the commits that are furthest away in the past.
    /// NB: this choice was done because it was the simplest to implement, but we could consider adding other policies of which commits to keep.
    void set_max_size(size_t new_max_size)
    {
        if (_next_command_group_to_execute)
        {
            _command_groups.set_max_size_and_preserve_given_iterator(new_max_size, *_next_command_group_to_execute);
            if (new_max_size == 0)
            {
                _next_command_group_to_execute.reset();
            }
        }
        else
        {
            _command_groups.set_max_size(new_max_size);
        }
    }

    /// Removes commits until the size of the history is <= max_size
    void shrink(size_t max_size)
    {
        if (_next_command_group_to_execute)
        {
            _command_groups.shrink_and_preserve_given_iterator(max_size, *_next_command_group_to_execute);
            if (max_size == 0)
            {
                _next_command_group_to_execute.reset();
            }
        }
        else
        {
            // No need to do anything since _commands is already empty
            assert(_command_groups.is_empty());
        }
    }

    auto underlying_container() const -> std::list<CommandGroup> const& { return _command_groups.underlying_container(); }
    auto underlying_container() -> std::list<CommandGroup>& { return _command_groups.underlying_container(); }

    auto current_command_group_iterator() const { return _next_command_group_to_execute; }

    // Exposed for serialization purposes. Don't use this unless you have a really good reason to.
    void unsafe_set_next_command_group_to_execute(std::optional<size_t> index)
    {
        if (index)
        {
            _next_command_group_to_execute = _command_groups.begin();
            for (size_t i = 0; i < *index; ++i)
            {
                (*_next_command_group_to_execute)++;
            }
        }
        else
        {
            _next_command_group_to_execute.reset();
        }
    }

    // Exposed for serialization purposes. Don't use this unless you have a really good reason to.
    auto unsafe_get_next_command_group_to_execute() const -> std::optional<size_t>
    {
        if (_next_command_group_to_execute)
        {
            size_t dist{0};
            for (auto it = _command_groups.begin(); it != *_next_command_group_to_execute; ++it)
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
    template<typename CommandType, typename MergerType> // CommandType instead of CommandT to not override CommandT which is already the template parameter of the whole class; CommandT and CommandType need to be different otherwise perfect forwarding won't kick in
    void push_impl(CommandType&& command, const MergerType& merger)
    {
        auto const push_the_command = [&]() {
            if (_should_put_next_command_in_new_group
                || _command_groups.is_empty())
            {
                _command_groups.push_back(CommandGroup{});
            }
            _should_put_next_command_in_new_group = false;
            _command_groups.back().push_back(std::forward<CommandType>(command));
        };

        if (_next_command_group_to_execute)
        {
            _command_groups.erase_all_starting_at(*_next_command_group_to_execute);
        }
        if (!_command_groups.is_empty()
            && _can_try_to_merge_next_command)
        {
            auto&      last_command = _command_groups.back().back(); // The second back() is safe because we should never have empty command groups.
            auto const merged       = merger.merge(last_command, command);
            if (merged)
                last_command = *merged;
            else
                push_the_command();
        }
        else
        {
            push_the_command();
        }
        _next_command_group_to_execute = _command_groups.end();
        _can_try_to_merge_next_command = true;
    }

    History(const History&)            = default; // Use `clone()` instead
    History& operator=(const History&) = default; // if you really want a copy of your history

private:
    internal::CircularBuffer<CommandGroup>                                   _command_groups;
    std::optional<typename internal::CircularBuffer<CommandGroup>::iterator> _next_command_group_to_execute{};
    mutable bool                                                             _can_try_to_merge_next_command{false};
    bool                                                                     _should_put_next_command_in_new_group{true};
};

} // namespace cmd