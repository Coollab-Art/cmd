#pragma once

/// An Executor is the user-defined class responsible for executing commands.
/// This is where they should put (or at least dispatch) all their logic.

#include "Command.hpp"

namespace cmd {

template<typename ExecutorT, typename CommandT>
concept Executor = requires(ExecutorT executor, CommandT command)
{
    executor.execute(command);
};

template<typename ReverterT, typename CommandT>
concept Reverter = requires(ReverterT reverter, CommandT command)
{
    reverter.revert(command);
};

} // namespace cmd