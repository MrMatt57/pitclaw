// Exercise the production sample pipeline with independently calculated
// voltage/resistance fixture points. No duplicated conversion implementation.
#include <unity.h>
#include <initializer_list>
#include "temp_manager.h"
#include "temp_manager.cpp"

static TempManager temp;
void setUp() { temp = TempManager(); temp.setEMAAlpha(1.0f); }
void tearDown() {}

void test_equal_resistors_at_1_65V() {
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.65f, TempManager::adcToVoltage(13200));
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 10000.0f, TempManager::adcToResistance(13200));
    temp.processRawReading(PROBE_PIT, 13200);
    TEST_ASSERT_TRUE(temp.isConnected(PROBE_PIT));
    TEST_ASSERT_FLOAT_WITHIN(0.02f, 84.53775f, temp.getTempC(PROBE_PIT));
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 184.16795f, temp.getPitTemp());
}
void test_100k_fixture_at_3V() {
    TEST_ASSERT_FLOAT_WITHIN(2.0f, 100000.0f, TempManager::adcToResistance(24000));
    temp.processRawReading(PROBE_PIT, 24000);
    TEST_ASSERT_FLOAT_WITHIN(0.02f, 24.17125f, temp.getTempC(PROBE_PIT));
}
void test_5k_fixture_at_1_1V() {
    TEST_ASSERT_FLOAT_WITHIN(0.2f, 5000.0f, TempManager::adcToResistance(8800));
    temp.processRawReading(PROBE_PIT, 8800);
    TEST_ASSERT_TRUE(temp.getTempC(PROBE_PIT) > 100.0f);
}
void test_warming_probe_lowers_voltage_and_raises_temperature() {
    temp.processRawReading(PROBE_PIT, 24000);
    float cold = temp.getPitTemp();
    temp.processRawReading(PROBE_PIT, 13200);
    float warm = temp.getPitTemp();
    temp.processRawReading(PROBE_PIT, 8800);
    TEST_ASSERT_TRUE(cold < warm && warm < temp.getPitTemp());
}
void test_open_at_supply_rail_is_disconnected() {
    temp.processRawReading(PROBE_PIT, 26400);
    TEST_ASSERT_FALSE(temp.isConnected(PROBE_PIT));
    TEST_ASSERT_EQUAL((int)ProbeStatus::OPEN_CIRCUIT, (int)temp.getStatus(PROBE_PIT));
    TEST_ASSERT_EQUAL_FLOAT(0.0f, temp.getPitTemp());
    TEST_ASSERT_TRUE(isinf(TempManager::adcToResistance(26400)));
}
void test_open_threshold_and_overrange() {
    for (int16_t raw : { (int16_t)ERROR_PROBE_OPEN_THRESHOLD, (int16_t)32767 }) {
        temp.processRawReading(PROBE_PIT, raw);
        TEST_ASSERT_EQUAL((int)ProbeStatus::OPEN_CIRCUIT, (int)temp.getStatus(PROBE_PIT));
    }
}
void test_short_threshold_and_negative() {
    for (int16_t raw : { (int16_t)-10, (int16_t)0, (int16_t)ERROR_PROBE_SHORT_THRESHOLD }) {
        temp.processRawReading(PROBE_PIT, raw);
        TEST_ASSERT_FALSE(temp.isConnected(PROBE_PIT));
        TEST_ASSERT_EQUAL((int)ProbeStatus::SHORT_CIRCUIT, (int)temp.getStatus(PROBE_PIT));
    }
}
void test_ice_bath_region_remains_valid() {
    // 300k fixture = 3.193548V, 25548 counts, approximately +1.12C.
    temp.processRawReading(PROBE_PIT, 25548);
    TEST_ASSERT_TRUE(temp.isConnected(PROBE_PIT));
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 1.12f, temp.getTempC(PROBE_PIT));
}
void test_reconnect_discards_old_ema_value() {
    temp.setEMAAlpha(0.2f);
    temp.processRawReading(PROBE_PIT, 24000);
    temp.processRawReading(PROBE_PIT, 26400);
    temp.processRawReading(PROBE_PIT, 13200);
    TEST_ASSERT_FLOAT_WITHIN(0.02f, 84.53775f, temp.getTempC(PROBE_PIT));
}
void test_channels_are_independent() {
    temp.processRawReading(PROBE_PIT, 13200);
    temp.processRawReading(PROBE_MEAT1, 24000);
    temp.processRawReading(PROBE_MEAT2, 0);
    TEST_ASSERT_FLOAT_WITHIN(0.02f, 84.53775f, temp.getTempC(PROBE_PIT));
    TEST_ASSERT_FLOAT_WITHIN(0.02f, 24.17125f, temp.getTempC(PROBE_MEAT1));
    TEST_ASSERT_FALSE(temp.isConnected(PROBE_MEAT2));
}
void test_offset_and_units() {
    temp.setOffset(PROBE_PIT, 1.0f);
    temp.setUseFahrenheit(false);
    temp.processRawReading(PROBE_PIT, 13200);
    TEST_ASSERT_FLOAT_WITHIN(0.02f, 85.53775f, temp.getPitTemp());
    TEST_ASSERT_EQUAL_FLOAT(32.0f, TempManager::cToF(0));
    TEST_ASSERT_EQUAL_FLOAT(212.0f, TempManager::cToF(100));
}
int main() {
    UNITY_BEGIN();
    RUN_TEST(test_equal_resistors_at_1_65V);
    RUN_TEST(test_100k_fixture_at_3V);
    RUN_TEST(test_5k_fixture_at_1_1V);
    RUN_TEST(test_warming_probe_lowers_voltage_and_raises_temperature);
    RUN_TEST(test_open_at_supply_rail_is_disconnected);
    RUN_TEST(test_open_threshold_and_overrange);
    RUN_TEST(test_short_threshold_and_negative);
    RUN_TEST(test_ice_bath_region_remains_valid);
    RUN_TEST(test_reconnect_discards_old_ema_value);
    RUN_TEST(test_channels_are_independent);
    RUN_TEST(test_offset_and_units);
    return UNITY_END();
}
