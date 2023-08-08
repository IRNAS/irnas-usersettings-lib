#include <user_settings.h>
#include <user_settings_list.h>

#include <zephyr/ztest.h>
#include <zephyr/ztest_error_hook.h>

#include "user_settings_json.h"
/* JSON parser */
#include <cJSON.h>
#include <cJSON_os.h>

static void *user_settings_json_suite_setup(void)
{
	user_settings_init();

	/* add 4 common items */
	user_settings_add(1, "t1", USER_SETTINGS_TYPE_BOOL);
	user_settings_add(2, "t2", USER_SETTINGS_TYPE_U32);
	user_settings_add_sized(3, "t3", USER_SETTINGS_TYPE_BYTES, 4);
	user_settings_add_sized(4, "t4", USER_SETTINGS_TYPE_STR, 10);

	user_settings_load();

	return NULL;
}

void user_settings_json_suite_before_each(void *fixture)
{
	/* create default state for each setting */
	user_settings_set_global_on_change_cb(NULL);
	user_settings_set_on_change_cb_with_id(1, NULL);
	user_settings_set_on_change_cb_with_id(2, NULL);
	user_settings_set_on_change_cb_with_id(3, NULL);
	user_settings_set_on_change_cb_with_id(4, NULL);

	bool value1 = 0;
	user_settings_set_with_id(1, &value1, 1);
	uint32_t value2 = 0;
	user_settings_set_with_id(2, &value2, 4);
	uint8_t value3[] = {0, 0, 0, 0};
	user_settings_set_with_id(3, &value3, 4);
	char value4[] = "";
	user_settings_set_with_id(4, &value4, strlen(value4) + 1);
}

ZTEST_SUITE(user_settings_json_suite, NULL, user_settings_json_suite_setup,
	    user_settings_json_suite_before_each, NULL, NULL);

ZTEST(user_settings_json_suite, test_settings_parse_invalid_json_structure)
{
	/* This is a valid JSON, but not in a structure the settings lib can parse */
	char invalid[] = "{ settings: {\"t1\": false}}";

	cJSON *settings = cJSON_Parse(invalid);

	int err = user_settings_set_from_json(settings, false);
	zassert_not_equal(err, 0, "Parsing json should have failed.");

	/* make sure setting was not modified */
	bool t1 = *(bool *)user_settings_get_with_id(1, NULL);
	zassert_equal(t1, false, "Setting should be unmodified");
}

ZTEST(user_settings_json_suite, test_settings_parse_json)
{
	int err;

	char new_str[] = "banana";
	uint32_t new_val = 1000;

	cJSON *settings = cJSON_CreateObject();
	cJSON *bool_value = cJSON_CreateTrue();
	cJSON *number_value = cJSON_CreateNumber((int)new_val);
	cJSON *bytes_value = cJSON_CreateString("FFFFFFFF");
	cJSON *string_value = cJSON_CreateString(new_str);

	zassert_not_null(settings, "cJSON object was NULL");
	zassert_not_null(number_value, "cJSON object was NULL");
	zassert_not_null(string_value, "cJSON object was NULL");
	zassert_not_null(bool_value, "cJSON object was NULL");

	cJSON_AddItemToObject(settings, "t1", bool_value);
	cJSON_AddItemToObject(settings, "t2", number_value);
	cJSON_AddItemToObject(settings, "t3", bytes_value);
	cJSON_AddItemToObject(settings, "t4", string_value);

	err = user_settings_set_from_json(settings, false);
	zassert_equal(err, 0, "Parsing json failed.");

	/* Check set values */
	size_t size;
	bool *out_bool = user_settings_get_with_id(1, &size);
	zassert_equal(*out_bool, true, "What was set should be what was gotten");

	uint32_t *out_number = user_settings_get_with_id(2, &size);
	zassert_equal(*out_number, new_val, "What was set should be what was gotten");

	uint8_t *bytes_out = user_settings_get_with_id(3, &size);
	for (size_t i = 0; i < size; i++) {
		zassert_equal(bytes_out[i], 0xFF, "What was set should be what was gotten");
	}

	char *out_str = user_settings_get_with_id(4, &size);
	zassert_ok(strcmp(new_str, out_str), 0, "What was set should be what was gotten");

	cJSON_Delete(settings);
}

