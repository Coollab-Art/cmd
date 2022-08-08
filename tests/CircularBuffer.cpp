#include "../src/internal/CircularBuffer.hpp"
#include <doctest/doctest.h>

TEST_CASE("CircularBuffer::push_back() adds an element to the buffer, and removes the oldest one if max_size is reached")
{
    auto buffer = cmd::internal::CircularBuffer<int>(3);

    const auto REQUIRE_BUFFER_TO_BE = [&](std::list<int> list) {
        REQUIRE(buffer.underlying_container() == list);
    };

    buffer.push_back(0);
    REQUIRE_BUFFER_TO_BE({0});
    buffer.push_back(1);
    REQUIRE_BUFFER_TO_BE({0, 1});
    buffer.push_back(2);
    REQUIRE_BUFFER_TO_BE({0, 1, 2});
    buffer.push_back(3);
    REQUIRE_BUFFER_TO_BE({1, 2, 3});
    buffer.push_back(4);
    REQUIRE_BUFFER_TO_BE({2, 3, 4});
    buffer.set_max_size(2);
    REQUIRE_BUFFER_TO_BE({3, 4});
}