#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "turquoise.hpp"


TEST_CASE("Ping-Pong")
{
    turquoise::ExecutorPool executorPool;

    executorPool.postTask();

    executorPool.run();
}
