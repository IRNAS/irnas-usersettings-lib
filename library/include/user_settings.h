/** @file user_settings.h
 *
 * @brief User settings.
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2022 Irnas.  All rights reserved.
 */

#ifndef USER_SETTINGS_H
#define USER_SETTINGS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/kernel.h>
#include <user_settings_types.h>

/**
 * @brief Initialize the user settings module
 *
 * This will set up facilities for loading and storing setting values
 * from NVS, but will not load any settings.
 *
 * Should only be called once.
 *
 * @retval 0 on success
 * @retval -EIO if initializing the underlying zephyr settings lib fails
 */
int user_settings_init(void);

/**
 * @brief Add a user setting with known size
 *
 * This makes the setting known to the module, so it can be subsequently loaded,
 * get and set.
 * Use this function for all setting types with a fixed size - all except for
 * string and bytes.
 *
 * All settings of a device should be added before calling user_settings_load
 *
 * @param[in] id The ID of the setting to add. Must be unique to all other settings
 * @param[in] key The key of the setting to add. Must be unique to all other settings. The string
 * behind the pointer must live for the lifetime of the program (should be static/hardcoded)
 * @param[in] type The type of the setting to add
 */
void user_settings_add(uint16_t id, const char *key, enum user_setting_type type);

/**
 * @brief Add a user setting with unknown size
 *
 * Behaves the same as user_settings_add(), but should be used for the string and bytes type,
 * since the length can not be inferred from the type itself.
 *
 * @param[in] id The ID of the setting to add. Must be unique to all other settings
 * @param[in] key The key of the setting to add. Must be unique to all other settings. The string
 * behind the pointer must live for the lifetime of the program (should be static/hardcoded)
 * @param[in] type The type of the setting to add
 * @param[in] max_size The maximum size of the value
 */
void user_settings_add_sized(uint16_t id, const char *key, enum user_setting_type type,
			     size_t max_size);

/**
 * @brief Load add setting values and default from NVS
 *
 * Make sure you call user_settings_add() for all settings supported by your application
 * before calling this function.
 *
 * This will load each setting value with its saved value, or it's default if no value was set.
 * If no default exists for a setting, its value's memory will be set to 0. It is recommended
 * that each setting has a default value to make reasoning about setting validity easier.
 *
 * @retval 0 on success
 * @retval -EIO if loading values from NVS fails
 */
int user_settings_load(void);

/**
 * @brief Set the default value of a setting
 *
 * This will set the default value of a setting and store it to NVS if no default
 * value for that setting exists.
 * If a default value has already been set, this will return -EALREADY.
 *
 * For string types, @p len must include the NULL terminator (len = strlen(data) + 1).
 *
 * Setting a new default value can only be achieved by clearing NVS first.
 *
 * It is envisioned that this will be used in a factory provisioning step to load default
 * setting values for all settings. The final application will then have default values
 * for all settings and only custom setting values can then be set with user_settings_set_*()
 *
 * Will assert of a setting with this key does not exist.
 * If the key input for this function is unknown to the application (i.e. parsed from user), then
 * it should first be checked with user_settings_exists_with_key().
 *
 * @note If overwriting default values is required, set CONFIG_USER_SETTINGS_DEFAULT_OVERWRITE=y.
 * This can be used in use-cases where the default values are set form within the application, and a
 * firmware update can change the default values. In that case, -EALREADY is never returned.
 *
 * @param[in] key The key of the setting to set
 * @param[in] data The default value
 * @param[in] len The length of the default value (in bytes)
 *
 * @retval 0 on success
 * @retval -EALREADY if the default value has already been set. Only returned if
 * CONFIG_USER_SETTINGS_DEFAULT_OVERWRITE=n.
 * @retval -ENOMEM if len is >= then the max_size specified when adding the setting with
 * user_settings_add() or user_settings_add_sized()
 * @retval -EIO if the setting value could not be stored to NVS
 */
int user_settings_set_default_with_key(char *key, void *data, size_t len);

/**
 * @brief Behaves the same as user_settings_set_default_with_key()
 *
 * Will assert of a setting with this ID does not exist.
 * If the ID input for this function is unknown to the application (i.e. parsed from user), then
 * it should first be checked with user_settings_exists_with_id().
 *
 * @param[in] id The ID of the setting to set
 * @param[in] data The default value
 * @param[in] len The length of the default value (in bytes)
 *
 * @return See user_settings_set_default_with_key()
 */
int user_settings_set_default_with_id(uint16_t id, void *data, size_t len);

/**
 * @brief Set each setting value to it's default value
 *
 * This will reset all setting values to their defaults and store those to NVS.
 * If no default exists for a setting, the value remains unchanged.
 *
 */
