#include <user_settings_list.h>

#include <zephyr/ztest.h>
#include <zephyr/ztest_error_hook.h>

static void *user_settings_list_suite_setup(void)
{
	/* init list */
	user_settings_list_init();

	return NULL;
}

/* this is called before each test */
static void user_settings_list_suite_before_each(void *f)
{
	/* clear/free list */
	user_settings_list_free();
}

ZTEST_SUITE(user_settings_list_suite, NULL, user_settings_list_suite_setup,
	    user_settings_list_suite_before_each, NULL, NULL);

ZTEST(user_settings_list_suite, test_list_add_items)
{
	struct user_setting *us;

	us = user_settings_list_add_fixed_size(1, "t1", USER_SETTINGS_TYPE_BOOL);

	zassert_not_null(us, "User setting should be returned");
	zassert_equal(us->id, 1, "User setting should have ID set correctly");
	zassert_equal(strcmp("t1", us->key), 0, "User setting should have key set correctly");
	zassert_equal(us->type, USER_SETTINGS_TYPE_BOOL,
		      "User setting should have type set correctly");
	zassert_equal(us->max_size, 1, "User setting should have max_size set correctly");
	zassert_not_null(us->data, "User setting should have allocated data");
	zassert_equal(us->data_len, 0, "Data length should be zero since it was not set yet");
	zassert_false(us->is_set, "Data should not be marked set");
	zassert_not_null(us->default_data, "User setting should have allocated default data");
	zassert_equal(us->default_data_len, 0,
		      "default data length should be zero since it was not set yet");
	zassert_false(us->default_is_set, "default data should not be marked set");
	zassert_is_null(us->on_change_cb, "on change callback should not be set");

	us = user_settings_list_add_fixed_size(2, "t2", USER_SETTINGS_TYPE_U16);

	zassert_not_null(us, "User setting should be returned");
	zassert_equal(us->id, 2, "User setting should have ID set correctly");
	zassert_equal(strcmp("t2", us->key), 0, "User setting should have key set correctly");
	zassert_equal(us->type, USER_SETTINGS_TYPE_U16,
		      "User setting should have type set correctly");
	zassert_equal(us->max_size, 2, "User setting should have max_size set correctly");
	zassert_not_null(us->data, "User setting should have allocated data");
	zassert_equal(us->data_len, 0, "Data length should be zero since it was not set yet");
	zassert_false(us->is_set, "Data should not be marked set");
	zassert_not_null(us->default_data, "User setting should have allocated default data");
	zassert_equal(us->default_data_len, 0,
		      "default data length should be zero since it was not set yet");
	zassert_false(us->default_is_set, "default data should not be marked set");
	zassert_is_null(us->on_change_cb, "on change callback should not be set");

	us = user_settings_list_add_fixed_size(3, "t3", USER_SETTINGS_TYPE_U32);

	zassert_not_null(us, "User setting should be returned");
	zassert_equal(us->id, 3, "User setting should have ID set correctly");
	zassert_equal(strcmp("t3", us->key), 0, "User setting should have key set correctly");
	zassert_equal(us->type, USER_SETTINGS_TYPE_U32,
		      "User setting should have type set correctly");
	zassert_equal(us->max_size, 4, "User setting should have max_size set correctly");
	zassert_not_null(us->data, "User setting should have allocated data");
	zassert_equal(us->data_len, 0, "Data length should be zero since it was not set yet");
	zassert_false(us->is_set, "Data should not be marked set");
	zassert_not_null(us->default_data, "User setting should have allocated default data");
	zassert_equal(us->default_data_len, 0,
		      "default data length should be zero since it was not set yet");
	zassert_false(us->default_is_set, "default data should not be marked set");
	zassert_is_null(us->on_change_cb, "on change callback should not be set");

	us = user_settings_list_add_variable_size(4, "t4", USER_SETTINGS_TYPE_STR, 10);

	zassert_not_null(us, "User setting should be returned");
	zassert_equal(us->id, 4, "User setting should have ID set correctly");
	zassert_equal(strcmp("t4", us->key), 0, "User setting should have key set correctly");
	zassert_equal(us->type, USER_SETTINGS_TYPE_STR,
		      "User setting should have type set correctly");
	zassert_equal(us->max_size, 10, "User setting should have max_size set correctly");
	zassert_not_null(us->data, "User setting should have allocated data");
	zassert_equal(us->data_len, 0, "Data length should be zero since it was not set yet");
	zassert_false(us->is_set, "Data should not be marked set");
	zassert_not_null(us->default_data, "User setting should have allocated default data");
	zassert_equal(us->default_data_len, 0,
		      "default data length should be zero since it was not set yet");
	zassert_false(us->default_is_set, "default data should not be marked set");
	zassert_is_null(us->on_change_cb, "on change callback should not be set");
}

ZTEST(user_settings_list_suite, test_list_add_repeated_ids_will_assert)
{
	user_settings_list_add_fixed_size(1, "t1", USER_SETTINGS_TYPE_BOOL);
	ztest_set_assert_valid(true);
	user_settings_list_add_fixed_size(1, "t2", USER_SETTINGS_TYPE_BOOL);

	/* the above assertion will abort this test function. If the assert does not happen,
	 * we have to mark the test as failed */
	ztest_test_fail();
}

ZTEST(user_settings_list_suite, test_list_add_repeated_keys_will_assert)
{
	user_settings_list_add_fixed_size(1, "t1", USER_SETTINGS_TYPE_BOOL);
	ztest_set_assert_valid(true);
	user_settings_list_add_fixed_size(2, "t1", USER_SETTINGS_TYPE_BOOL);

	ztest_test_fail();
}

