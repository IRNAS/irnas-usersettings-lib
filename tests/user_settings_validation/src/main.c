#include <user_settings.h>
#include <user_settings_list.h>

#include <zephyr/ztest.h>
#include <zephyr/ztest_error_hook.h>

#define NUM_SETTINGS 1

static void *user_settings_suite_setup(void)
{
	user_settings_init();

	/* add 4 common items */
	user_settings_add(0, "t0", USER_SETTINGS_TYPE_U32);

	user_settings_load();

	return NULL;
}

void user_settings_suite_before_each(void *fixture)
{
	/* create default state for each setting */
	user_settings_set_global_on_change_cb(NULL);
	user_settings_set_on_change_cb_with_id(0, NULL);
	user_settings_set_validate_cb_with_id(0, NULL);

	uint32_t value = 0;
	user_settings_set_with_id(0, &value, sizeof(value));
	user_settings_set_default_with_id(0, &value, sizeof(value));
}

ZTEST_SUITE(user_settings_suite, NULL, user_settings_suite_setup, user_settings_suite_before_each,
	    NULL, NULL);

static bool validate_u32_range(uint32_t id, const char *key, void *data, size_t len)
{
	uint32_t value = *(uint32_t *)data;
	return value >= 10 && value <= 20;
}

ZTEST(user_settings_suite, test_validation_callback)
{
	int err;
	uint32_t value;
	size_t size;
	uint32_t *out_value;
	uint32_t *default_value;
	const uint16_t ID = 0;

	/* Set validation callback */
	user_settings_set_validate_cb_with_id(ID, validate_u32_range);

	/* Test setting value within range */
	value = 15;
	err = user_settings_set_with_id(ID, &value, sizeof(value));
	zassert_ok(err, "set should not error here");
	out_value = user_settings_get_with_id(ID, &size);
	zassert_equal(*out_value, value, "What was set should be what was gotten");
	zassert_equal(size, sizeof(value), "size of u32 setting should be 4");

	/* Test setting value out of range */
	value = 25;
	err = user_settings_set_with_id(ID, &value, sizeof(value));
	zassert_equal(err, -EINVAL, "set should error here due to validation failure");
	out_value = user_settings_get_with_id(ID, &size);
	zassert_not_equal(*out_value, value, "Value should not be updated");

	/* Test setting default value within range */
	value = 15;
	err = user_settings_set_default_with_id(ID, &value, sizeof(value));
	zassert_ok(err, "set default should not error here, ret: %d", err);
	default_value = user_settings_get_default_with_id(ID, &size);
	zassert_equal(*default_value, value, "What was set should be what was gotten");
	zassert_equal(size, sizeof(value), "size of u32 setting should be 4");

	/* Test setting default value out of range */
	value = 30;
	err = user_settings_set_default_with_id(ID, &value, sizeof(value));
	zassert_equal(err, -EINVAL, "set default should error here due to validation failure");
	default_value = user_settings_get_default_with_id(ID, &size);
	zassert_not_equal(*default_value, value, "Default value should not be updated");

	/* Test setting value without validation callback */
	user_settings_set_validate_cb_with_id(ID, NULL);
	value = 25;
	err = user_settings_set_with_id(ID, &value, sizeof(value));
	zassert_ok(err, "set should not error here without validation");
	out_value = user_settings_get_with_id(ID, &size);
	zassert_equal(*out_value, value, "What was set should be what was gotten");

	/* Remove validation callback and set out of range values, the settings should be
	 * changed */
	user_settings_set_validate_cb_with_id(ID, NULL);

	/* Set out of range value */
	value = 25;
	err = user_settings_set_with_id(ID, &value, sizeof(value));
	zassert_ok(err, "set should not error here without validation");
	out_value = user_settings_get_with_id(ID, &size);
	zassert_equal(*out_value, value, "What was set should be what was gotten");

	/* Set out of range default value */
	value = 30;
	err = user_settings_set_default_with_id(ID, &value, sizeof(value));
	zassert_ok(err, "set default should not error here without validation");
	default_value = user_settings_get_default_with_id(ID, &size);
	zassert_equal(*default_value, value, "What was set should be what was gotten");
}