void user_settings_restore_defaults(void);

/**
 * @brief Set setting value, defined by key to it's default value
 *
 * This will reset key specific setting value to its default and store it to NVS.
 * If no default exists for a setting, the value is still deleted. Calls to
 * @user_settings_get_with_*() will return NULL.
 *
 * @param[in] key The key of the setting to set
 *
 * @retval 0 on success
 * @retval -EIO if no default exists for a setting
 */
int user_settings_restore_default_with_key(char *key);

/**
 * @brief Set setting value, defined by id to it's default value
 *
 * See @user_settings_restore_default_with_key()
 *
 * @param[in] id The ID of the setting to set
 *
 * @retval 0 on success
 * @retval -EIO if no default exists for a setting
 */
int user_settings_restore_default_with_id(uint16_t id);

/**
 * @brief Check if a user setting with the provided key exists
 *
 * @param[in] key The key to check
 *
 * @retval true If a setting with this key exists
 * @retval false If a setting with this key does not exist
 */
bool user_settings_exists_with_key(char *key);

/**
 * @brief Check if a user setting with the provided id exists
 *
 * @param[in] id The id to check
 *
 * @retval true If a setting with this id exists
 * @retval false If a setting with this id does not exist
 */
bool user_settings_exists_with_id(uint16_t id);

/**
 * @brief Set a settings value
 *
 * Set a settings value and store it to NVS
 * This will also call the global and setting specific on_change callback, if registered.
 *
 * For string types, @p len must include the NULL terminator (len = strlen(data) + 1).
 *
 * Will assert of a setting with this key does not exist.
 * If the key input for this function is unknown to the application (i.e. parsed from user), then
 * it should first be checked with user_settings_exists_with_key().
 *
 * @param[in] key The key of the setting to set
 * @param[in] data The default value
 * @param[in] len The length of the value (in bytes)
 *
 * @retval 0 On success
 * @retval -ENOMEM If the new value is larger than the max_size
 * @retval -EIO if the setting value could not be stored to NVS
 */
int user_settings_set_with_key(char *key, void *data, size_t len);

/**
 * @brief Set a settings value
 *
 * Behaves the same as user_settings_set_with_key()
 *
 * Will assert of a setting with this ID does not exist.
 * If the ID input for this function is unknown to the application (i.e. parsed from user), then
 * it should first be checked with user_settings_exists_with_id().
 *
 * @param[in] id  The ID of the setting to set
 * @param[in] data The default value
 * @param[in] len The length of the value (in bytes)
 *
 * @return int See user_settings_set_with_key()
 */
int user_settings_set_with_id(uint16_t id, void *data, size_t len);

/**
 * @brief Get a settings value
 *
 * If the setting has a value set, return the value.
 * If no value is set but a default value is set, return the default value.
 * If no value and no default value are set, return NULL.
 *
 * The out parameter @p len is useful for the string and bytes setting types,
 * when a consumer of the setting value might not know the length of the array.
 *
 * Will assert of a setting with this key does not exist.
 * If the key input for this function is unknown to the application (i.e. parsed from user), then
 * it should first be checked with user_settings_exists_with_key().
 *
 * @param[in] key The key of the setting to get
 * @param[out] len The length of the setting value. Can be NULL
 *
 * @return void* A pointer to the settings value. The consumer of the setting value is expected to
 * know the type of the setting in order to be able to cast the pointer to the correct type.
 */
void *user_settings_get_with_key(char *key, size_t *len);

/**
 * @brief Get a settings value
 *
 * See user_settings_get_with_key()
 *
 * Will assert of a setting with this ID does not exist.
 * If the ID input for this function is unknown to the application (i.e. parsed from user), then
 * it should first be checked with user_settings_exists_with_id().
 *
 * @param[in] id The ID of the setting to get
 * @param[out] len The length of the setting value. Can be NULL
 *
 * @return void* A pointer to the settings value. The consumer of the setting value is expected to
 * know the type of the setting in order to be able to cast the pointer to the correct type.
 */
void *user_settings_get_with_id(uint16_t id, size_t *len);

/**
 * @brief Get a settings default value
 *
 * If no default value is set, return NULL.
 *
 * The out parameter @p len is useful for the string and bytes setting types,
 * when a consumer of the setting value might not know the length of the array.
 *
 * Will assert of a setting with this key does not exist.
 * If the key input for this function is unknown to the application (i.e. parsed from user), then
 * it should first be checked with user_settings_exists_with_key().
 *
 * @param[in] key The key of the setting to get
 * @param[out] len The length of the setting default value. Can be NULL
 *
 * @return void* A pointer to the settings default value. The consumer of the value is
 * expected to know the type of the setting in order to be able to cast the pointer to the correct
 * type.
 */
