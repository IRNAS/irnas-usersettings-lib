/** @file user_settings_json.c
 *
 * @brief JSON encode/decode module
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2023 Irnas.  All rights reserved.
 */

#include "user_settings_json.h"
#include "user_settings_list.h"
#include <zephyr/kernel.h>
#include <user_settings.h>
#include <user_settings_types.h>

#include <cJSON.h>
#include <cJSON_os.h>

#include <stdio.h>
#include <string.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(user_settings_json, CONFIG_USER_SETTINGS_LOG_LEVEL);

/**
 * @brief Set value from JSON structure.
 * Function expects we have already checked that setting key and value are valid.
 *
 * @param[in] type - setting type
 * @param[in] setting - setting to set.
 * @param[in] always_mark_changed If true, always mark settings as changed, even if the new value is
 * the same as the old one.
 *
 * @retval 0 On success
 * @retval -ENOMEM If the new value is larger than the max_size
 * @retval -EIO if the setting value could not be stored to NVS
 * @retval -EINVAL if the invalid data format is provided in json structure
 */
static int prv_set_from_json(enum user_setting_type type, cJSON *setting, bool always_mark_changed)
{
	int err = -EINVAL;
	switch (type) {
	case USER_SETTINGS_TYPE_BOOL: {
		if (cJSON_IsBool(setting)) {
			bool v = cJSON_IsTrue(setting);
			err = user_settings_set_with_key(setting->string, &v, sizeof(v));
		}
		break;
	}
	case USER_SETTINGS_TYPE_U8: {
		if (cJSON_IsNumber(setting)) {
			uint8_t v = (uint8_t)setting->valueint;
			err = user_settings_set_with_key(setting->string, &v, sizeof(v));
		}
		break;
	}
	case USER_SETTINGS_TYPE_U16: {
		if (cJSON_IsNumber(setting)) {
			uint16_t v = (uint16_t)setting->valueint;
			err = user_settings_set_with_key(setting->string, &v, sizeof(v));
		}
		break;
	}
	case USER_SETTINGS_TYPE_U32: {
		if (cJSON_IsNumber(setting)) {
			uint32_t v = (uint32_t)setting->valueint;
			err = user_settings_set_with_key(setting->string, &v, sizeof(v));
		}
		break;
	}
	case USER_SETTINGS_TYPE_U64: {
		if (cJSON_IsNumber(setting)) {
			uint64_t v = (uint64_t)setting->valueint;
			err = user_settings_set_with_key(setting->string, &v, sizeof(v));
		}
		break;
	}
	case USER_SETTINGS_TYPE_I8: {
		if (cJSON_IsNumber(setting)) {
			int8_t v = (int8_t)setting->valueint;
			err = user_settings_set_with_key(setting->string, &v, sizeof(v));
		}
		break;
	}
	case USER_SETTINGS_TYPE_I16: {
		if (cJSON_IsNumber(setting)) {
			int16_t v = (int16_t)setting->valueint;
			err = user_settings_set_with_key(setting->string, &v, sizeof(v));
		}
		break;
	}
	case USER_SETTINGS_TYPE_I32: {
		if (cJSON_IsNumber(setting)) {
			int32_t v = (int32_t)setting->valueint;
			err = user_settings_set_with_key(setting->string, &v, sizeof(v));
		}
		break;
	}
	case USER_SETTINGS_TYPE_I64: {
		if (cJSON_IsNumber(setting)) {
			int64_t v = (int64_t)setting->valueint;
			err = user_settings_set_with_key(setting->string, &v, sizeof(v));
		}
		break;
	}
	case USER_SETTINGS_TYPE_STR: {
		if (cJSON_IsString(setting)) {
			char *v = setting->valuestring;
			err = user_settings_set_with_key(setting->string, v,
							 strlen(setting->valuestring) + 1);
		}
		break;
	}
	case USER_SETTINGS_TYPE_BYTES: {
		if (cJSON_IsString(setting)) {
			/* convert hex string to byte array */
			uint8_t bytes[1024];
			size_t bytes_len = strlen(setting->valuestring) / 2;

			for (size_t i = 0, j = 0; i < bytes_len; i++, j += 2) {
				bytes[i] = (setting->valuestring[j] % 32 + 9) % 25 * 16 +
					   (setting->valuestring[j + 1] % 32 + 9) % 25;
			}
			err = user_settings_set_with_key(setting->string, bytes, bytes_len);
		}
		break;
	}
	default: {
		LOG_ERR("Type not supported!");
		err = -EINVAL;
	}
	}

	if (err) {
		return err;
	}

	/* user_settings_set_with_key() only marks a setting as changed if the new value is
	 * different. If we want to always mark them, we have do it explicitly here.
	 *
	 * If the new value is different, the setting will be marked as changed twice, but this is
	 * not a problem.
	 */
	if (always_mark_changed) {
		user_settings_set_changed_with_key(setting->string);
	}

	return 0;
}

