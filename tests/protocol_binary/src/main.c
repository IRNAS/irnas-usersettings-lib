#include <user_settings_list.h>
#include <user_settings_protocol_binary.h>

#include <zephyr/ztest.h>
#include <zephyr/ztest_error_hook.h>

struct helper_id_only {
	uint8_t type;
	uint16_t id;
} __attribute__((packed));

struct helper_id_value {
	uint8_t type;
	uint16_t id;
	uint8_t data_len;
	uint8_t data[100];
} __attribute__((packed));

ZTEST_SUITE(protocol_binary_suite, NULL, NULL, NULL, NULL, NULL);

ZTEST(protocol_binary_suite, test_bad_command_type)
{
	int err;
	uint8_t buf[] = {0x0};

	struct user_settings_protocol_command cmd;
	err = user_settings_protocol_binary_decode_command(buf, sizeof(buf), &cmd);
	zassert_not_equal(err, 0, "Decoding should fail on wrong command");

	buf[0] = USPC_NUM_COMMANDS;
	err = user_settings_protocol_binary_decode_command(buf, sizeof(buf), &cmd);
	zassert_not_equal(err, 0, "Decoding should fail on wrong command");
}

ZTEST(protocol_binary_suite, test_commands_without_id)
{
	int err;
	struct user_settings_protocol_command cmd;

	uint8_t cmds_without_id[] = {
		USPC_LIST,
		USPC_LIST_FULL,
		USPC_RESTORE,
	};

	for (int i = 0; i < ARRAY_SIZE(cmds_without_id); i++) {

		err = user_settings_protocol_binary_decode_command(&cmds_without_id[i], 1, &cmd);
		zassert_true(err > 0, "Decoding should succeed when just decoding a id");
		zassert_equal(cmd.type, cmds_without_id[i], "type should be parsed correctly");
		zassert_equal(cmd.id, 0, "Id should be set to 0 when no id is provided");
	}
}

ZTEST(protocol_binary_suite, test_commands_with_id)
{
	int err;
	struct user_settings_protocol_command cmd;

	struct helper_id_only cmds_with_id[] = {
		{USPC_GET, 1},
		{USPC_GET_FULL, 2},
	};

	for (int i = 0; i < ARRAY_SIZE(cmds_with_id); i++) {
		err = user_settings_protocol_binary_decode_command(
			(uint8_t *)&cmds_with_id[i], sizeof(struct helper_id_only), &cmd);
		zassert_equal(cmd.type, cmds_with_id[i].type, "type should be parsed correctly");
		zassert_equal(cmd.id, cmds_with_id[i].id, "Id should be parsed correctly");
	}
}

ZTEST(protocol_binary_suite, test_commands_with_id_when_no_id_is_passed)
{
	int err;
	struct user_settings_protocol_command cmd;

	uint8_t cmds[] = {
		USPC_GET,
		USPC_GET_FULL,
		USPC_SET,
		USPC_SET_DEFAULT,
	};

	for (int i = 0; i < ARRAY_SIZE(cmds); i++) {
		err = user_settings_protocol_binary_decode_command(&cmds[i], 1, &cmd);
		zassert_not_equal(
			err, 0, "Parsing should fail on a command with id when no id is provided");
	}
}

ZTEST(protocol_binary_suite, test_commands_with_id_and_value)
{
	int err;
	struct user_settings_protocol_command cmd;

	struct helper_id_value cmds[] = {
		{USPC_SET, 1, 3, {0x01, 0x02, 0x03}},
		{USPC_SET_DEFAULT, 2, 2, {0x04, 0x05}},
	};

	for (int i = 0; i < ARRAY_SIZE(cmds); i++) {
		err = user_settings_protocol_binary_decode_command((uint8_t *)&cmds[i],
								   4 + cmds[i].data_len, &cmd);
		zassert_true(err > 0, "Parsing should not fail");
		zassert_equal(cmd.type, cmds[i].type, "Type should be parsed correctly");
		zassert_equal(cmd.id, cmds[i].id, "Id should be parsed correctly");
		zassert_equal(cmd.value_len, cmds[i].data_len,
			      "data length should be parsed correctly");
		zassert_mem_equal(cmd.value, cmds[i].data, cmds[i].data_len,
				  "data should be a copy of the the original buffer");
	}
}