void *user_settings_get_default_with_key(char *key, size_t *len);

/**
 * @brief GGet a settings default value
 *
 * See user_settings_get_default_with_key()
 *
 * Will assert of a setting with this ID does not exist.
 * If the ID input for this function is unknown to the application (i.e. parsed from user), then
 * it should first be checked with user_settings_exists_with_id().
 *
 * @param[in] id The ID of the setting to get
 * @param[out] len The length of the setting default value. Can be NULL
 *
 * @return void* A pointer to the settings default value. The consumer of the value is
 * expected to know the type of the setting in order to be able to cast the pointer to the correct
 * type.
 */
void *user_settings_get_default_with_id(uint16_t id, size_t *len);

/**
 * @brief Set the on change callback for changes on all settings
 *
 * The provided function is called when any setting is updated to a new value.
 * it is not called if the setting is updated to the same value it already has.
 *
 * @param[in] on_change_cb The callback function. NULL to disable the
 * notification.
 */
void user_settings_set_global_on_change_cb(user_settings_on_change_t on_change_cb);

/**
 * @brief Set the on change callback for changes to a specific setting
 *
 * The provided function is called when the provided setting is updated to a new value.
 * it is not called if the setting is updated to the same value it already has.
 *
 * This will assert if no setting with the provided key exists.
 * If the key input for this function is unknown to the application (i.e. parsed from user), then
 * it should first be checked with user_settings_exists_with_key().
 *
 * @param[in] key The key of the setting to set the callback on
 * @param[in] on_change_cb The callback function. NULL to disable the
 * notification.
 */
void user_settings_set_on_change_cb_with_key(char *key, user_settings_on_change_t on_change_cb);

/**
 * @brief Set the on change callback for changes to a specific setting
 *
 * This behaves the same as user_settings_set_on_change_cb_with_key()
 *
 * This will assert if no setting with the provided ID exists.
 * If the ID input for this function is unknown to the application (i.e. parsed from user), then
 * it should first be checked with user_settings_exists_with_id().
 *
 * @param[in] id The ID of the setting to set the callback on
 * @param[in] on_change_cb The callback function. NULL to disable the
 * notification.
 */
void user_settings_set_on_change_cb_with_id(uint16_t id, user_settings_on_change_t on_change_cb);

/**
 * @brief Check if a setting has its value set
 *
 * This is only false if no default value for this setting exists and if
 * no value was ever set.
 *
 * This will assert if no setting with the provided key exists.
 * If the key input for this function is unknown to the application (i.e. parsed from user), then
 * it should first be checked with user_settings_exists_with_key().
 *
 * @param[in] key The key of the setting to check
 *
 * @return true If the setting has a value
 * @return false If the setting has no value
 */
bool user_settings_is_set_with_key(char *key);

/**
 * @brief Check if a setting has its value set
 *
 * This is only false of no default value for this setting exists and if
 * no value was ever set.
 *
 * This will assert if no setting with the provided ID exists.
 * If the ID input for this function is unknown to the application (i.e. parsed from user), then
 * it should first be checked with user_settings_exists_with_id().
 *
 * @param[in] id The ID of the setting to check
 *
 * @return true If the setting has a value
 * @return false If the setting has no value
 */
bool user_settings_is_set_with_id(uint16_t id);

/**
 * @brief Check if a setting has its default value set
 *
 * This will assert if no setting with the provided key exists.
 * If the key input for this function is unknown to the application (i.e. parsed from user), then
 * it should first be checked with user_settings_exists_with_key().
 *
 * @param[in] key The key of the setting to check
 *
 * @return true If the setting has a value
 * @return false If the setting has no value
 */
bool user_settings_has_default_with_key(char *key);

/**
 * @brief Check if a setting has its default value set
 *
 * This will assert if no setting with the provided ID exists.
 * If the ID input for this function is unknown to the application (i.e. parsed from user), then
 * it should first be checked with user_settings_exists_with_id().
 *
 * @param[in] id The ID of the setting to check
 *
 * @return true If the setting has a value
 * @return false If the setting has no value
 */
bool user_settings_has_default_with_id(uint16_t id);

/**
 * @brief Convert user settings key to an id
 *
 * This will assert if no setting with the provided key exists.
 * If the key input for this function is unknown to the application (i.e. parsed from user), then
 * it should first be checked with user_settings_exists_with_key().
 *
 * @param[in] key A valid user setting key
 *
 * @return uint16_t The id of setting with the provided key
 */
