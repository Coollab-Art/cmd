#pragma once

#include <cereal/types/list.hpp>
#include <cereal/types/optional.hpp>
#include "cmd.hpp"

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
