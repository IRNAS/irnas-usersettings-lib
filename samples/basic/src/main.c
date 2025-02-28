#include <zephyr/kernel.h>

#include <user_settings.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

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

	/* Load settings - this will set every setting to its value from NVS, or a default value (if
	 * it is set)
	 */
	user_settings_load();

	LOG_INF("Use the shell to list, get and set the setting values");
	LOG_INF("Reboot the device to see that settings are reboot persistent");

	return 0;
}