ZTEST(user_settings_list_suite, test_list_iter_empty)
{
	/* empty list can be iterated over but returns NULL immediately */
	user_settings_list_iter_start();

	struct user_setting *us = user_settings_list_iter_next();
	zassert_is_null(us, "Iterating empty list should return NULL");
}

ZTEST(user_settings_list_suite, test_list_iter)
{
	/* add some items */
	user_settings_list_add_fixed_size(1, "t1", USER_SETTINGS_TYPE_BOOL);
	user_settings_list_add_fixed_size(2, "t2", USER_SETTINGS_TYPE_U16);
	user_settings_list_add_fixed_size(3, "t3", USER_SETTINGS_TYPE_U32);
	user_settings_list_add_variable_size(4, "t4", USER_SETTINGS_TYPE_STR, 10);

	/* check that iteration is in order */
	struct user_setting *us;
	user_settings_list_iter_start();

	us = user_settings_list_iter_next();
	zassert_not_null(us, "Item should be in list");
	zassert_equal(us->id, 1, "Id of item is wrong");

	us = user_settings_list_iter_next();
	zassert_not_null(us, "Item should be in list");
	zassert_equal(us->id, 2, "Id of item is wrong");

	us = user_settings_list_iter_next();
	zassert_not_null(us, "Item should be in list");
	zassert_equal(us->id, 3, "Id of item is wrong");

	us = user_settings_list_iter_next();
	zassert_not_null(us, "Item should be in list");
	zassert_equal(us->id, 4, "Id of item is wrong");

	us = user_settings_list_iter_next();
	zassert_is_null(us, "Iter should be NULL after all elements have been returned");

	us = user_settings_list_iter_next();
	zassert_is_null(us, "Iter should be NULL after all elements have been returned");

	us = user_settings_list_iter_next();
	zassert_is_null(us, "Iter should be NULL after all elements have been returned");
}

ZTEST(user_settings_list_suite, test_list_iter_reset)
{
	/* add some items */
	user_settings_list_add_fixed_size(1, "t1", USER_SETTINGS_TYPE_BOOL);
	user_settings_list_add_fixed_size(2, "t2", USER_SETTINGS_TYPE_U16);
	user_settings_list_add_fixed_size(3, "t3", USER_SETTINGS_TYPE_U32);
	user_settings_list_add_variable_size(4, "t4", USER_SETTINGS_TYPE_STR, 10);

	/* start iterating */
	struct user_setting *us;
	user_settings_list_iter_start();
	user_settings_list_iter_next();
	user_settings_list_iter_next();

	/* start again - we should be at the beginning of the list */
	user_settings_list_iter_start();
	us = user_settings_list_iter_next();
	zassert_not_null(us, "Item should be in list");
	zassert_equal(us->id, 1, "Id of item is wrong");
}

ZTEST(user_settings_list_suite, test_list_find_by_key)
{
	/* add some items */
	user_settings_list_add_fixed_size(1, "t1", USER_SETTINGS_TYPE_BOOL);
	user_settings_list_add_fixed_size(2, "t2", USER_SETTINGS_TYPE_U16);
	user_settings_list_add_fixed_size(3, "t3", USER_SETTINGS_TYPE_U32);
	user_settings_list_add_variable_size(4, "t4", USER_SETTINGS_TYPE_STR, 10);

	/* find item and  check it */
	struct user_setting *us;

	us = user_settings_list_get_by_key("t1");
	zassert_equal(us->id, 1, "Id of item is wrong");
	zassert_equal(strcmp("t1", us->key), 0, "Key of item is wrong");

	us = user_settings_list_get_by_key("t4");
	zassert_equal(us->id, 4, "Id of item is wrong");
	zassert_equal(strcmp("t4", us->key), 0, "Key of item is wrong");

	us = user_settings_list_get_by_key("t3");
	zassert_equal(us->id, 3, "Id of item is wrong");
	zassert_equal(strcmp("t3", us->key), 0, "Key of item is wrong");

	us = user_settings_list_get_by_key("t2");
	zassert_equal(us->id, 2, "Id of item is wrong");
	zassert_equal(strcmp("t2", us->key), 0, "Key of item is wrong");

	/* try to get non-existent item */
	us = user_settings_list_get_by_key("t0");
	zassert_is_null(us, "NULL should be returned when a non-existent setting is got");
}

ZTEST(user_settings_list_suite, test_list_find_by_id)
{
	/* add some items */
	user_settings_list_add_fixed_size(1, "t1", USER_SETTINGS_TYPE_BOOL);
	user_settings_list_add_fixed_size(2, "t2", USER_SETTINGS_TYPE_U16);
	user_settings_list_add_fixed_size(3, "t3", USER_SETTINGS_TYPE_U32);
	user_settings_list_add_variable_size(4, "t4", USER_SETTINGS_TYPE_STR, 10);

	/* find item and  check it */
	struct user_setting *us;

	us = user_settings_list_get_by_id(1);
	zassert_equal(us->id, 1, "Id of item is wrong");
	zassert_equal(strcmp("t1", us->key), 0, "Key of item is wrong");

	us = user_settings_list_get_by_id(4);
	zassert_equal(us->id, 4, "Id of item is wrong");
	zassert_equal(strcmp("t4", us->key), 0, "Key of item is wrong");

	us = user_settings_list_get_by_id(3);
	zassert_equal(us->id, 3, "Id of item is wrong");
	zassert_equal(strcmp("t3", us->key), 0, "Key of item is wrong");

	us = user_settings_list_get_by_id(2);
	zassert_equal(us->id, 2, "Id of item is wrong");
	zassert_equal(strcmp("t2", us->key), 0, "Key of item is wrong");

	/* try to get non-existent item */
	us = user_settings_list_get_by_id(0);
	zassert_is_null(us, "NULL should be returned when a non-existent setting is got");
}
