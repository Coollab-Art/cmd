#pragma once

#include <cereal/types/list.hpp>
#include <cereal/types/optional.hpp>
#include "cmd.hpp"

namespace cmd {

struct SerializationForHistory {
    size_t max_saved_size{100};

    template<class Archive, Command CommandT>
    void save(Archive& archive, const History<CommandT>& history) const
    {
        auto copy = history.clone(); // We make a copy because we don't want to shrink the actual history,
        copy.shrink(max_saved_size); // in case it is still used even after being serialized
                                     // TODO(JF) performance could be improved by only copying the commits that we want to serialize. Currently we copy all the commits and then discard many of them in shrink()
        archive(cereal::make_nvp("History", copy),
                cereal::make_nvp("Max saved size", max_saved_size));
    }

    template<class Archive, Command CommandT>
    void load(Archive& archive, History<CommandT>& history)
    {
        archive(history,
                max_saved_size);
    }
};

template<Command CommandT>
class HistoryWithSerialization {
public:
    // ---Boilerplate to replicate the API of an History---
    explicit HistoryWithSerialization(size_t max_size = 1000)
        : _history{max_size} {}
    template<typename MergerT>
    requires Merger<MergerT, CommandT>
    void push(const CommandT& command, const MergerT& merger) { _history.push(command, merger); }
    template<typename MergerT>
    requires Merger<MergerT, CommandT>
    void push(CommandT&& command, const MergerT& merger) { _history.push(std::move(command), merger); }
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
    History<CommandT>       _history;
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

namespace cereal {

template<class Archive, cmd::Command CommandT>
void save(Archive& archive, const cmd::History<CommandT>& history)
{
    archive(cereal::make_nvp("Commits", history.underlying_container()),
            cereal::make_nvp("Position in history", history.unsafe_get_next_command_to_execute()),
            cereal::make_nvp("Max size", history.max_size()));
}

template<class Archive, cmd::Command CommandT>
void load(Archive& archive, cmd::History<CommandT>& history)
{
    std::optional<size_t> next_command_index;
    std::size_t           max_size;
    archive(history.underlying_container(),
            next_command_index,
            max_size);
    history.unsafe_set_next_command_to_execute(next_command_index);
    history.set_max_size(max_size);
}

} // namespace cereal
