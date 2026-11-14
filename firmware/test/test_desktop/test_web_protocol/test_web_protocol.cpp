#include <unity.h>
#include <cstring>
#include "web_protocol.cpp"
using namespace bbq_protocol;
void setUp() {}
void tearDown() {}
static ParsedCommand parse(const char* s) { return parseCommand(s, strlen(s)); }
void test_partial_alarm_preserves_other_targets() {
    auto c = parse("{\"type\":\"alarm\",\"pitBand\":15}");
    TEST_ASSERT_TRUE(c.type == CmdType::ALARM);
    TEST_ASSERT_TRUE(c.hasPitBand);
    TEST_ASSERT_FALSE(c.hasMeat1Target);
    TEST_ASSERT_FALSE(c.hasMeat2Target);
}
void test_explicit_null_disables_only_selected_probe() {
    auto c = parse("{\"type\":\"alarm\",\"meat1Target\":null}");
    TEST_ASSERT_TRUE(c.hasMeat1Target);
    TEST_ASSERT_EQUAL_FLOAT(0, c.meat1Target);
    TEST_ASSERT_FALSE(c.hasMeat2Target);
}
void test_invalid_commands_cannot_control_outputs() {
    const char* invalid[] = {
        "{\"type\":\"set\"}", "{\"type\":\"set\",\"sp\":null}",
        "{\"type\":\"set\",\"sp\":\"225\"}", "{\"type\":\"set\",\"sp\":true}",
        "{\"type\":\"set\",\"sp\":99}", "{\"type\":\"set\",\"sp\":501}",
        "{\"type\":\"set\",\"sp\":1e100}", "{\"type\":\"config\",\"fanMode\":\"bad\"}",
        "{\"type\":\"alarm\",\"meat1Target\":999}",
        "{\"type\":\"alarm\",\"pitBand\":0}", "broken"
    };
    for (auto s : invalid) TEST_ASSERT_TRUE_MESSAGE(parse(s).type == CmdType::UNKNOWN, s);
    char large[513]; memset(large, ' ', sizeof(large));
    TEST_ASSERT_TRUE(parseCommand(large, sizeof(large)).type == CmdType::UNKNOWN);
}
void test_valid_boundaries_and_nonterminated_input() {
    TEST_ASSERT_TRUE(parse("{\"type\":\"set\",\"sp\":100}").type == CmdType::SET_SP);
    TEST_ASSERT_TRUE(parse("{\"type\":\"set\",\"sp\":500}").type == CmdType::SET_SP);
    const char input[] = "{\"type\":\"set\",\"sp\":225}TRAILER";
    auto c = parseCommand(input, strlen(input)-7);
    TEST_ASSERT_TRUE(c.type == CmdType::SET_SP);
    TEST_ASSERT_EQUAL_FLOAT(225, c.setpoint);
}
int main() {
    UNITY_BEGIN();
    RUN_TEST(test_partial_alarm_preserves_other_targets);
    RUN_TEST(test_explicit_null_disables_only_selected_probe);
    RUN_TEST(test_invalid_commands_cannot_control_outputs);
    RUN_TEST(test_valid_boundaries_and_nonterminated_input);
    return UNITY_END();
}
