#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <cmd/cmd.hpp>
#include <iostream>

struct Command_SayHello {
};

void apply(Command_SayHello)
{
    std::cout << "Hello\n";
}

void do_something(cmd::Command auto command)
{
    apply(command);
}

TEST_CASE("Hello World")
{
    do_something(Command_SayHello{});
}