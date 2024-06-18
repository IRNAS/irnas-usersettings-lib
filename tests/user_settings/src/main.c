#include <user_settings.h>
#include <user_settings_list.h>

#include <zephyr/ztest.h>
#include <zephyr/ztest_error_hook.h>

#define NUM_SETTINGS 5

static void *user_settings_suite_setup(void)
{
	user_settings_init();

	/* add 4 common items */
	user_settings_add(1, "t1", USER_SETTINGS_TYPE_BOOL);
	user_settings_add(2, "t2", USER_SETTINGS_TYPE_U32);
	user_settings_add(3, "t3", USER_SETTINGS_TYPE_I8);
	user_settings_add_sized(4, "t4", USER_SETTINGS_TYPE_STR, 10);
	user_settings_add(5, "t5", USER_SETTINGS_TYPE_U32);

	user_settings_load();

	return NULL;
}

void user_settings_suite_before_each(void *fixture)
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
	int8_t value3 = 0;
	user_settings_set_with_id(3, &value3, 1);
	char value4[] = "";
	user_settings_set_with_id(4, &value4, strlen(value4) + 1);
}

ZTEST_SUITE(user_settings_suite, NULL, user_settings_suite_setup, user_settings_suite_before_each,
	    NULL, NULL);

ZTEST(user_settings_suite, test_settings_exist)
{
	zassert_equal(user_settings_exists_with_id(1), true, "Setting should exist");
	zassert_equal(user_settings_exists_with_id(2), true, "Setting should exist");
	zassert_equal(user_settings_exists_with_id(3), true, "Setting should exist");
	zassert_equal(user_settings_exists_with_id(4), true, "Setting should exist");
}

ZTEST(user_settings_suite, test_nonexistand_settings_dont_exist)
{
	zassert_equal(user_settings_exists_with_id(0), false, "Setting should not exist");
	zassert_equal(user_settings_exists_with_key("t0"), false, "Setting should not exist");
}

ZTEST(user_settings_suite, test_settings_key_id_conversion)
{
	zassert_equal(user_settings_key_to_id("t1"), 1, "key t1 should map to id 1");
	zassert_ok(strcmp(user_settings_id_to_key(2), "t2"), "id 2 should map to key \"t2\"");
}

ZTEST(user_settings_suite, test_settings_set)
{
	int err;

	bool value = true;
	size_t size;
	err = user_settings_set_with_id(1, &value, 1);
	zassert_ok(err, "set should not error here");

	bool *out_value = user_settings_get_with_id(1, &size);
	zassert_equal(*out_value, value, "What was set should be what was gotten");
	zassert_equal(size, 1, "size of bool setting should be 1");

	/* you can also get without size out param */
	bool *out_value2 = user_settings_get_with_id(1, NULL);
	zassert_equal(*out_value2, value, "What was set should be what was gotten");

	/* test set and get of string settings */
	char new_str[] = "banana";
	err = user_settings_set_with_id(4, &new_str, strlen(new_str) + 1);
	zassert_ok(err, "set should not error here");

	char *out_str = user_settings_get_with_id(4, &size);
	zassert_ok(strcmp(new_str, out_str), 0, "What was set should be what was gotten");
	zassert_equal(size, strlen(new_str) + 1, "size of str setting should be %d",
		      strlen(new_str) + 1);
}

ZTEST(user_settings_suite, test_settings_set_value_too_large)
{
	int err;

	/* fist set a good value */
	char new_str[] = "banana";
	err = user_settings_set_with_id(4, &new_str, strlen(new_str) + 1);

	/* try set a value that is to long */
	char new_str2[] = "bananarama";
	err = user_settings_set_with_id(4, &new_str2, strlen(new_str2) + 1);
	zassert_equal(err, -ENOMEM, "set should error here since new string is too long");

	/* setting should be old */
	char *out_str = user_settings_get_with_id(4, NULL);
	zassert_ok(strcmp(new_str, out_str), 0, "Setting not at value that was expected");
}