ZTEST(protocol_binary_suite, test_user_setting_encode_buffer_to_small)
{
	int err;
	uint8_t buffer[3];

	/* create valid user setting */
	bool value = true;
	bool default_value = false;

	struct user_setting us = {
		.id = 1,
		.key = "1",
		.type = USER_SETTINGS_TYPE_BOOL,
		.max_size = 1,
		.data = &value,
		.data_len = 1,
		.is_set = true,
		.default_data = &default_value,
		.default_data_len = 1,
		.default_is_set = true,
	};

	err = user_settings_protocol_binary_encode(&us, buffer, sizeof(buffer));
	zassert_equal(err, -ENOMEM, "encoding should fail when buffer is to small");
}

ZTEST(protocol_binary_suite, test_user_setting_encode_no_value)
{
	int err;
	uint8_t buffer[255];

	/* create valid user setting */

	struct user_setting us = {
		.id = 1,
		.key = "1",
		.type = USER_SETTINGS_TYPE_BOOL,
		.max_size = 1,
		.is_set = false,
		.default_is_set = false,
	};

	err = user_settings_protocol_binary_encode(&us, buffer, sizeof(buffer));
	zassert_equal(err, 6, "encoding should take exactly 7 bytes (got: %d)", err);
	zassert_equal(*(uint16_t *)&buffer[0], 1, "Id should be encoded here");
	zassert_equal(strcmp(&buffer[2], us.key), 0, "ID should be encoded here");
	zassert_equal(buffer[3], '\0', "ID should be followed by NULL terminator");
	zassert_equal(buffer[4], us.type, "Type should be here");
	zassert_equal(buffer[5], 0, "length should be 0 since no value is set");
}

ZTEST(protocol_binary_suite, test_user_setting_encode_correct_bool)
{
	int err;
	uint8_t buffer[255];

	/* create valid user setting */
	bool value = true;

	struct user_setting us = {
		.id = 1,
		.key = "1",
		.type = USER_SETTINGS_TYPE_BOOL,
		.max_size = 1,
		.data = &value,
		.data_len = 1,
		.is_set = true,
		.default_is_set = false,
	};

	err = user_settings_protocol_binary_encode(&us, buffer, sizeof(buffer));
	zassert_equal(err, 7, "encoding should take exactly 7 bytes (got: %d)", err);
	zassert_equal(*(uint16_t *)&buffer[0], 1, "Id should be encoded here");
	zassert_equal(strcmp(&buffer[2], us.key), 0, "ID should be encoded here");
	zassert_equal(buffer[3], '\0', "ID should be followed by NULL terminator");
	zassert_equal(buffer[4], us.type, "Type should be here");
	zassert_equal(buffer[5], us.data_len, "length should be here");
	zassert_equal(buffer[6], value, "Value should be here");
}

ZTEST(protocol_binary_suite, test_user_setting_encode_correct_bytes)
{
	int err;
	uint8_t buffer[255];

	/* create valid user setting */
	uint8_t value[] = {1, 2, 3, 4};

	struct user_setting us = {
		.id = 1,
		.key = "1",
		.type = USER_SETTINGS_TYPE_BYTES,
		.max_size = 1,
		.data = value,
		.data_len = sizeof(value),
		.is_set = true,
		.default_is_set = false,
	};

	err = user_settings_protocol_binary_encode(&us, buffer, sizeof(buffer));
	zassert_equal(err, 10, "encoding should take exactly 10 bytes (got: %d)", err);
	zassert_equal(*(uint16_t *)&buffer[0], 1, "Id should be encoded here");
	zassert_equal(strcmp(&buffer[2], us.key), 0, "ID should be encoded here");
	zassert_equal(buffer[3], '\0', "ID should be followed by NULL terminator");
	zassert_equal(buffer[4], us.type, "Type should be here");
	zassert_equal(buffer[5], us.data_len, "length should be here");
	zassert_mem_equal(&buffer[6], value, sizeof(value), "Value should be here");
}

ZTEST(protocol_binary_suite, test_user_setting_encode_full_buffer_to_small)
{
	int err;
	uint8_t buffer[7];

	/* create valid user setting */
	bool value = true;
	bool default_value = false;

	struct user_setting us = {
		.id = 1,
		.key = "1",
		.type = USER_SETTINGS_TYPE_BOOL,
		.max_size = 1,
		.data = &value,
		.data_len = 1,
		.is_set = true,
		.default_data = &default_value,
		.default_data_len = 1,
		.default_is_set = true,
	};

	err = user_settings_protocol_binary_encode_full(&us, buffer, sizeof(buffer));
	zassert_equal(err, -ENOMEM, "encoding should fail when buffer is to small");
}

