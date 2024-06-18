/** @file user_settings_json.h
 *
 * @brief JSON encode/decode module
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2023 Irnas.  All rights reserved.
 */

#ifndef USER_SETTINGS_JSON_H
#define USER_SETTINGS_JSON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/kernel.h>
#include <user_settings_types.h>
/* JSON parser */
#include <cJSON.h>
#include <cJSON_os.h>

/**
 * @brief Set settings from a flat json structure
 *
 * Does not modify the passed in JSON.
 *
 * Expected structure looks like:
 * {
 *   "s_key_1": <value>,
 *   "s_key_2": <value>,
 *   // ...
 * }
 *
 * @param[in] settings The settings to apply
 * @param[in] always_mark_changed If true, always mark settings as changed, even if the new value is
 * the same as the old one. If false, a setting will only be marked
 * changed if the new value is different from the old one.
 *
 * @retval 0 On success
 * @retval -ENOMEM If the new value is larger than the max_size
 * @retval -EIO if the setting value could not be stored to NVS
 * @retval -EINVAL if the invalid json structure
 */
int user_settings_set_from_json(const cJSON *settings, bool always_mark_changed);

/**
 * @brief Create a JSON with containing only settings marked changed.
 *
 * Calling this function will not clear the changed flag of any user setting.
 *
 * The caller is expected to free the created cJSON structure.
 *
 * @param[out] settings Created json
 * @retval 0 On success
 * @retval -ENOMEM If we failed to allocate JSON struct
 */
int user_settings_get_changed_json(cJSON **settings);

/**
 * @brief Create a JSON with containing all settings.
 *
 * The caller is expected to free the created cJSON structure.
 *
 * @param[out] settings Created json
 * @retval 0 On success
 * @retval -ENOMEM If we failed to allocate JSON struct
 */
int user_settings_get_all_json(cJSON **settings);

#ifdef __cplusplus
}
#endif

#endif // USER_SETTINGS_JSON_H
