/** @file settings_shell_cmds.c
 *
 * @brief Custom implementation for settings shell command so we are able to set
 * settings also
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2022 Irnas. All rights reserved.
 */

#include "user_settings_list.h"

#include <user_settings.h>

#include <zephyr/shell/shell.h>
#include <zephyr/sys/util.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FMT_SETTING(fmt)                     "id: %d, key: \"%s\", value: " fmt ", default: " fmt
#define FMT_SETTING_NO_VALUE(fmt)            "id: %d, key: \"%s\", value: /, default: " fmt
#define FMT_SETTING_NO_DEFAULT(fmt)          "id: %d, key: \"%s\", value: " fmt ", default: /"
#define FMT_SETTING_NO_VALUE_NO_DEFAULT(fmt) "id: %d, key: \"%s\", value: /, default: /"

#define SETTING_PRINT(setting, fmt, cast)                                                          \
	do {                                                                                       \
		if (setting->is_set && setting->default_is_set) {                                  \
			shell_print(shell_ptr, FMT_SETTING(fmt), setting->id, setting->key,        \
				    cast setting->data, cast setting->default_data);               \
		} else if (setting->is_set && !setting->default_is_set) {                          \
			shell_print(shell_ptr, FMT_SETTING_NO_DEFAULT(fmt), setting->id,           \
				    setting->key, cast setting->data);                             \
		} else if (!setting->is_set && setting->default_is_set) {                          \
			shell_print(shell_ptr, FMT_SETTING_NO_VALUE(fmt), setting->id,             \
				    setting->key, cast setting->default_data);                     \
		} else {                                                                           \
			shell_print(shell_ptr, FMT_SETTING_NO_VALUE_NO_DEFAULT(fmt), setting->id,  \
				    setting->key);                                                 \
		}                                                                                  \
	} while (0);

/*
 * Format to print:
 *
 * id: ID, key: KEY, value: VALUE, default: DEFAULT
 */
static void prv_shell_print_setting(const struct shell *shell_ptr, struct user_setting *setting)
{
	switch (setting->type) {
	case USER_SETTINGS_TYPE_BOOL:
		SETTING_PRINT(setting, "%d", *(bool *));
		break;
	case USER_SETTINGS_TYPE_U8:
		SETTING_PRINT(setting, "%u", *(uint8_t *));
		break;
	case USER_SETTINGS_TYPE_I8:
		SETTING_PRINT(setting, "%d", *(int8_t *));
		break;
	case USER_SETTINGS_TYPE_U16:
		SETTING_PRINT(setting, "%u", *(uint16_t *));
		break;
	case USER_SETTINGS_TYPE_I16:
		SETTING_PRINT(setting, "%d", *(int16_t *));
		break;
	case USER_SETTINGS_TYPE_U32:
		SETTING_PRINT(setting, "%u", *(uint32_t *));
		break;
	case USER_SETTINGS_TYPE_I32:
		SETTING_PRINT(setting, "%d", *(int32_t *));
		break;
	case USER_SETTINGS_TYPE_U64:
		SETTING_PRINT(setting, "%llu", *(uint64_t *));
		break;
	case USER_SETTINGS_TYPE_I64:
		SETTING_PRINT(setting, "%lld", *(int64_t *));
		break;
	case USER_SETTINGS_TYPE_STR:
		SETTING_PRINT(setting, "\"%s\"", (char *));
		break;
	case USER_SETTINGS_TYPE_BYTES:
		/* bytes can not be handled with the above macro */

		shell_fprintf(shell_ptr, SHELL_NORMAL, "id: %d, key: \"%s\", value: ", setting->id,
			      setting->key);
		if (setting->is_set) {
			for (int i = 0; i < setting->data_len; i++) {
				shell_fprintf(shell_ptr, SHELL_NORMAL, "%02X",
					      ((uint8_t *)setting->data)[i]);
			}
		} else {
			shell_fprintf(shell_ptr, SHELL_NORMAL, "/");
		}
		shell_fprintf(shell_ptr, SHELL_NORMAL, ", default: ");

		if (setting->default_is_set) {
			for (int i = 0; i < setting->default_data_len; i++) {
				shell_fprintf(shell_ptr, SHELL_NORMAL, "%02X",
					      ((uint8_t *)setting->default_data)[i]);
			}
		} else {
			shell_fprintf(shell_ptr, SHELL_NORMAL, "/");
		}
		shell_fprintf(shell_ptr, SHELL_NORMAL, "\n");
		break;
	}
}