ZTEST(protocol_binary_suite, test_user_setting_encode_correct_bool_full)
{
	int err;
	uint8_t buffer[255];

	/* create valid user setting */
	bool value = true;
	bool default_value = false;

	struct user_setting us = {
		.id = 1,
		.key = "1",
		.type = USER_SETTINGS_TYPE_BOOL,
		.max_size = 1,
		.data = &value,
		.data_len = 1,
		.is_set = true,
		.default_data = &default_value,
		.default_data_len = 1,
		.default_is_set = true,
	};

	err = user_settings_protocol_binary_encode_full(&us, buffer, sizeof(buffer));
	zassert_equal(err, 10, "encoding should take exactly 10 bytes (got: %d)", err);
	zassert_equal(*(uint16_t *)&buffer[0], 1, "Id should be encoded here");
	zassert_equal(strcmp(&buffer[2], us.key), 0, "ID should be encoded here");
	zassert_equal(buffer[3], '\0', "ID should be followed by NULL terminator");
	zassert_equal(buffer[4], us.type, "Type should be here");
	zassert_equal(buffer[5], us.data_len, "length should be here");
	zassert_equal(buffer[6], value, "Value should be here");
	zassert_equal(buffer[7], us.default_data_len, "default length should be here");
	zassert_equal(buffer[8], default_value, "default value should be here");
	zassert_equal(buffer[9], us.max_size, "max size should be here");
}

ZTEST(protocol_binary_suite, test_user_setting_encode_correct_bool_no_default)
{
	int err;
	uint8_t buffer[255];

	/* create valid user setting */
	bool value = true;

	struct user_setting us = {
		.id = 1,
		.key = "1",
		.type = USER_SETTINGS_TYPE_BOOL,
		.max_size = 1,
		.data = &value,
		.data_len = 1,
		.is_set = true,
		.default_is_set = false,
	};

	err = user_settings_protocol_binary_encode_full(&us, buffer, sizeof(buffer));
	zassert_equal(err, 9, "encoding should take exactly 9 bytes (got: %d)", err);
	zassert_equal(*(uint16_t *)&buffer[0], 1, "Id should be encoded here");
	zassert_equal(strcmp(&buffer[2], us.key), 0, "ID should be encoded here");
	zassert_equal(buffer[3], '\0', "ID should be followed by NULL terminator");
	zassert_equal(buffer[4], us.type, "Type should be here");
	zassert_equal(buffer[5], us.data_len, "length should be here");
	zassert_equal(buffer[6], value, "Value should be here");
	zassert_equal(buffer[7], 0, "default length should be 0");
	zassert_equal(buffer[8], us.max_size, "max size should be here");
}

ZTEST(protocol_binary_suite, test_user_setting_encode_correct_bytes_full)
{
	int err;
	uint8_t buffer[255];

	/* create valid user setting */
	uint8_t value[] = {1, 2, 3, 4};
	uint8_t default_value[] = {5, 6};

	struct user_setting us = {
		.id = 1,
		.key = "1",
		.type = USER_SETTINGS_TYPE_BYTES,
		.max_size = 1,
		.data = value,
		.data_len = sizeof(value),
		.is_set = true,
		.default_data = default_value,
		.default_data_len = sizeof(default_value),
		.default_is_set = true,
	};

	err = user_settings_protocol_binary_encode_full(&us, buffer, sizeof(buffer));
	zassert_equal(err, 14, "encoding should take exactly 14 bytes (got: %d)", err);
	zassert_equal(*(uint16_t *)&buffer[0], 1, "Id should be encoded here");
	zassert_equal(strcmp(&buffer[2], us.key), 0, "ID should be encoded here");
	zassert_equal(buffer[3], '\0', "ID should be followed by NULL terminator");
	zassert_equal(buffer[4], us.type, "Type should be here");
	zassert_equal(buffer[5], us.data_len, "length should be here");
	zassert_mem_equal(&buffer[6], value, sizeof(value), "Value should be here");
	zassert_equal(buffer[10], us.default_data_len, "default length should be here");
	zassert_mem_equal(&buffer[11], default_value, sizeof(default_value),
			  "default value should be here");
	zassert_equal(buffer[13], us.max_size, "max size should be here");
}

/* TODO: test list_some commands */
/* TODO: test that the number of bytes decoded is correct for each command */

/* TODO: add multi-command support to executor. Since decode returns number of bytes decoded, we
 * could have a loop and move that number of bytes in the buffer and decode+execute a new command */
