/** @file user_settings_protocol_types.h
 *
 * @brief Types for the user settings protocol
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2023 Irnas.  All rights reserved.
 */

#ifndef USER_SETTINGS_PROTOCOL_TYPES_H
#define USER_SETTINGS_PROTOCOL_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/types.h>

/**
 * @brief Command types
 */
enum user_settings_protocol_command_type {
	/**  Get id, key, type, length, value (id must be provided). */
	USPC_GET = 1,

	/**  Get id, key, type, length, value, max length, default value (id must be provided). */
	USPC_GET_FULL = 2,

	/**  Get id, key, name, type, length and value for each setting. */
	USPC_LIST = 3,

	/**  Get id, key, name, type, length, max length value, default value for each setting. */
	USPC_LIST_FULL = 4,

	/** Set value (id must be provided). */
	USPC_SET = 5,

	/** Set default value (id must be provided). */
	USPC_SET_DEFAULT = 6,

	/** Restore default values. */
	USPC_RESTORE = 7,

	/* Get id, key, name, type, length and value for a list of settings. */
	USPC_LIST_SOME = 8,

	/* Get id, key, name, type, length, max length value, default value for a list of settings.
	 */
	USPC_LIST_SOME_FULL = 9,

	/** Internal use only. */
	USPC_NUM_COMMANDS,

} __attribute__((packed));

/**
 * @brief Decoded command
 *
 * A raw buffer is some encoding format should be decoded into this struct
 */
struct user_settings_protocol_command {
	/** The type of the command. */
	enum user_settings_protocol_command_type type;

	/** Setting ID. Might not be set (based on chosen command). */
	uint16_t id;

	/** if set, number of bytes in value. */
	uint8_t value_len;

	/** if value_len > 0, the decoded value.
	 * Note that this is always safe since the maximum length of a setting is 255 bytes.
	 */
	uint8_t value[256];

} __attribute__((packed));

/* Forward declaration of an internal user setting representation. This is required to wire the
 * callbacks of the executor to a specific protocol implementation. */
struct user_setting;

#ifdef __cplusplus
}
#endif

#endif /* USER_SETTINGS_PROTOCOL_TYPES_H */
