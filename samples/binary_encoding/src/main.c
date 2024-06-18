#include <zephyr/kernel.h>

#include <user_settings.h>
#include <user_settings_list.h>
#include <user_settings_protocol_binary.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

/* Use this instead of LOG_HEXDUMP to a get a more compact output */
void print_buf(uint8_t *buf, size_t len, char *text)
{
	LOG_PRINTK("%s: ", text);
	for (int i = 0; i < len; i++) {
		LOG_PRINTK("%02X ", buf[i]);
	}
	LOG_PRINTK("\n");
}

void main(void)
{
/* This sleep is only here for native_posix build so that all log messages will actually be
 * printed. if you don't sleep a bit, they get skipped */
#ifdef CONFIG_BOARD_NATIVE_POSIX
	k_sleep(K_SECONDS(1));
#endif

	LOG_INF("Sample for setting binary encoding");

	/* Initialize settings defaults */
	int err = user_settings_init();
	if (err) {
		LOG_ERR("err: %d", err);
	}

	/* Provide all application settings - one setting of each type is set here for demonstration
	 * purposes */
	user_settings_add(1, "enabled", USER_SETTINGS_TYPE_BOOL);
	user_settings_add(2, "number", USER_SETTINGS_TYPE_U8);
	user_settings_add(3, "hey", USER_SETTINGS_TYPE_U16);
	user_settings_add(4, "yo", USER_SETTINGS_TYPE_U32);
	user_settings_add(5, "lets", USER_SETTINGS_TYPE_U64);
	user_settings_add(6, "go", USER_SETTINGS_TYPE_I8);
	user_settings_add(7, "t7", USER_SETTINGS_TYPE_I16);
	user_settings_add(8, "t8", USER_SETTINGS_TYPE_I32);
	user_settings_add(9, "ttt", USER_SETTINGS_TYPE_I64);
	user_settings_add_sized(10, "text", USER_SETTINGS_TYPE_STR, 10);
	user_settings_add_sized(11, "secret", USER_SETTINGS_TYPE_BYTES, 8);

	/* Load settings - this will set every setting to its value from NVS, or a default value (if
	 * it is set)
	 */
	user_settings_load();

	/* vars to hold data */
	// bool b;
	uint8_t u8;
	uint16_t u16;
	uint32_t u32;
	uint64_t u64;
	int8_t s8;
	int16_t s16;
	int32_t s32;
	int64_t s64;
	char str[] = "banana";
	uint8_t bytes[] = {1, 2, 3, 4, 5, 6};

	/* set default values */
	// b = true;
	// user_settings_set_default_with_id(1, &b, sizeof(b));
	u8 = 13;
	user_settings_set_default_with_id(2, &u8, sizeof(u8));
	u16 = 1337;
	user_settings_set_default_with_id(3, &u16, sizeof(u16));
	u32 = 1234567;
	user_settings_set_default_with_id(4, &u32, sizeof(u32));
	// u64 = 1234567654321;
	// user_settings_set_default_with_id(5, &u64, sizeof(u64));

	s8 = -1;
	user_settings_set_default_with_id(6, &s8, sizeof(s8));
	s16 = 505;
	user_settings_set_default_with_id(7, &s16, sizeof(s16));
	// s32 = -1234567;
	// user_settings_set_default_with_id(8, &s32, sizeof(s32));
	s64 = 65432123456;
	user_settings_set_default_with_id(9, &s64, sizeof(s64));

	user_settings_set_default_with_id(10, str, strlen(str) + 1);
	user_settings_set_default_with_id(11, bytes, sizeof(bytes));

	/* set values */
	// b = false;
	// user_settings_set_with_id(1, &b, sizeof(b));
	u8 = 69;
	user_settings_set_with_id(2, &u8, sizeof(u8));
	// u16 = 420;
	// user_settings_set_with_id(3, &u16, sizeof(u16));
	u32 = 7654321;
	user_settings_set_with_id(4, &u32, sizeof(u32));
	u64 = 1;
	user_settings_set_with_id(5, &u64, sizeof(u64));

	s8 = -2;
	user_settings_set_with_id(6, &s8, sizeof(s8));
	s16 = 202;
	user_settings_set_with_id(7, &s16, sizeof(s16));
	s32 = -7654321;
	user_settings_set_with_id(8, &s32, sizeof(s32));
	s64 = 987656789;
	user_settings_set_with_id(9, &s64, sizeof(s64));

	strcpy(str, "apple");
	user_settings_set_with_id(10, str, strlen(str) + 1);

	for (int i = 0; i < sizeof(bytes); i++) {
		bytes[i] += 7;
	}
	user_settings_set_with_id(11, bytes, sizeof(bytes));

	/* encode each setting */
	uint8_t buffer[256];
	int ret;
	for (int i = 1; i <= 11; i++) {
		struct user_setting *s = user_settings_list_get_by_id(i);
		LOG_PRINTK("ID: %d\n", s->id);
		ret = user_settings_protocol_binary_encode(s, buffer, sizeof(buffer));
		print_buf(buffer, ret, "GET");
		ret = user_settings_protocol_binary_encode_full(s, buffer, sizeof(buffer));
		print_buf(buffer, ret, "GET FULL");
	}

	k_sleep(K_FOREVER);
}