uint16_t user_settings_key_to_id(char *key);

/**
 * @brief Convert user settings id to a key
 *
 * This will assert if no setting with the provided id exists.
 * If the ID input for this function is unknown to the application (i.e. parsed from user), then
 * it should first be checked with user_settings_exists_with_id().
 *
 * @param[in] id A valid user setting id
 *
 * @return char* The key of setting with the provided id
 */
char *user_settings_id_to_key(uint16_t id);

/**
 * @brief Get maximal length of the setting
 *
 * This will assert if no setting with the provided key exists.
 * If the key input for this function is unknown to the application (i.e. parsed from user), then
 * it should first be checked with user_settings_exists_with_key().
 *
 * @param[in] key A valid user setting key
 *
 * @return The length of the setting value.
 */
size_t user_settings_get_max_len_with_key(char *key);

/**
 * @brief Get maximal length of the setting
 *
 * This will assert if no setting with the provided id exists.
 * If the ID input for this function is unknown to the application (i.e. parsed from user), then
 * it should first be checked with user_settings_exists_with_id().
 *
 * @param[in] id A valid user setting id
 *
 * @return The length of the setting value.
 */
size_t user_settings_get_max_len_with_id(uint16_t id);

/**
 * @brief Get the type of the setting
 *
 * This will assert if no setting with the provided key exists.
 * If the key input for this function is unknown to the application (i.e. parsed from user), then
 * it should first be checked with user_settings_exists_with_key().
 *
 * @param[in] key A valid user setting key
 *
 * @return The type of the setting value.
 */
enum user_setting_type user_settings_get_type_with_key(char *key);

/**
 * @brief Get the type of the setting
 *
 * This will assert if no setting with the provided id exists.
 * If the ID input for this function is unknown to the application (i.e. parsed from user), then
 * it should first be checked with user_settings_exists_with_id().
 *
 * @param[in] id A valid user setting id
 *
 * @return The type of the setting value.
 */
enum user_setting_type user_settings_get_type_with_id(uint16_t id);

/**
 * @brief Start iteration over all user settings
 *
 * Call this before getting elements with user_settings_list_iter_next.
 * This will reset an in-progress iteration.
 */
void user_settings_iter_start(void);

/**
 * @brief Get next settings ID and KEY in the iteration
 *
 * @param[out] key The key of the next settings.
 * @param[out] id The ID of the next settings.
 *
 * @return True if we have next setting, false if there are no more.
 */
bool user_settings_iter_next(char **key, uint16_t *id);

/**
 * @brief Get next settings ID and KEY in the iteration
 *
 * Only returns settings with a set changed flag.
 *
 * @param[out] key The key of the next changed settings.
 * @param[out] id The ID of the next changed settings.
 *
 * @return True if we have next setting, false if there are no more.
 */
bool user_settings_iter_next_changed(char **key, uint16_t *id);

/**
 * @brief Set the "has_changed_recently" flag
 *
 * This will assert if no setting with the provided key exists.
 * If the key input for this function is unknown to the application (i.e. parsed from user), then
 * it should first be checked with user_settings_exists_with_key().
 *
 * @param[in] key A valid user setting key
 */
void user_settings_set_changed_with_key(char *key);

/**
 * @brief Set the "has_changed_recently" flag
 *
 * This will assert if no setting with the provided id exists.
 * If the ID input for this function is unknown to the application (i.e. parsed from user), then
 * it should first be checked with user_settings_exists_with_id().
 *
 * @param[in] key A valid user setting id
 */
void user_settings_set_changed_with_id(uint16_t id);

/**
 * @brief Clear "has_changed_recently" flag
 *
 * This will assert if no setting with the provided key exists.
 * If the key input for this function is unknown to the application (i.e. parsed from user), then
 * it should first be checked with user_settings_exists_with_key().
 *
 * @param[in] key A valid user setting key
 */
void user_settings_clear_changed_with_key(char *key);

/**
 * @brief Clear "has_changed_recently" flag
 *
 * This will assert if no setting with the provided id exists.
 * If the ID input for this function is unknown to the application (i.e. parsed from user), then
 * it should first be checked with user_settings_exists_with_id().
 *
 * @param[in] id A valid user setting id
 */
void user_settings_clear_changed_with_id(uint16_t id);

/**
 * @brief Clears changed flag on all settings
 */
void user_settings_clear_changed(void);

/**
 * @brief Check if any setting has been changed
 *
 * @retval true At least 1 setting has been changed
 * @retval false No settings have been changed
 */
bool user_settings_any_changed(void);

#ifdef __cplusplus
}
#endif

#endif /* USER_SETTINGS_H */