ZTEST(user_settings_suite, test_settings_default_value)
{
	/* set value */
	uint32_t value = 11;
	user_settings_set_with_id(2, &value, 4);

	/* set some default value */
	uint32_t default_value = 69;
	user_settings_set_default_with_id(2, &default_value, 4);

	/* Get default should return what was set above */
	size_t len_out;
	uint32_t val_get = *(uint32_t *)user_settings_get_default_with_id(2, &len_out);
	zassert_equal(len_out, sizeof(default_value), "default value should be %d",
		      sizeof(default_value));
	zassert_equal(val_get, default_value, "default value should be %d", default_value);

	/* value should be unchanged */
	uint32_t value_out;
	value_out = *(uint32_t *)user_settings_get_with_id(2, NULL);
	zassert_equal(value_out, value, "Value should be unchanged");
}

ZTEST(user_settings_suite, test_settings_default_value_twice)
{
	int err;

	/* Set default value for t5 */
	uint32_t default_value = 69;

	err = user_settings_set_default_with_id(5, &default_value, sizeof(default_value));
	zassert_ok(err, "set default should not error here");

	/* Set default with same value should not error */
	err = user_settings_set_default_with_id(5, &default_value, sizeof(default_value));
	zassert_ok(err, "set default should not error here");

	/* Set default with new value should error if CONFIG_USER_SETTINGS_DEFAULT_OVERWRITE=n.
	 *
	 * NOTE: If running these test with Twister, both cases will be tested.
	 * The command is: east twister -T . -p native_posix
	 */
	uint32_t new_default_value = 70;
	err = user_settings_set_default_with_id(5, &new_default_value, sizeof(new_default_value));

	if (IS_ENABLED(CONFIG_USER_SETTINGS_DEFAULT_OVERWRITE)) {
		zassert_ok(err, "set default should not error here, err: %d", err);
	} else {
		zassert_equal(err, -EALREADY, "set default should error here, err: %d", err);
	}
}

ZTEST(user_settings_suite, test_settings_restore_one)
{
	/* set value */
	int8_t default_value = 0, value = -1;
	user_settings_set_with_id(3, &value, 1);
	user_settings_set_default_with_id(3, &default_value, 1);

	/* value should be what was set */
	int8_t value_out = *(int8_t *)user_settings_get_with_id(3, NULL);
	zassert_equal(value_out, value, "Value should be %d before restore", value);

	/* restore default for this setting */
	user_settings_restore_default_with_id(3);

	/* value should be default */
	int8_t value_out_after_restore = *(int8_t *)user_settings_get_with_id(3, NULL);
	zassert_equal(value_out_after_restore, default_value, "Value should be %d after restore",
		      default_value);
}

static uint32_t on_change_id_store;
static const char *on_change_key_store;
void on_change(uint32_t id, const char *key)
{
	on_change_id_store = id;
	on_change_key_store = key;
}

ZTEST(user_settings_suite, test_settings_on_change)
{
	user_settings_set_on_change_cb_with_id(2, on_change);

	uint32_t value = 1337;
	user_settings_set_with_id(2, &value, 4);

	zassert_equal(on_change_id_store, 2, "On change callback should have been called");
	zassert_ok(strcmp(on_change_key_store, "t2"), "On change callback should have been called");
}

ZTEST(user_settings_suite, test_settings_global_on_change)
{
	user_settings_set_global_on_change_cb(on_change);

	uint32_t value = 1337;
	user_settings_set_with_id(2, &value, 4);

	zassert_equal(on_change_id_store, 2, "global on change callback should have been called");
	zassert_ok(strcmp(on_change_key_store, "t2"),
		   "global on change callback should have been called");

	int8_t value2 = -1;
	user_settings_set_with_id(3, &value2, 1);
	zassert_equal(on_change_id_store, 3, "global on change callback should have been called");
	zassert_ok(strcmp(on_change_key_store, "t3"),
		   "global on change callback should have been called");
}

ZTEST(user_settings_suite, test_settings_callback_is_called_on_restore)
{
	/* Set default value */
	bool default_value = false;
	user_settings_set_default_with_id(1, &default_value, sizeof(default_value));

	/* set value different from default */
	bool value = true;
	user_settings_set_with_id(1, &value, sizeof(value));

	/* Register callback for setting */
	user_settings_set_on_change_cb_with_id(1, on_change);

	/* restore */
	user_settings_restore_default_with_id(1);
	/* callback should be triggered */
	zassert_equal(on_change_id_store, 1, "on change callback should have been called");

	/* restoring again should not trigger callback */
	on_change_id_store = 0;
	user_settings_restore_default_with_id(1);
	zassert_equal(on_change_id_store, 0, "on change callback should not have been called");
}

