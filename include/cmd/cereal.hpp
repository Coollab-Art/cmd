#pragma once

#include <cereal/types/list.hpp>
#include "cmd.hpp"

namespace cereal {

template<class Archive, cmd::Command CommandT>
void serialize(Archive& archive, cmd::History<CommandT>& history)
{
    archive(history.underlying_container()
            // ,history.unsafe_current_command_index_ref()
    );
}

} // namespace cereal
