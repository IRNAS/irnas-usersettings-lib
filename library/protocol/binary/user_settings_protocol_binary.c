/** @file user_settings_protocol_binary.c
 *
 * @brief Binary protocol for the user settings protocol
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2023 Irnas. All rights reserved.
 */

#include "user_settings_protocol_binary.h"

#include <user_settings_list.h>

#include <zephyr/kernel.h>

#include <string.h>

/**
 * @brief Calculate the required bytes to encode a user setting
 *
 * @param[in] user_setting The user setting to encode
 *
 * @return int The number of bytes required
 */
static int prv_encode_required_bytes(struct user_setting *user_setting)
{
	/* check if we can we fit setting into buffer.
	 * We need:
	 * - 2 byte ID
	 * - strlen for key
	 * - 1 byte NULL terminator
	 * - 1 byte type
	 * - 1 byte length (if 0 no value is set)
	 * - length bytes value
	 */

	/* calculate base up to length */
	int base = 2 + strlen(user_setting->key) + 1 + 1 + 1;

	if (user_setting->is_set) {
		base += user_setting->data_len;
	}
	return base;
}

/**
 * @brief Calculate the required bytes to full encode a user setting
 *
 * @param[in] user_setting The user setting to encode
 *
 * @return int The number of bytes required
 */
static int prv_encode_required_bytes_full(struct user_setting *user_setting)
{
	/* Start with short format and add 1 for default len and 1 for max len */
	int base = prv_encode_required_bytes(user_setting) + 1 + 1;

	if (user_setting->default_is_set) {
		base += user_setting->default_data_len;
	}
	return base;
}

int user_settings_protocol_binary_decode_command(uint8_t *buffer, size_t len,
						 struct user_settings_protocol_command *command)
{
	__ASSERT(buffer, "buffer must be provided");
	__ASSERT(command, "command must be provided");

	memset(command, 0, sizeof(struct user_settings_protocol_command));

	if (len == 0) {
		return -EPROTO;
	}

	/* first byte is type */
	int i = 0;
	command->type = buffer[i++];

	/* If command is get or set, key must be provided */
	switch (command->type) {
	case USPC_LIST:
	case USPC_LIST_FULL:
	case USPC_RESTORE: {
		/* No additional fields  */
		return i;
	}
	case USPC_GET:
	case USPC_GET_FULL: {
		/* Key only */
		if (len != sizeof(command->type) + sizeof(command->id)) {
			return -EPROTO;
		}
		command->id = *(uint16_t *)&buffer[i];
		i += 2;
		return i;
	}
	case USPC_SET:
	case USPC_SET_DEFAULT: {
		/* key and data (the + 1 is there since at least 1 byte of data is required) */
		if (len <
		    sizeof(command->type) + sizeof(command->id) + sizeof(command->value_len) + 1) {
			return -EPROTO;
		}
		command->id = *(uint16_t *)&buffer[i];
		i += 2;
		command->value_len = *(uint8_t *)&buffer[i++];

		memcpy(command->value, &buffer[i], command->value_len);
		i += command->value_len;

		return i;
	}
	case USPC_LIST_SOME:
	case USPC_LIST_SOME_FULL: {
		/* key, 1 byte for number of setting IDs and N*2 bytes for the IDs */
		size_t id_buffer_len = len - sizeof(command->type) - 1;
		uint8_t num_ids = buffer[i++];
		if (id_buffer_len / 2 != num_ids) {
			return -EPROTO;
		}
		command->value_len = num_ids * 2;
		memcpy(command->value, &buffer[sizeof(command->type) + 1], id_buffer_len);
		i += command->value_len;
		return i;
	}
	default:
		/* Unknown command */
		return -ENOTSUP;
	}
}

int user_settings_protocol_binary_encode(struct user_setting *user_setting, uint8_t *buffer,
					 size_t len)
{
	__ASSERT(user_setting, "Valid user setting must be provided");
	__ASSERT(buffer, "buffer must be provided");

	if (len < prv_encode_required_bytes(user_setting)) {
		return -ENOMEM;
	}

	int i = 0;

	/* ID */
	buffer[i] = user_setting->id;
	i += 2;

	/* key with null terminator */
	int key_len = strlen(user_setting->key) + 1;
	memcpy(&buffer[i], user_setting->key, key_len);
	i += key_len;

	/* type */
	buffer[i++] = user_setting->type;

	if (user_setting->is_set) {
		/* length */
		buffer[i++] = user_setting->data_len;
		/* value */
		memcpy(&buffer[i], user_setting->data, user_setting->data_len);
		i += user_setting->data_len;
	} else {
		/* set length to 0 */
		buffer[i++] = 0;
	}

	return i;
}

int user_settings_protocol_binary_encode_full(struct user_setting *user_setting, uint8_t *buffer,
					      size_t len)
{
	__ASSERT(user_setting, "Valid user setting must be provided");
	__ASSERT(buffer, "buffer must be provided");

	/* Use above encode and add:
	 * 1 byte default length (if 0 no default value is set)
	 * length bytes default value
	 * 1 byte max_len
	 */

	if (len < prv_encode_required_bytes_full(user_setting)) {
		return -ENOMEM;
	}

	int i = user_settings_protocol_binary_encode(user_setting, buffer, len);

	if (user_setting->default_is_set) {
		/* length */
		buffer[i++] = user_setting->default_data_len;
		/* value */
		memcpy(&buffer[i], user_setting->default_data, user_setting->default_data_len);
		i += user_setting->default_data_len;
	} else {
		/* set length to 0 */
		buffer[i++] = 0;
	}

	/* max length */
	buffer[i++] = user_setting->max_size;

	return i;
}