ZTEST(user_settings_suite, test_settings_get_max_len)
{
	/* Each user setting should return the correct max length */
	size_t len;

	len = user_settings_get_max_len_with_id(1);
	zassert_equal(len, 1, "Max length should be 1 for bool type");
	len = user_settings_get_max_len_with_id(2);
	zassert_equal(len, 4, "Max length should be 4 for u32 type");
	len = user_settings_get_max_len_with_id(3);
	zassert_equal(len, 1, "Max length should be 1 for i8 type");
	len = user_settings_get_max_len_with_id(4);
	zassert_equal(len, 10, "Max length should be 10 for string type");
}

ZTEST(user_settings_suite, test_settings_get_type)
{
	/* Each user setting should return the correct max length */
	enum user_setting_type type;

	type = user_settings_get_type_with_id(1);
	zassert_equal(type, USER_SETTINGS_TYPE_BOOL,
		      "Get type does not match the configured type for t1");
	type = user_settings_get_type_with_id(2);
	zassert_equal(type, USER_SETTINGS_TYPE_U32,
		      "Get type does not match the configured type for t2");
	type = user_settings_get_type_with_id(3);
	zassert_equal(type, USER_SETTINGS_TYPE_I8,
		      "Get type does not match the configured type for t3");
	type = user_settings_get_type_with_id(4);
	zassert_equal(type, USER_SETTINGS_TYPE_STR,
		      "Get type does not match the configured type for t4");
}

ZTEST(user_settings_suite, test_settings_iter_correct_count)
{
	uint16_t n_settings = 0;
	char *key = NULL;
	uint16_t id = 0;

	/* Start iteration */
	user_settings_iter_start();
	while (user_settings_iter_next(&key, &id)) {
		n_settings++;
		zassert_equal(id, n_settings, "wrong id");
	}
	zassert_equal(n_settings, NUM_SETTINGS, "number of settings should be %d", NUM_SETTINGS);
}

ZTEST(user_settings_suite, test_settings_iter_correct_key_and_id)
{
	char *key = "nope";
	uint16_t id = 0;
	bool ret;

	/* Start iteration */
	user_settings_iter_start();

	/* Assert that key and ID are returned as expected */
	ret = user_settings_iter_next(&key, &id);
	zassert_true(ret, "Return value should be true");
	zassert_equal(id, 1, "Id should be 1, was %d", id);
	zassert_ok(strcmp(key, "t1"), "Key should be t1, was: %s", key);

	ret = user_settings_iter_next(&key, &id);
	zassert_true(ret, "Return value should be true");
	zassert_equal(id, 2, "Id should be 2, was %d", id);
	zassert_ok(strcmp(key, "t2"), "Key should be t2, was: %s", key);

	ret = user_settings_iter_next(&key, &id);
	zassert_true(ret, "Return value should be true");
	zassert_equal(id, 3, "Id should be 3, was %d", id);
	zassert_ok(strcmp(key, "t3"), "Key should be t3, was: %s", key);

	ret = user_settings_iter_next(&key, &id);
	zassert_true(ret, "Return value should be true");
	zassert_equal(id, 4, "Id should be 4, was %d", id);
	zassert_ok(strcmp(key, "t4"), "Key should be t4, was: %s", key);

	ret = user_settings_iter_next(&key, &id);
	zassert_true(ret, "Return value should be true");
	zassert_equal(id, 5, "Id should be 5, was %d", id);
	zassert_ok(strcmp(key, "t5"), "Key should be t5, was: %s", key);

	/* Since we have 5 settings, we should get NULL here */
	ret = user_settings_iter_next(&key, &id);
	zassert_false(ret, "Return value should be false");
}

