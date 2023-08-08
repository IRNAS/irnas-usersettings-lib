/** @file user_settings_protocol_executor.h
 *
 * @brief User settings protocol executor
 *
 * The protocol executor is a generic "service" to decode and execute commands and also generate
 * responses to those commands.
 * The encoding protocol used is selected by selecting decoding/encoding functions in struct
 * usp_executor.
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2023 Irnas.  All rights reserved.
 */

#ifndef USER_SETTINGS_PROTOCOL_EXECUTOR_H
#define USER_SETTINGS_PROTOCOL_EXECUTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/types.h>

#include <user_settings_protocol_types.h>

typedef int (*uspe_decode_command_t)(uint8_t *buffer, size_t len,
				     struct user_settings_protocol_command *command);

typedef int (*uspe_encode_t)(struct user_setting *user_setting, uint8_t *buffer, size_t len);

typedef int (*uspe_write_response_t)(uint8_t *buffer, size_t len, void *user_data);
/**
 * @brief The protocol executor
 *
 * The decoding and encoding functions should come from the same protocol implementation.
 */
struct usp_executor {

	/**
	 * @brief Decode raw buffer in some format to a user settings protocol command
	 *
	 * @param[in] buffer The buffer to decode
	 * @param[in] len The length of the buffer
	 * @param[out] command The decoded command
	 *
	 * @retval 0 on success
	 * @retval -EPROTO if decoding failed
	 * @retval -ENOTSUP if the command is not supported in this protocol format
	 */
	uspe_decode_command_t decode_command;

	/**
	 * @brief Encode a user setting into some format in the short form
	 *
	 * What exactly the short form is is defined by the format in use.
	 *
	 * @param[in] user_setting The setting to encode
	 * @param[out] buffer The buffer to encode into
	 * @param[in] len The length of the buffer
	 *
	 * @return The number of bytes written or -ENOMEM if the provided buffer is to small
	 */
	uspe_encode_t encode;

	/**
	 * @brief Encode a user setting into some format in the full form
	 *
	 * What exactly the full form is is defined by the format in use.
	 *
	 * @param[in] user_setting The setting to encode
	 * @param[out] buffer The buffer to encode into
	 * @param[in] len The length of the buffer
	 *
	 * @return The number of bytes written or -ENOMEM if the provided buffer is to small
	 */
	uspe_encode_t encode_full;

	/**
	 * @brief Write a response from the executor into the protocol transport
	 *
	 * The buffer passed here is the one provided via @p resp_buffer.
	 * If this function returns a non-zero code, the executor will immediately stop executing
	 * the current command and return -EIO.
	 *
	 * This function must immediately send the buffer or create a copy. The executor might call
	 * this function multiple times while processing a single command (i. e. LIST).
	 *
	 * @param[in] buffer The buffer to write
	 * @param[in] len The number of bytes from the buffer to write
	 *
	 * @return 0 on success or negative error code otherwise
	 */
	uspe_write_response_t write_response;

	/**
	 * @brief A buffer to store generated responses in
	 */
	uint8_t *resp_buffer;

	/** @brief
	 * The length of the response buffer
	 */
	size_t resp_buffer_len;
};

/**
 * @brief Parse and execute a user settings protocol command
 *
 * @param[in] usp_executor The executor to use
 * @param[in] buffer The raw buffer to parse and execute
 * @param[in] len The length of the buffer
 *
 * @retval 0 on success
 * @retval -EPROTO if decoding failed
 * @retval -ENOTSUP if the command is not supported in this protocol format
 * @retval -ENOENT if the decoded command specifies a setting ID that does not exists
 * @retval -ENOMEM if the provided resp_buffer is to small to fit the encoded response
 * @retval -EIO if writing the response failed (see write_response above for details)
 * @retval -ENOEXEC if the operation on user settings failed (i.e. setting a new value)
 */
int usp_executor_parse_and_execute(struct usp_executor *usp_executor, uint8_t *buffer, size_t len,
				   void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* USER_SETTINGS_PROTOCOL_EXECUTOR_H */
