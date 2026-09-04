#include "InputService.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>

namespace {

using EventType = obsidia::InputService::EventType;

void expectNoEvent(obsidia::InputService &input) {
    obsidia::InputService::Event event;
    assert(!input.nextEvent(event));
}

void expectEvent(obsidia::InputService &input, std::uint8_t key, EventType type,
                 std::uint32_t timestampMs) {
    obsidia::InputService::Event event;
    assert(input.nextEvent(event));
    assert(event.key == key);
    assert(event.type == type);
    assert(event.timestampMs == timestampMs);
}

void testBounceAndRelease() {
    obsidia::InputService input({25, 600});
    input.begin(0, 0);
    input.poll(0x01, 10);
    input.poll(0x00, 20);
    input.poll(0x01, 25);
    input.poll(0x01, 49);
    expectNoEvent(input);
    input.poll(0x01, 50);
    expectEvent(input, 0, EventType::Press, 50);
    assert(input.stablePressedMask() == 0x01);

    input.poll(0x00, 60);
    input.poll(0x00, 85);
    expectEvent(input, 0, EventType::Release, 85);
    assert(input.stablePressedMask() == 0x00);
}

void testLongPressIsEmittedOnce() {
    obsidia::InputService input({10, 100});
    input.begin(0, 1000);
    input.poll(0x04, 1010);
    input.poll(0x04, 1020);
    expectEvent(input, 2, EventType::Press, 1020);
    input.poll(0x04, 1119);
    expectNoEvent(input);
    input.poll(0x04, 1120);
    expectEvent(input, 2, EventType::LongPress, 1120);
    input.poll(0x04, 2000);
    expectNoEvent(input);
}

void testSimultaneousKeysAndQueueOrder() {
    obsidia::InputService input({5, 500});
    input.begin(0, 0);
    input.poll(0x82, 1);
    input.poll(0x82, 6);
    expectEvent(input, 1, EventType::Press, 6);
    expectEvent(input, 7, EventType::Press, 6);
    expectNoEvent(input);
    assert(input.stablePressedMask() == 0x82);
}

void testMillisWraparound() {
    obsidia::InputService input({10, 100});
    input.begin(0, 0xFFFFFFF0U);
    input.poll(0x01, 0xFFFFFFFAU);
    input.poll(0x01, 4U);
    expectEvent(input, 0, EventType::Press, 4U);
}

void testQueueIsBounded() {
    obsidia::InputService input({1, 60000});
    input.begin(0, 0);
    std::uint8_t mask = 0;
    std::uint32_t now = 1;
    for (std::size_t cycle = 0; cycle < 3; ++cycle) {
        mask = 0xFF;
        input.poll(mask, now++);
        input.poll(mask, now++);
        mask = 0;
        input.poll(mask, now++);
        input.poll(mask, now++);
    }
    assert(input.droppedEventCount() == 32);
    obsidia::InputService::Event event;
    std::size_t count = 0;
    while (input.nextEvent(event)) ++count;
    assert(count == obsidia::InputService::kQueueCapacity);
}

} // namespace

int main() {
    testBounceAndRelease();
    testLongPressIsEmittedOnce();
    testSimultaneousKeysAndQueueOrder();
    testMillisWraparound();
    testQueueIsBounded();
    std::cout << "InputService tests passed\n";
    return 0;
}