/**
 * @brief Create cJSON object of appropriate type from user setting struct.
 *
 * @param setting - user setting struct
 * @return cJSON* setting object. NULL if invalid.
 */
static cJSON *prv_json_from_setting(struct user_setting *setting)
{
	cJSON *json_setting = NULL;

	switch (setting->type) {
	case USER_SETTINGS_TYPE_BOOL: {
		if (*(bool *)setting->data) {
			json_setting = cJSON_CreateTrue();
		} else {
			json_setting = cJSON_CreateFalse();
		}
		break;
	}
	case USER_SETTINGS_TYPE_U8: {
		json_setting = cJSON_CreateNumber((int)*(uint8_t *)setting->data);
		break;
	}
	case USER_SETTINGS_TYPE_U16: {
		json_setting = cJSON_CreateNumber((int)*(uint16_t *)setting->data);
		break;
	}
	case USER_SETTINGS_TYPE_U32: {
		json_setting = cJSON_CreateNumber((int)*(uint32_t *)setting->data);
		break;
	}
	case USER_SETTINGS_TYPE_U64: {
		json_setting = cJSON_CreateNumber((int)*(uint64_t *)setting->data);
		break;
	}
	case USER_SETTINGS_TYPE_I8: {
		json_setting = cJSON_CreateNumber((int)*(int8_t *)setting->data);
		break;
	}
	case USER_SETTINGS_TYPE_I16: {
		json_setting = cJSON_CreateNumber((int)*(int16_t *)setting->data);
		break;
	}
	case USER_SETTINGS_TYPE_I32: {
		json_setting = cJSON_CreateNumber((int)*(int32_t *)setting->data);
		break;
	}
	case USER_SETTINGS_TYPE_I64: {
		json_setting = cJSON_CreateNumber((int)*(int64_t *)setting->data);
		break;
	}
	case USER_SETTINGS_TYPE_STR: {
		json_setting = cJSON_CreateString((char *)setting->data);
		break;
	}
	case USER_SETTINGS_TYPE_BYTES: {
		/* convert bytes to hex string */
		char bytes[setting->data_len * 2 + 1];
		uint8_t *data = setting->data;

		for (size_t i = 0; i < setting->data_len; i++) {
			sprintf(bytes + 2 * i, "%02X", data[i]);
		}
		json_setting = cJSON_CreateString(bytes);
		break;
	}
	default: {
		LOG_ERR("Type not supported!");
	}
	}

	if (json_setting == NULL) {
		LOG_ERR("Invalid json object!");
		cJSON_Delete(json_setting);
		return NULL;
	}

	return json_setting;
}

int user_settings_set_from_json(const cJSON *settings, bool always_mark_changed)
{
	int err;

	if (settings == NULL) {
		LOG_ERR("Empty JSON settings object!");
		return -EINVAL;
	}

	/* Iterate items */
	cJSON *setting = NULL;
	enum user_setting_type type;

	cJSON_ArrayForEach(setting, settings)
	{
		/* Check if valid*/
		if (cJSON_IsInvalid(setting)) {
			LOG_WRN("Invalid setting: %s!", setting->string);
			continue;
		}

		/* Check if key exists */
		if (!user_settings_exists_with_key(setting->string)) {
			LOG_WRN("Key does not exists: %s!", setting->string);
			continue;
		}

		/* Get stored setting type */
		type = user_settings_get_type_with_key(setting->string);

		err = prv_set_from_json(type, setting, always_mark_changed);
		if (err == -EINVAL) {
			LOG_ERR("Invalid json data for setting: %s", setting->string);
		} else if (err) {
			LOG_ERR("Failed to store setting data: %d", err);
			return err;
		}
	}

	return 0;
}

int user_settings_get_all_json(cJSON **settings_out)
{
	/* Create json root object */
	cJSON *settings = cJSON_CreateObject();
	if (settings == NULL) {
		cJSON_Delete(settings);
		return -ENOMEM;
	}

	/* Iterate trough settings */
	user_settings_list_iter_start();
	struct user_setting *setting_data;
	while ((setting_data = user_settings_list_iter_next()) != NULL) {
		cJSON *setting = prv_json_from_setting(setting_data);
		if (setting != NULL) {
			cJSON_AddItemToObject(settings, setting_data->key, setting);
		}
	}

	*settings_out = settings;

	return 0;
}

int user_settings_get_changed_json(cJSON **settings_out)
{
	/* Create json root object */
	cJSON *settings = cJSON_CreateObject();
	if (settings == NULL) {
		cJSON_Delete(settings);
		return -ENOMEM;
	}

	/* Iterate trough settings */
	user_settings_list_iter_start();
	struct user_setting *setting_data;
	while ((setting_data = user_settings_list_iter_next()) != NULL) {
		if (setting_data->has_changed_recently) {
			cJSON *setting = prv_json_from_setting(setting_data);
			if (setting != NULL) {
				cJSON_AddItemToObject(settings, setting_data->key, setting);
			}
		}
	}

	*settings_out = settings;

	return 0;
}
