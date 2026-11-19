#include "unity.h"

#include "servo.h"
#include "timer_fake.h"

static Timer_Registers timer;
static Servo servo;

void setUp(void)
{
    servo = (Servo){
        .timer = &timer,
        .channel = 2U,
        .counterHz = 2000000U,
        .minPulseUs = 1000U,
        .maxPulseUs = 2000U,
    };
    timerFakeReset(true);
}

void tearDown(void)
{
}

static void assertCompareWasRequested(uint32_t expectedTicks)
{
    TimerFakeState state = timerFakeGetState();

    TEST_ASSERT_EQUAL_UINT32(1U, state.callCount);
    TEST_ASSERT_EQUAL_PTR(&timer, state.timer);
    TEST_ASSERT_EQUAL_UINT8(servo.channel, state.channel);
    TEST_ASSERT_EQUAL_UINT32(expectedTicks, state.compareTicks);
}

void test_converts_valid_pulse_width_to_timer_ticks(void)
{
    TEST_ASSERT_TRUE(servoSetPulseUs(&servo, 1250U));
    assertCompareWasRequested(2500U);
}

void test_accepts_minimum_and_maximum_pulse_widths(void)
{
    TEST_ASSERT_TRUE(servoSetPulseUs(&servo, 1000U));
    assertCompareWasRequested(2000U);

    timerFakeReset(true);
    TEST_ASSERT_TRUE(servoSetPulseUs(&servo, 2000U));
    assertCompareWasRequested(4000U);
}

void test_rejects_out_of_range_pulses_without_calling_timer(void)
{
    TEST_ASSERT_FALSE(servoSetPulseUs(&servo, 999U));
    TEST_ASSERT_FALSE(servoSetPulseUs(&servo, 2001U));
    TEST_ASSERT_FALSE(servoSetPulseUs(&servo, 0U));
    TEST_ASSERT_FALSE(servoSetPulseUs(NULL, 1500U));
    TEST_ASSERT_EQUAL_UINT32(0U, timerFakeGetState().callCount);
}

void test_rejects_invalid_configuration_and_overflowing_compare(void)
{
    servo.minPulseUs = 0U;
    TEST_ASSERT_FALSE(servoSetPulseUs(&servo, 1000U));

    servo.minPulseUs = 2001U;
    servo.maxPulseUs = 2000U;
    TEST_ASSERT_FALSE(servoSetPulseUs(&servo, 2000U));

    servo.minPulseUs = 1U;
    servo.maxPulseUs = UINT32_MAX;
    servo.counterHz = UINT32_MAX;
    TEST_ASSERT_FALSE(servoSetPulseUs(&servo, UINT32_MAX));
    TEST_ASSERT_EQUAL_UINT32(0U, timerFakeGetState().callCount);
}

void test_maps_position_endpoints_and_midpoint_to_pulse_widths(void)
{
    servo.counterHz = 1000000U;

    TEST_ASSERT_TRUE(servoSetPosition(&servo, 0U));
    assertCompareWasRequested(1000U);

    timerFakeReset(true);
    TEST_ASSERT_TRUE(servoSetPosition(&servo, 500U));
    assertCompareWasRequested(1500U);

    timerFakeReset(true);
    TEST_ASSERT_TRUE(servoSetPosition(&servo, 1000U));
    assertCompareWasRequested(2000U);
}

void test_rejects_invalid_positions_without_calling_timer(void)
{
    TEST_ASSERT_FALSE(servoSetPosition(&servo, 1001U));
    TEST_ASSERT_FALSE(servoSetPosition(NULL, 500U));
    TEST_ASSERT_EQUAL_UINT32(0U, timerFakeGetState().callCount);
}

void test_propagates_timer_failure_for_pulse_and_position_calls(void)
{
    timerFakeReset(false);
    TEST_ASSERT_FALSE(servoSetPulseUs(&servo, 1500U));
    assertCompareWasRequested(3000U);

    timerFakeReset(false);
    TEST_ASSERT_FALSE(servoSetPosition(&servo, 500U));
    assertCompareWasRequested(3000U);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_converts_valid_pulse_width_to_timer_ticks);
    RUN_TEST(test_accepts_minimum_and_maximum_pulse_widths);
    RUN_TEST(test_rejects_out_of_range_pulses_without_calling_timer);
    RUN_TEST(test_rejects_invalid_configuration_and_overflowing_compare);
    RUN_TEST(test_maps_position_endpoints_and_midpoint_to_pulse_widths);
    RUN_TEST(test_rejects_invalid_positions_without_calling_timer);
    RUN_TEST(test_propagates_timer_failure_for_pulse_and_position_calls);
    return UNITY_END();
}
