#pragma once

#include <cereal/types/list.hpp>
#include <cereal/types/optional.hpp>
#include "cmd.hpp"

namespace cereal {

template<class Archive, cmd::Command CommandT>
void save(Archive& archive, const cmd::History<CommandT>& history)
{
    archive(history.underlying_container(),
            history.unsafe_get_next_command_to_execute());
}

template<class Archive, cmd::Command CommandT>
void load(Archive& archive, cmd::History<CommandT>& history)
{
    std::optional<size_t> next_command_index;
    archive(history.underlying_container(),
            next_command_index);
    history.unsafe_set_next_command_to_execute(next_command_index);
}

} // namespace cereal
