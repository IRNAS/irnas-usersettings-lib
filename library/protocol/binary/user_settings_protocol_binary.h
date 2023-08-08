/** @file user_settings_protocol_binary.h
 *
 * @brief Binary protocol for the user settings protocol
 *
 * The decode/encode functions satisfy the API in user_settings_protocol_executor.h.
 *
 * The protocol is described in more details in the README.md in this folder.
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2023 Irnas.  All rights reserved.
 */

#ifndef USER_SETTINGS_PROTOCOL_BINARY_H
#define USER_SETTINGS_PROTOCOL_BINARY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <user_settings_protocol_types.h>

/**
 * @brief Decode command in binary format to a command in user settings protocol structure
 * representation
 *
 * The binary command format is defined as follows:
 * - 1 byte	command type (from enum user_settings_protocol_command_type)
 * - 2 byte	setting key
 * - 1 byte	value len key
 * - len bytes	value
 *
 * USPC_LIST, USPC_LIST_FULL, USPC_RESTORE must only provide the first byte
 * USPC_GET, USPC_GET_FULL must provide the first byte and the setting key
 * USPC_SET, USPC_SET_DEFAULT must provide the first byte, the setting key, the length and the value
 *
 * @param[in] buffer The buffer to decode
 * @param[in] len The length of the buffer
 * @param[out] command The decoded command
 *
 * @retval 0 on success
 * @retval -EPROTO if decoding failed
 * @retval -ENOTSUP if the command is not supported
 */
int user_settings_protocol_binary_decode_command(uint8_t *buffer, size_t len,
						 struct user_settings_protocol_command *command);

/**
 * @brief Encode a user setting into its binary format
 *
 * The binary format is defined as follows (all numbers are little endian):
 * - 2 byte 	ID
 * - N bytes 	key (NULL terminated string)
 * - 1 byte 	type (from enum user_setting_type)
 * - 1 byte	length of the value (LEN) or 0 if the value is not set
 * - LEN bytes 	value (or nothing if LEN is 0)
 *
 * @param[in] user_setting The setting to encode
 * @param[out] buffer The buffer to encode into
 * @param[in] len The length of the buffer
 *
 * @return The number of bytes written or -ENOMEM if the provided buffer is to small
 */
int user_settings_protocol_binary_encode(struct user_setting *user_setting, uint8_t *buffer,
					 size_t len);

/**
 * @brief Encode a user setting into its binary FULL format
 *
 * The full binary format is the same as the short format with the following additional fields added
 * to the end:
 * - 1 byte 		default length (DEFAULT_LEN) or 0 if the default value is not set
 * - DEFAULT_LEN bytes	default value (or nothing if DEFAULT_LEN is 0)
 * - 1 byte 		maximum length of the value
 *
 * @param[in] user_setting The setting to encode
 * @param[out] buffer The buffer to encode into
 * @param[in] len The length of the buffer
 *
 * @return The number of bytes written or -ENOMEM if the provided buffer is to small
 */
int user_settings_protocol_binary_encode_full(struct user_setting *user_setting, uint8_t *buffer,
					      size_t len);

/**
 * @brief Define a protocol executor using the binary protocol
 *
 * @param[in] buffer The resp_buffer of the executor used to put responses in
 * @param[in] len The length of the resp_buffer
 * @param[in] write_response_fn The function used to write responses from the executor
 *
 */
#define USP_BINARY_EXECUTOR_DECLARE(buffer, len, write_response_fn)                                \
	{                                                                                          \
		.decode_command = user_settings_protocol_binary_decode_command,                    \
		.encode = user_settings_protocol_binary_encode,                                    \
		.encode_full = user_settings_protocol_binary_encode_full, .resp_buffer = buffer,   \
		.resp_buffer_len = len, .write_response = write_response_fn,                       \
	}

#ifdef __cplusplus
}
#endif

#endif /* USER_SETTINGS_PROTOCOL_BINARY_H */
