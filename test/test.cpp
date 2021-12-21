#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <cmd/cmd.hpp>

struct Command_SayHello {
};

struct Command_SayWorld {
};

class Executor {
public:
    std::string message;

    void operator()(Command_SayHello)
    {
        message = "Hello";
    }

    void operator()(Command_SayWorld)
    {
        message = "World";
    }
};

template<typename Executor, typename Command>
requires cmd::Executor<Executor, Command>
void execute(Executor& executor, Command command)
{
    executor(command);
}

TEST_CASE("Executor and Command concepts")
{
    Executor executor{};
    execute(executor, Command_SayHello{});
    CHECK(executor.message == "Hello");
    execute(executor, Command_SayWorld{});
    CHECK(executor.message == "World");
}