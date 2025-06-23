/** @file user_settings_list.h
 *
 * @brief List of IoT user settings
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2022 Irnas.  All rights reserved.
 */

#ifndef USER_SETTINGS_LIST_H
#define USER_SETTINGS_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/kernel.h>
#include <user_settings_types.h>

/**
 * @brief Internal representation of a user_setting.
 *
 * This holds all information of a user setting and pointers to the data in the private heap
 *
 */
struct user_setting {

	/** Used for storing settings items in a linked list */
	sys_snode_t list_node;

	/** The ID of the setting.
	 *
	 * This is a simple enumeration and can be used instead of the key
	 * if space/storage is your concern. The storage backend always uses the string key when
	 * storing and loading settings. */
	uint16_t id;

	/** Identifier of the setting. The length of the key must not be greater
	 * then CONFIG_USER_SETTINGS_MAX_KEY_LEN. */
	char *key;

	/** Its type */
	enum user_setting_type type;

	/** Maximum size in bytes. This is fixed for the numeric types and user
	 * specified for the string and bytes type. This should never be decreased in consecutive
	 * firmware releases. */
	size_t max_size;

	/** Space for the setting is dynamically allocated during initialization
	 * and the pointer is stored here */
	void *data;

	/** The length (in bytes) of the data in use. This is always <= max_size */
	size_t data_len;

	/* Is true if the setting was set/loaded.
	 * Is false if no value for this setting is available */
	bool is_set;

	/** Space for the setting default value is dynamically allocated during initialization
	 * and the pointer is stored here. */
	void *default_data;

	/** The length (in bytes) of the default data. This is always <= max_size */
	size_t default_data_len;

	/* This is set to true if a default value for this setting has been provided. */
	bool default_is_set;

	/* This is set to true when setting data is changed. It is reset by calling ...TODO*/
	bool has_changed_recently;

	/** On change callback for this specific setting. Can be NULL. This will be called
	 * by the settings module when this setting is updated. */
	user_settings_on_change_t on_change_cb;

	/** Validate callback for this specific setting. Can be NULL. This will be called
	 * by the settings module before this setting is updated. If false is returned, the setting
	 * will not be changed. */
	user_settings_validate_t validate_cb;
};

/**
 * @brief Initialize the user settings list
 *
 * This should be called prior to adding any items to the list with
 * user_settings_list_add_new().
 *
 */
void user_settings_list_init(void);

/**
 * @brief Add a new user_setting to the list with known size
 *
 * This will allocate the required number of bytes of space for the data and default_data
 * pointers in the returned struct user_setting.
 * The size required is inferred from the type.
 *
 * This will assert if
 *
 * @note This will assert if:
 *  - the type is string or bytes, since those are variable sized
 *  - a setting with the same ID is already in the list
 *  - a setting with the same key is already in the list
 *
 * @param[in] id The ID of the setting to add
 * @param[in] key The key of the setting to add
 * @param[in] type The type of the setting
 * @param[in] size The size of the setting (in bytes)
 *
 * @return struct user_setting* The newly created setting
 */
struct user_setting *user_settings_list_add_fixed_size(uint16_t id, const char *key,
						       enum user_setting_type type);

/**
 * @brief Add a new user_setting to the list with known size
 *
 * This will allocate @p size bytes of space for the data and default_data
 * pointers in the returned struct user_setting.
 *
 * @note This will assert if:
 *  - if the type is not string or bytes
 *  - a setting with the same ID is already in the list
 *  - a setting with the same key is already in the list
 *
 * @param[in] id The ID of the setting to add
 * @param[in] key The key of the setting to add
 * @param[in] type The type of the setting
 * @param[in] size The size of the setting (in bytes)
 *
 * @return struct user_setting* The newly created setting
 */
struct user_setting *user_settings_list_add_variable_size(uint16_t id, const char *key,
							  enum user_setting_type type, size_t size);

/**
 * @brief Free all items in the list
 *
 * This also frees the data and default data allocated memory
 */
void user_settings_list_free(void);

/**
 * @brief Start iteration over the list
 */
void user_settings_list_iter_start(struct user_settings_iter_ctx *ctx);

/**
 * @brief Get the next item in the list
 *
 * Will return NULL after all items in the list have been returned
 *
 * @return struct user_setting* The next item in the list
 */
struct user_setting *user_settings_list_iter_next(struct user_settings_iter_ctx *ctx);

/**
 * @brief Get item in list by key
 *
 * Will return NULL if item with this key does not exists
 *
 * @param[in] key The key to search for
 *
 * @return struct user_setting* The item found. NULL if item with this key does not exists
 */
struct user_setting *user_settings_list_get_by_key(const char *key);

/**
 * @brief Get item in list by ID
 *
 * Will return NULL if item with this ID does not exists
 *
 * @param[in] id The ID to search for
 *
 * @return struct user_setting* The item found. NULL if item with this ID does not exists
 */
struct user_setting *user_settings_list_get_by_id(const uint16_t id);

#ifdef __cplusplus
}
#endif

#endif /* USER_SETTINGS_LIST_H */
