#include <zephyr/kernel.h>

#include <user_settings.h>
#include <user_settings_validation_helpers.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

/* Define vaidation callbacks using the helper macros */
USER_SETTINGS_VALIDATE_CB_RANGE(prv_validate_t2, uint8_t, 20, 29);
USER_SETTINGS_VALIDATE_CB_RANGE(prv_validate_t3, uint16_t, 30, 39);
USER_SETTINGS_VALIDATE_CB_RANGE(prv_validate_t4, uint32_t, 40, 49);
USER_SETTINGS_VALIDATE_CB_RANGE(prv_validate_t5, uint64_t, 50, 59);
USER_SETTINGS_VALIDATE_CB_RANGE(prv_validate_t6, int8_t, -69, -60);
USER_SETTINGS_VALIDATE_CB_RANGE(prv_validate_t7, int16_t, -79, -70);
USER_SETTINGS_VALIDATE_CB_RANGE(prv_validate_t8, int32_t, -89, -80);
USER_SETTINGS_VALIDATE_CB_RANGE(prv_validate_t9, int64_t, -99, -90);
USER_SETTINGS_VALIDATE_CB_EXACT_SIZE(validate_t10, 10);
USER_SETTINGS_VALIDATE_CB_EXACT_SIZE(validate_t11, 8);

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

	/* Add validation callbacks */
	user_settings_set_validate_cb_with_key("t2", prv_validate_t2);
	user_settings_set_validate_cb_with_key("t3", prv_validate_t3);
	user_settings_set_validate_cb_with_key("t4", prv_validate_t4);
	user_settings_set_validate_cb_with_key("t5", prv_validate_t5);
	user_settings_set_validate_cb_with_key("t6", prv_validate_t6);
	user_settings_set_validate_cb_with_key("t7", prv_validate_t7);
	user_settings_set_validate_cb_with_key("t8", prv_validate_t8);
	user_settings_set_validate_cb_with_key("t9", prv_validate_t9);
	user_settings_set_validate_cb_with_id(10, validate_t10);
	user_settings_set_validate_cb_with_id(11, validate_t11);

	/* Load settings */
	user_settings_load();

	LOG_INF("Use the shell to list, get and set the setting values");
	LOG_INF("Reboot the device to see that settings are reboot persistent");

	k_sleep(K_FOREVER);
	return 0;
}
