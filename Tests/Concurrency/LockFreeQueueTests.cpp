#include <catch2/catch_test_macros.hpp>
#include "Concurrency/LockFreeQueue.h"
#include <thread>
#include <vector>

TEST_CASE("LockFreeQueue basics", "[concurrency]") {
    Nimbus::LockFreeQueue<int> queue(8);

    REQUIRE(queue.isEmpty() == true);

    SECTION("Push and pop single item") {
        REQUIRE(queue.push(42) == true);
        REQUIRE(queue.isEmpty() == false);

        int popped = 0;
        REQUIRE(queue.pop(popped) == true);
        REQUIRE(popped == 42);
        REQUIRE(queue.isEmpty() == true);
    }

    SECTION("Queue full and empty bounds") {
        // Capacity is 8, but effectively we can push up to 8 items since mask is 7 and we allow queueing up to buffer size.
        // Let's push exactly 8 items.
        for (int i = 0; i < 8; ++i) {
            REQUIRE(queue.push(i) == true);
        }

        // 9th should fail
        REQUIRE(queue.push(9) == false);

        for (int i = 0; i < 8; ++i) {
            int popped = -1;
            REQUIRE(queue.pop(popped) == true);
            REQUIRE(popped == i);
        }

        // Empty should fail
        int popped = -1;
        REQUIRE(queue.pop(popped) == false);
    }
}
