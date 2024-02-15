#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <cmd/cmd.hpp>

struct Command_SayHello {};
struct Command_SayWorld {};

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
    requires cmd::ExecutorC<Executor, Command>
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
        const auto command = Command_SetInt{.new_value = n, .previous_value = _value};
        _value             = n;
        history.push(command, _merger);
        history.start_new_commands_group(); // We don't want commands to be put in a single group and be done / undone all at once.
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
    struct Merger {
        auto merge(Command_SetInt, Command_SetInt) const -> std::optional<Command_SetInt>
        {
            return std::nullopt;
        }
    } _merger;
};

TEST_CASE("History")
{
    Executor_SetInt              executor{};
    cmd::History<Command_SetInt> history{};

    SUBCASE("Moving backward and forward")
    {
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

    SUBCASE("set_max_size() while the current commit is in the middle of the history")
    {
        history.set_max_size(3);
        REQUIRE(history.max_size() == 3);
        REQUIRE(executor.value() == 0);

        executor.set_value(1, history);
        executor.set_value(2, history);
        executor.set_value(3, history);
        // Now history is full
        history.move_backward(executor);
        history.move_backward(executor);
        REQUIRE(executor.value() == 1);
        // Now we are one after the first commit in the history
        history.set_max_size(1);
        // Now there is only one commit left in the history, and this is the one we can execute next
        history.move_backward(executor); // no-op, the first commit is gone
        REQUIRE(executor.value() == 1);
        history.move_forward(executor); // move forward
        REQUIRE(executor.value() == 2);
        history.move_forward(executor); // no-op, the last commit is gone
        REQUIRE(executor.value() == 2);
    }

    SUBCASE("set_max_size() when we are at the end of the history")
    {
        history.set_max_size(3);
        REQUIRE(history.max_size() == 3);
        REQUIRE(executor.value() == 0);
        executor.set_value(1, history);
        executor.set_value(2, history);
        executor.set_value(3, history);

        history.set_max_size(1);

        history.move_backward(executor);
        REQUIRE(executor.value() == 2);
        history.move_forward(executor);
        REQUIRE(executor.value() == 3);
    }

    SUBCASE("set_max_size(0) when we are in the middle of the history")
    {
        history.set_max_size(2);
        REQUIRE(history.max_size() == 2);
        REQUIRE(executor.value() == 0);
        executor.set_value(1, history);
        executor.set_value(2, history);
        history.move_backward(executor);
        REQUIRE(executor.value() == 1);

        history.set_max_size(0);

        history.move_backward(executor); // no-op
        REQUIRE(executor.value() == 1);
        history.move_forward(executor); // no-op
        REQUIRE(executor.value() == 1);
    }
}

struct Command_AlwaysMerge {
};

struct Executor_AlwaysMerge {
    void execute(Command_AlwaysMerge)
    {
    }

    void revert(Command_AlwaysMerge)
    {
    }
};

struct Merger_AlwaysMerge {
    static auto merge(Command_AlwaysMerge, Command_AlwaysMerge) -> std::optional<Command_AlwaysMerge>
    {
        return Command_AlwaysMerge{};
    }
};

TEST_CASE("Merging commands in an History")
{
    auto       history  = cmd::History<Command_AlwaysMerge>{};
    auto       executor = Executor_AlwaysMerge{};
    const auto merger   = Merger_AlwaysMerge{};

    SUBCASE("Commands that are not the one at the end of the history are never merged")
    {
        auto const push = [&]() {
            history.push({}, merger);
            history.start_new_commands_group();
        };
        push();
        history.dont_merge_next_command();
        push();
        history.dont_merge_next_command();
        push();
        REQUIRE(history.size() == 3);
        history.move_backward(executor); // We will discard two commits when pushing
        history.move_backward(executor); // and add one (unless it gets merged which souldn't happen)
        push();
        REQUIRE(history.size() == 2);
    }
}