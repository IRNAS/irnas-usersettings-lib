#include <zephyr/kernel.h>

#include <user_settings.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

/**
 * @brief Called when ANY setting has been changed.
 *
 * Prints the key of the setting that has been changed.
 *
 * @param[in] id The ID of the setting that was changed.
 * @param[in] key The key of the setting that was changed.
 */
static void prv_on_change_global(uint32_t id, const char *key)
{
	LOG_INF("%s has been changed!", key);
}

/**
 * @brief Called when setting with ID 1 has been changed.
 *
 * @param[in] id The ID of the setting that was changed.
 * @param[in] key The key of the setting that was changed.
 */
static void prv_on_change_t1(uint32_t id, const char *key)
{
	uint8_t val = *(bool *)user_settings_get_with_id(1, NULL);
	LOG_INF("t1 changed to %s", val ? "true" : "false");
}

/**
 * @brief Called when any setting with ID 2 through 10 has been changed.
 *
 * @param[in] id The ID of the setting that was changed.
 * @param[in] key The key of the setting that was changed.
 */
static void prv_on_change_t2_t10(uint32_t id, const char *key)
{
	switch (id) {
	case 2: {
		uint8_t val = *(uint8_t *)user_settings_get_with_id(2, NULL);
		LOG_INF("%s has been changed to %d", key, val);
		break;
	}
	case 10: {
		char *val = (char *)user_settings_get_with_id(10, NULL);
		LOG_INF("%s has been changed to \"%s\"", key, val);
		break;
	}
	}
}

/**
 * @brief Called before setting with ID 2 through 9 is changed to validate the value.
 *
 * The value must be in range [id * 10, (id + 1) * 10 - 1] to be valid.
 *
 * @param[in] id The ID of the setting to be validated.
 * @param[in] key The key of the setting to be validated.
 * @param[in] data The new setting value (not applied yet).
 * @param[in] len The length of the new setting value.
 *
 * @retval true If the value is valid.
 * @retval False if the value is not valid.
 */
static bool prv_validate_t2_t9(uint32_t id, const char *key, void *data, size_t len)
{
	/* The value must be in range [id * 10, (id + 1) * 10 - 1] */
	uint64_t val = 0;

	switch (user_settings_get_type_with_id(id)) {
	case USER_SETTINGS_TYPE_U8:
		val = *(uint8_t *)data;
		break;
	case USER_SETTINGS_TYPE_U16:
		val = *(uint16_t *)data;
		break;
	case USER_SETTINGS_TYPE_U32:
		val = *(uint32_t *)data;
		break;
	case USER_SETTINGS_TYPE_U64:
		val = *(uint64_t *)data;
		break;
	case USER_SETTINGS_TYPE_I8:
		val = *(int8_t *)data;
		break;
	case USER_SETTINGS_TYPE_I16:
		val = *(int16_t *)data;
		break;
	case USER_SETTINGS_TYPE_I32:
		val = *(int32_t *)data;
		break;
	case USER_SETTINGS_TYPE_I64:
		val = *(int64_t *)data;
		break;
	default:
		LOG_ERR("Unsupported type");
		return false;
	};

	uint64_t lower = id * 10;
	uint64_t upper = (id + 1) * 10 - 1;

	if (val < lower || val > upper) {
		LOG_ERR("Value must be in range [%lld, %lld]", lower, upper);
		return false;
	}

	return true;
}

/**
 * @brief Called before setting with ID 10 is changed to validate the value.
 *
 * The first character must be an upper case letter.
 *
 * @param[in] id The ID of the setting to be validated.
 * @param[in] key The key of the setting to be validated.
 * @param[in] data The new setting value (not applied yet).
 * @param[in] len The length of the new setting value.
 *
 * @retval true If the value is valid.
 * @retval False if the value is not valid.
 */
static bool validate_t10(uint32_t id, const char *key, void *data, size_t len)
{
	/* first char must be an upper case letter */
	char *s = (char *)data;

	if (s[0] < 'A' || s[0] > 'Z') {
		LOG_ERR("First char must be an upper case letter");
		return false;
	}

	return true;
}

int main(void)
{
/* This sleep is only here for native_sim build so that all log messages will actually be
 * printed. if you don't sleep a bit, they get skipped */
#ifdef CONFIG_BOARD_NATIVE_SIM
	k_sleep(K_SECONDS(1));
#endif

	LOG_INF("Testing settings");

	/* Initialize settings defaults */
	int err = user_settings_init();
	if (err) {
		LOG_ERR("err: %d", err);
	}

	/* Provide all application settings - one setting of each type is set here for demonstration
	 * purposes */
	user_settings_add(1, "t1", USER_SETTINGS_TYPE_BOOL);
	user_settings_add(2, "t2", USER_SETTINGS_TYPE_U8);
	user_settings_add(3, "t3", USER_SETTINGS_TYPE_U16);
	user_settings_add(4, "t4", USER_SETTINGS_TYPE_U32);
	user_settings_add(5, "t5", USER_SETTINGS_TYPE_U64);
	user_settings_add(6, "t6", USER_SETTINGS_TYPE_I8);
	user_settings_add(7, "t7", USER_SETTINGS_TYPE_I16);
	user_settings_add(8, "t8", USER_SETTINGS_TYPE_I32);
	user_settings_add(9, "t9", USER_SETTINGS_TYPE_I64);
	user_settings_add_sized(10, "t10", USER_SETTINGS_TYPE_STR, 10);
	user_settings_add_sized(11, "t11", USER_SETTINGS_TYPE_BYTES, 8);

	/* Setting callbacks can be set either before or after calling user_settings_load.
	 * If they are set before, then the callback will be called during the initial load of the
	 * settings.
	 */

	/* register global callback - this is called for EVERY setting change */
	user_settings_set_global_on_change_cb(prv_on_change_global);

	/* set specific callback for some settings */
	user_settings_set_on_change_cb_with_id(1, prv_on_change_t1);

	user_settings_set_on_change_cb_with_key("t2", prv_on_change_t2_t10);
	user_settings_set_on_change_cb_with_id(10, prv_on_change_t2_t10);

	/* Add validation callbacks */
	user_settings_set_validate_cb_with_key("t2", prv_validate_t2_t9);
	user_settings_set_validate_cb_with_key("t3", prv_validate_t2_t9);
	user_settings_set_validate_cb_with_key("t4", prv_validate_t2_t9);
	user_settings_set_validate_cb_with_key("t5", prv_validate_t2_t9);
	user_settings_set_validate_cb_with_key("t6", prv_validate_t2_t9);
	user_settings_set_validate_cb_with_key("t7", prv_validate_t2_t9);
	user_settings_set_validate_cb_with_key("t8", prv_validate_t2_t9);
	user_settings_set_validate_cb_with_key("t9", prv_validate_t2_t9);
	user_settings_set_validate_cb_with_id(10, validate_t10);

	/* Load settings */
	user_settings_load();

	LOG_INF("Use the shell to list, get and set the setting values");
	LOG_INF("Reboot the device to see that settings are reboot persistent");

	return 0;
}
