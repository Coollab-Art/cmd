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

    void execute(Command_SayHello)
    {
        message = "Hello";
    }

    void execute(Command_SayWorld)
    {
        message = "World";
    }
};

template<typename Executor, typename Command>
requires cmd::Executor<Executor, Command>
void execute(Executor& executor, Command command)
{
    executor.execute(command);
}

TEST_CASE("Executor and Command concepts")
{
    Executor executor{};
    execute(executor, Command_SayHello{});
    CHECK(executor.message == "Hello");
    execute(executor, Command_SayWorld{});
    CHECK(executor.message == "World");
}

struct Command_SetInt {
    int new_value;
    int previous_value;
};

class Executor_SetInt {
public:
    int value() const
    {
        return _value;
    }

    void set_value(int n, cmd::History<Command_SetInt>& history)
    {
        const auto command = Command_SetInt{.new_value      = n,
                                            .previous_value = _value};
        _value             = n;
        history.push(command);
    }

    void execute(Command_SetInt command)
    {
        _value = command.new_value;
    }

    void revert(Command_SetInt command)
    {
        _value = command.previous_value;
    }

private:
    int _value = 0;
};

TEST_CASE("History")
{
    Executor_SetInt              executor{};
    cmd::History<Command_SetInt> history{};
    REQUIRE(executor.value() == 0);

    executor.set_value(10, history);
    REQUIRE(executor.value() == 10);

    history.move_backward(executor);
    REQUIRE(executor.value() == 0);

    history.move_backward(executor);
    REQUIRE(executor.value() == 0);

    history.move_forward(executor);
    REQUIRE(executor.value() == 10);

    history.move_forward(executor);
    REQUIRE(executor.value() == 10);

    history.move_backward(executor);
    REQUIRE(executor.value() == 0);

    executor.set_value(25, history);
    REQUIRE(executor.value() == 25);

    history.move_backward(executor);
    REQUIRE(executor.value() == 0);

    history.move_forward(executor);
    REQUIRE(executor.value() == 25);

    history.move_forward(executor);
    REQUIRE(executor.value() == 25);

    history.move_backward(executor);
    REQUIRE(executor.value() == 0);

    history.move_backward(executor);
    REQUIRE(executor.value() == 0);

    history.move_backward(executor);
    REQUIRE(executor.value() == 0);
}