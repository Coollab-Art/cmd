#pragma once

namespace cmd {

template<typename T>
concept Command = requires(T command)
{
    apply(command);
};

} // namespace cmd