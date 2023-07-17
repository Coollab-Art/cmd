#pragma once
#include <vector>
#include "Command.hpp"
#include "Executor.hpp"

namespace cmd {

template<CommandC CommandT>
class ExecutorChain {
public:
    ExecutorChain(std::vector<Executor<CommantT>>&& chain)
        : _chain{std::move(chain)}
    {
    }

    void execute(CommandT const& command) const
    {
        for (auto const& executor : _chain)
            executor.execute();
    }

private:
    std::vector<Executor<CommantT>> _chain;
};

} // namespace cmd