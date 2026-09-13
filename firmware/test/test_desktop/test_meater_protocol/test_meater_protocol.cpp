/**
 * test_meater_protocol.cpp
 *
 * Unit tests for Meater BLE temperature decode formulas (classic + Meater2).
 */
#include <unity.h>
#include <math.h>
#include <string.h>
#include "meater_protocol.h"
#include "meater_protocol.cpp"

void setUp(void) {}
void tearDown(void) {}

void test_decode_classic_tip_and_ambient(void) {
    // tipRaw=152 → (152+8)/16 = 10.0°C tip
    // ra=48, oa=48 → ambientAdj=0 → ambient also 10.0°C
    uint8_t data[6] = {152, 0, 48, 0, 48, 0};
    meater_protocol::ClassicReading out;
    TEST_ASSERT_TRUE(meater_protocol::decodeClassic(data, 6, out));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 10.0f, out.tipC);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 10.0f, out.ambientC);
}

void test_decode_classic_rejects_short(void) {
    uint8_t data[4] = {0, 0, 0, 0};
    meater_protocol::ClassicReading out;
    TEST_ASSERT_FALSE(meater_protocol::decodeClassic(data, 4, out));
}

void test_decode_classic_ambient_above_tip(void) {
    // tipRaw=152 (10°C), ra=100, oa=48
    // ambientAdj = ((100-48)*16*589)/1487 = (52*9424)/1487 ≈ 329
    // ambient = (152+329+8)/16 = 30.5625°C
    uint8_t data[6] = {152, 0, 100, 0, 48, 0};
    meater_protocol::ClassicReading out;
    TEST_ASSERT_TRUE(meater_protocol::decodeClassic(data, 6, out));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 10.0f, out.tipC);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 30.5625f, out.ambientC);
}

void test_decode_battery(void) {
    uint8_t data[2] = {9, 0};  // 9 * 10 = 90%
    TEST_ASSERT_EQUAL_INT(90, meater_protocol::decodeBatteryPercent(data, 2));
    TEST_ASSERT_EQUAL_INT(-1, meater_protocol::decodeBatteryPercent(data, 1));
}

void test_decode_meater2(void) {
    // tip0=640 → 20°C, tip1=672 → 21°C, ..., amb bytes at 10/11 = 800 → 25°C
    // ambient with app fix: 25 + (25-tip4)/5
    uint8_t data[12] = {};
    // tip0 = 640 = 0x0280 LE
    data[0] = 0x80; data[1] = 0x02;
    data[2] = 0xA0; data[3] = 0x02; // 672
    data[4] = 0xC0; data[5] = 0x02;
    data[6] = 0xE0; data[7] = 0x02;
    data[8] = 0x00; data[9] = 0x03; // 768 → 24°C tip4
    data[10] = 0x20; data[11] = 0x03; // 800 → 25°C raw amb

    meater_protocol::Meater2Reading out;
    TEST_ASSERT_TRUE(meater_protocol::decodeMeater2(data, 12, out));
    TEST_ASSERT_EQUAL_UINT8(5, out.tipCount);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 20.0f, out.tipC[0]);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 21.0f, out.tipC[1]);
    // amb = 25 + (25-24)/5 = 25.2
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 25.2f, out.ambientC);
}

void test_is_meater_device_name(void) {
    TEST_ASSERT_TRUE(meater_protocol::isMeaterDeviceName("MEATER"));
    TEST_ASSERT_TRUE(meater_protocol::isMeaterDeviceName("MEATER+"));
    TEST_ASSERT_TRUE(meater_protocol::isMeaterDeviceName("meater block"));
    TEST_ASSERT_FALSE(meater_protocol::isMeaterDeviceName("ThermoWorks"));
    TEST_ASSERT_FALSE(meater_protocol::isMeaterDeviceName(""));
    TEST_ASSERT_FALSE(meater_protocol::isMeaterDeviceName(nullptr));
}

int main(int argc, char** argv) {
    UNITY_BEGIN();
    RUN_TEST(test_decode_classic_tip_and_ambient);
    RUN_TEST(test_decode_classic_rejects_short);
    RUN_TEST(test_decode_classic_ambient_above_tip);
    RUN_TEST(test_decode_battery);
    RUN_TEST(test_decode_meater2);
    RUN_TEST(test_is_meater_device_name);
    return UNITY_END();
}