static int cmd_list(const struct shell *shell_ptr, size_t argc, char *argv[])
{
	user_settings_list_iter_start();
	struct user_setting *setting;
	while ((setting = user_settings_list_iter_next()) != NULL) {
		prv_shell_print_setting(shell_ptr, setting);
	}

	return 0;
}

static int cmd_list_changed(const struct shell *shell_ptr, size_t argc, char *argv[])
{
	user_settings_list_iter_start();
	struct user_setting *setting;
	while ((setting = user_settings_list_iter_next()) != NULL) {
		if (setting->has_changed_recently) {
			prv_shell_print_setting(shell_ptr, setting);
		}
	}

	return 0;
}

static int cmd_get(const struct shell *shell_ptr, size_t argc, char *argv[])
{
	const char *key = argv[1];

	struct user_setting *s = user_settings_list_get_by_key(key);
	if (!s) {
		shell_error(shell_ptr, "Setting with this key not found: %s", key);
		return -ENOENT;
	}

	prv_shell_print_setting(shell_ptr, s);

	return 0;
}

static int prv_set_helper(const char *value, struct user_setting *s,
			  int setter_f(char *key, void *data, size_t len))
{
	/* Save value correctly based on type */
	switch (s->type) {
	case USER_SETTINGS_TYPE_BOOL: {
		bool v = strtol(value, NULL, 10);
		return setter_f(s->key, &v, sizeof(v));
	}
	case USER_SETTINGS_TYPE_I8: {
		int8_t v = strtol(value, NULL, 10);
		return setter_f(s->key, &v, sizeof(v));
	}
	case USER_SETTINGS_TYPE_I16: {
		int16_t v = strtol(value, NULL, 10);
		return setter_f(s->key, &v, sizeof(v));
	}
	case USER_SETTINGS_TYPE_I32: {
		int32_t v = strtol(value, NULL, 10);
		return setter_f(s->key, &v, sizeof(v));
	}
	case USER_SETTINGS_TYPE_I64: {
		int64_t v = strtoll(value, NULL, 10);
		return setter_f(s->key, &v, sizeof(v));
	}
	case USER_SETTINGS_TYPE_U8: {
		uint8_t v = strtol(value, NULL, 10);
		return setter_f(s->key, &v, sizeof(v));
	}
	case USER_SETTINGS_TYPE_U16: {
		uint16_t v = strtol(value, NULL, 10);
		return setter_f(s->key, &v, sizeof(v));
	}
	case USER_SETTINGS_TYPE_U32: {
		uint32_t v = strtol(value, NULL, 10);
		return setter_f(s->key, &v, sizeof(v));
	}
	case USER_SETTINGS_TYPE_U64: {
		uint64_t v = strtoll(value, NULL, 10);
		return setter_f(s->key, &v, sizeof(v));
	}
	case USER_SETTINGS_TYPE_STR: {
		char *v = (char *)value;
		return setter_f(s->key, v, strlen(value) + 1);
	}
	case USER_SETTINGS_TYPE_BYTES: {
		/* convert hex string to byte array */
		uint8_t bytes[1024];
		size_t bytes_len = strlen(value) / 2;

		for (size_t i = 0, j = 0; i < bytes_len; i++, j += 2) {
			bytes[i] = (value[j] % 32 + 9) % 25 * 16 + (value[j + 1] % 32 + 9) % 25;
		}
		return setter_f(s->key, bytes, bytes_len);
	}
	}

	__ASSERT(0, "How did we get here? All setting types should be handled by the above switch");
	return 0;
}

static int cmd_set(const struct shell *shell_ptr, size_t argc, char *argv[])
{
	const char *name = argv[1];
	const char *value = argv[2];

	/* Check if key exists */
	struct user_setting *s = user_settings_list_get_by_key(name);
	if (!s) {
		shell_error(shell_ptr, "Setting with this key not found: %s", name);
		return -ENOENT;
	}

	return prv_set_helper(value, s, user_settings_set_with_key);
}

