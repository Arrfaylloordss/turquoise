#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "turquoise-details.hpp"


TEST_CASE("Queue")
{
    using namespace turquoise::details;

    Queue<int> queue;

    constexpr int n = 1000;

    for (int i = 0; i < n; ++i)
    {
        queue.push(i);
    }

    for (int i = 0; i < n; ++i)
    {
        REQUIRE(i == queue.pop());
    }
}