ZTEST(user_settings_suite, test_settings_iter_restart_midway)
{
	char *key = NULL;
	uint16_t id = 0;

	bool ret;

	/* Start iteration */
	user_settings_iter_start();

	/* Assert that key and ID are returned as expected */
	ret = user_settings_iter_next(&key, &id);
	zassert_true(ret, "Return value should be true");
	zassert_equal(id, 1, "Id should be 1, was %d", id);
	zassert_ok(strcmp(key, "t1"), "Key should be t1, was: %s", key);

	ret = user_settings_iter_next(&key, &id);
	zassert_true(ret, "Return value should be true");
	zassert_equal(id, 2, "Id should be 2, was %d", id);
	zassert_ok(strcmp(key, "t2"), "Key should be t2, was: %s", key);

	/* Start again  */
	user_settings_iter_start();

	/* We should be back at first element */
	ret = user_settings_iter_next(&key, &id);
	zassert_true(ret, "Return value should be true");
	zassert_equal(id, 1, "Id should be 1, was %d", id);
	zassert_ok(strcmp(key, "t1"), "Key should be t1, was: %s", key);
}

ZTEST(user_settings_suite, test_settings_changed_recently)
{
	int err;
	char *key = NULL;
	uint16_t id = 0;
	int n_changed = 0;

	/* Clear all changed settings */
	user_settings_clear_changed();

	/* Start iteration */
	user_settings_iter_start();
	while (user_settings_iter_next_changed(&key, &id)) {
		n_changed++;
	}
	zassert_equal(n_changed, 0, "we cleared all, number of changed settings should be 0");

	/* Change single value */
	bool value = true;
	err = user_settings_set_with_id(1, &value, 1);
	zassert_ok(err, "set should not error here");

	/* Start iteration */
	n_changed = 0;
	user_settings_iter_start();
	while (user_settings_iter_next_changed(&key, &id)) {
		zassert_equal(id, 1, "changed id should be 1");
		zassert_ok(strcmp(key, "t1"), "changed key should be t1");
		n_changed++;
	}
	zassert_equal(n_changed, 1, "number of changed settings should be 1");

	/* Change another two values */
	int8_t int_value = -1;
	err = user_settings_set_with_id(3, &int_value, 1);
	zassert_ok(err, "set should not error here");

	char new_str[] = "pineapple";
	err = user_settings_set_with_key("t4", &new_str, strlen(new_str) + 1);
	zassert_ok(err, "set should not error here");

	/* Start iteration */
	n_changed = 0;
	user_settings_iter_start();
	while (user_settings_iter_next_changed(&key, &id)) {
		n_changed++;
	}
	zassert_equal(n_changed, 3, "we set 2 more, number of changed settings should be 3");

	/* Clear changed with id */
	user_settings_clear_changed_with_id(1);

	/* Start iteration */
	n_changed = 0;
	user_settings_iter_start();
	while (user_settings_iter_next_changed(&key, &id)) {
		n_changed++;
	}
	zassert_equal(n_changed, 2, "we cleared 1 with id, number of changed settings should be 2");

	/* Clear changed with key */
	user_settings_clear_changed_with_key("t4");

	/* Start iteration */
	n_changed = 0;
	user_settings_iter_start();
	while (user_settings_iter_next_changed(&key, &id)) {
		n_changed++;
	}
	zassert_equal(n_changed, 1,
		      "we cleared one with key, number of changed settings should be 1");

	/* Clear all */
	user_settings_clear_changed();

	/* Start iteration */
	n_changed = 0;
	user_settings_iter_start();
	while (user_settings_iter_next_changed(&key, &id)) {
		n_changed++;
	}
	zassert_equal(n_changed, 0, "we cleared all, number of changed settings should be 0");
}

ZTEST(user_settings_suite, test_settings_user_settings_any_changed)
{
	bool c;

	/* Clear all changed settings */
	user_settings_clear_changed();

	c = user_settings_any_changed();
	zassert_false(c, "No setting should be marked changed");

	/* change 1 setting */
	int8_t int_value = 11;
	user_settings_set_with_id(3, &int_value, 1);

	c = user_settings_any_changed();
	zassert_true(c, "Since a setting was changed, this should return true");

	/* change another */
	char new_str[] = "orange";
	user_settings_set_with_key("t4", &new_str, strlen(new_str) + 1);

	c = user_settings_any_changed();
	zassert_true(c, "Since a setting was changed, this should return true");

	/* clear changed flag one one setting */
	user_settings_clear_changed_with_id(3);

	c = user_settings_any_changed();
	zassert_true(c, "Since a setting is still changed, this should return true");

	/* clear all */
	user_settings_clear_changed();

	c = user_settings_any_changed();
	zassert_false(c, "No setting should be marked changed");
}
/*
 * NOT TESTED:
 *
 * - assertions when getting/setting nonexistent settings
 */