static int cmd_set_default(const struct shell *shell_ptr, size_t argc, char *argv[])
{
	const char *name = argv[1];
	const char *value = argv[2];

	/* Check if key exists */
	struct user_setting *s = user_settings_list_get_by_key(name);
	if (!s) {
		shell_error(shell_ptr, "Setting with this key not found: %s", name);
		return -ENOENT;
	}

	return prv_set_helper(value, s, user_settings_set_default_with_key);
}

static int cmd_restore(const struct shell *shell_ptr, size_t argc, char *argv[])
{
	user_settings_restore_defaults();

	return 0;
}

static int cmd_restore_one(const struct shell *shell_ptr, size_t argc, char *argv[])
{
	char *key = argv[1];

	struct user_setting *s = user_settings_list_get_by_key(key);
	if (!s) {
		shell_error(shell_ptr, "Setting with this key not found: %s", key);
		return -ENOENT;
	}

	user_settings_restore_default_with_key(key);

	return 0;
}

static int cmd_clear_changed(const struct shell *shell_ptr, size_t argc, char *argv[])
{
	user_settings_clear_changed();
	return 0;
}

static int cmd_clear_changed_one(const struct shell *shell_ptr, size_t argc, char *argv[])
{
	char *key = argv[1];

	struct user_setting *s = user_settings_list_get_by_key(key);
	if (!s) {
		shell_error(shell_ptr, "Setting with this key not found: %s", key);
		return -ENOENT;
	}

	user_settings_clear_changed_with_key(s->key);
	return 0;
}

/**
 * @brief Get user setting at position @p idx in the list
 *
 * @param[in] idx The index to get
 * @return struct user_setting* The user setting, NULL if no user setting is at that index
 */
static struct user_setting *prv_get_us_by_idx(size_t idx)
{
	user_settings_list_iter_start();
	struct user_setting *setting;
	int c = 0;
	while ((setting = user_settings_list_iter_next()) != NULL) {
		if (idx == c) {
			return setting;
		}
		c++;
	}

	return NULL;
}

/**
 * @brief Provide a list of user settings keys as dynamic subcommands
 *
 * This is done to make tab-completion suggest user setting keys
 */
static void prv_shell_key_get(size_t idx, struct shell_static_entry *entry)
{
	struct user_setting *s = prv_get_us_by_idx(idx);

	entry->syntax = s ? s->key : NULL;
	entry->handler = NULL;
	entry->help = NULL;
	entry->subcmd = NULL;
}

// create a dynamic set of subcommands. This is called every time
// "voltage_divider get" is invoked in a shell and will create a list of devices
// with @device_name_get
SHELL_DYNAMIC_CMD_CREATE(dsub_setting_key, prv_shell_key_get);

SHELL_STATIC_SUBCMD_SET_CREATE(
	settings_cmds, SHELL_CMD_ARG(list, NULL, "list all user settings", cmd_list, 1, 0),
	SHELL_CMD_ARG(list_changed, NULL, "list all user settings", cmd_list_changed, 1, 0),
	SHELL_CMD_ARG(get, &dsub_setting_key, "<name> List one user setting", cmd_get, 2, 0),
	SHELL_CMD_ARG(set, &dsub_setting_key, "<name> <value> Set a user setting", cmd_set, 3, 0),
	SHELL_CMD_ARG(set_default, &dsub_setting_key,
		      "<name> <value> Set the default value for one user setting", cmd_set_default,
		      3, 0),
	SHELL_CMD_ARG(restore, NULL, "Restore all settings to default values", cmd_restore, 1, 0),
	SHELL_CMD_ARG(restore_one, NULL, "Restore one setting to its default value",
		      cmd_restore_one, 2, 0),
	SHELL_CMD_ARG(clear_changed, NULL, "Clear the changed flag for all settings",
		      cmd_clear_changed, 1, 0),
	SHELL_CMD_ARG(clear_changed_one, NULL, "Clear the changed flag for one setting",
		      cmd_clear_changed_one, 2, 0),
	SHELL_SUBCMD_SET_END);

static int cmd_settings(const struct shell *shell_ptr, size_t argc, char **argv)
{
	shell_error(shell_ptr, "%s unknown parameter: %s", argv[0], argv[1]);
	return -EINVAL;
}

SHELL_CMD_ARG_REGISTER(usettings, &settings_cmds, "Settings shell commands", cmd_settings, 2, 0);