ZTEST(user_settings_json_suite, test_settings_get_all_json)
{
	/* Set stuff to specific values */
	int err;

	bool value = true;
	char new_str[] = "banana";
	uint32_t new_val = 1000;

	err = user_settings_set_with_id(1, &value, sizeof(value));
	zassert_ok(err, "set should not error here");

	err = user_settings_set_with_id(4, &new_str, strlen(new_str) + 1);
	zassert_ok(err, "set should not error here");

	err = user_settings_set_with_id(2, &new_val, sizeof(new_val));
	zassert_ok(err, "set should not error here");

	/*Get JSON object */
	cJSON *settings = NULL;
	user_settings_get_all_json(&settings);

	zassert_not_null(settings, "cJSON object was NULL");

	cJSON *setting;
	setting = cJSON_GetObjectItem(settings, "t1");
	zassert_not_null(setting, "cJSON object 0 was NULL");
	zassert_true(cJSON_IsBool(setting), "Should be bool type");
	zassert_true(cJSON_IsTrue(setting), "Should be true value");

	setting = cJSON_GetObjectItem(settings, "t2");
	zassert_not_null(setting, "cJSON object 1 was NULL");
	zassert_true(cJSON_IsNumber(setting), "Should be number");
	zassert_equal(setting->valueint, new_val, "What was set should be what was gotten");

	setting = cJSON_GetObjectItem(settings, "t3");
	zassert_not_null(setting, "cJSON object was NULL");
	zassert_true(cJSON_IsString(setting), "Should be string");
	zassert_ok(strcmp("00000000", setting->valuestring), 0,
		   "What was set should be what was gotten");

	setting = cJSON_GetObjectItem(settings, "t4");
	zassert_not_null(setting, "cJSON object was NULL");
	zassert_true(cJSON_IsString(setting), "Should be string");
	zassert_ok(strcmp(new_str, setting->valuestring), 0,
		   "What was set should be what was gotten");

	cJSON_Delete(settings);
}

ZTEST(user_settings_json_suite, test_settings_get_changed_json)
{
	/* Clear all changed settings */
	user_settings_clear_changed();

	/* Set stuff to specific values */
	int err;

	bool value = true;
	char new_str[] = "banana";
	uint32_t new_val = 1000;

	err = user_settings_set_with_id(1, &value, sizeof(value));
	zassert_ok(err, "set should not error here");

	err = user_settings_set_with_id(4, &new_str, strlen(new_str) + 1);
	zassert_ok(err, "set should not error here");

	err = user_settings_set_with_id(2, &new_val, sizeof(new_val));
	zassert_ok(err, "set should not error here");

	/*Get JSON object */
	cJSON *settings = NULL;
	user_settings_get_changed_json(&settings);

	zassert_not_null(settings, "cJSON object was NULL");

	cJSON *setting;
	setting = cJSON_GetObjectItem(settings, "t1");
	zassert_not_null(setting, "cJSON object 0 was NULL");
	zassert_ok(strcmp(setting->string, "t1"), "We should get t1");
	zassert_true(cJSON_IsBool(setting), "Should be bool type");
	zassert_true(cJSON_IsTrue(setting), "Should be true value");

	setting = cJSON_GetObjectItem(settings, "t2");
	zassert_not_null(setting, "cJSON object 1 was NULL");
	zassert_ok(strcmp(setting->string, "t2"), "We should get t2");
	zassert_true(cJSON_IsNumber(setting), "Should be number");
	zassert_equal(setting->valueint, new_val, "What was set should be what was gotten");

	setting = cJSON_GetObjectItem(settings, "t4");
	zassert_not_null(setting, "cJSON object was NULL");
	zassert_ok(strcmp(setting->string, "t4"), "We should get t4");
	zassert_true(cJSON_IsString(setting), "Should be string");
	zassert_ok(strcmp(new_str, setting->valuestring), 0,
		   "What was set should be what was gotten");

	/* t3 should not be in the JSON */
	setting = cJSON_GetObjectItem(settings, "t3");
	zassert_is_null(setting, "t3 should be missing");

	cJSON_Delete(settings);
}
