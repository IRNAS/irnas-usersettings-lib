/** @file user_settings_protocol_executor.c
 *
 * @brief User settings protocol executor
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2023 Irnas. All rights reserved.
 */

#include "user_settings_protocol_executor.h"

#include <user_settings.h>
#include <user_settings_list.h>

/**
 * @brief Execute a GET command
 *
 * Fetch the specified setting, encode it using the provided @p encode function and write it as a
 * response.
 *
 * @param[in] usp_executor The executor
 * @param[in] id The setting ID
 * @param[in] encode the encode function to use
 * @param[in] user_data The user data to pass to the write_response function
 *
 * @retval 0 on success
 * @retval -ENOENT setting ID does not exists
 * @retval -ENOMEM if the resp_buffer is to small to fit the encoded response
 * @retval -EIO if writing the response failed
 */
static int prv_exec_get_common(struct usp_executor *usp_executor, uint16_t id, uspe_encode_t encode,
			       void *user_data)
{
	struct user_setting *us = user_settings_list_get_by_id(id);
	if (!us) {
		/* Setting with this ID not found */
		return -ENOENT;
	}

	/* encode setting */
	int ret = encode(us, usp_executor->resp_buffer, usp_executor->resp_buffer_len);
	if (ret < 0) {
		__ASSERT(ret == -ENOMEM, "The encode function must only return the -ENOMEM error");
		return ret;
	}

	/* Write encoded setting */
	ret = usp_executor->write_response(usp_executor->resp_buffer, ret, user_data);
	if (ret < 0) {
		return -EIO;
	}
	return 0;
}

static int prv_exec_get(struct usp_executor *usp_executor, uint16_t id, void *user_data)
{
	return prv_exec_get_common(usp_executor, id, usp_executor->encode, user_data);
}

static int prv_exec_get_full(struct usp_executor *usp_executor, uint16_t id, void *user_data)
{
	return prv_exec_get_common(usp_executor, id, usp_executor->encode_full, user_data);
}

/**
 * @brief Execute a LIST command
 *
 * Iterate over all settings, encode them and write each one as a response.
 *
 * @param[in] usp_executor The executor
 * @param[in] encode The encode function to use
 * @param[in] user_data The user data to pass to the write_response function
 *
 * @retval 0 on success
 * @retval -ENOMEM if the resp_buffer is to small to fit the encoded response
 * @retval -EIO if writing the response failed
 */
static int prv_exec_list_common(struct usp_executor *usp_executor, uspe_encode_t encode,
				void *user_data)
{
	/* encode and write each setting */
	int ret;
	struct user_setting *us;

	struct user_settings_iter_ctx ctx;
	user_settings_list_iter_start(&ctx);
	while ((us = user_settings_list_iter_next(&ctx)) != NULL) {
		ret = encode(us, usp_executor->resp_buffer, usp_executor->resp_buffer_len);
		if (ret < 0) {
			__ASSERT(ret == -ENOMEM,
				 "The encode function must only return the -ENOMEM error");
			return ret;
		}
		ret = usp_executor->write_response(usp_executor->resp_buffer, ret, user_data);
		if (ret < 0) {
			return -EIO;
		}
	}
	return 0;
}

static int prv_exec_list(struct usp_executor *usp_executor, void *user_data)
{
	return prv_exec_list_common(usp_executor, usp_executor->encode, user_data);
}

static int prv_exec_list_full(struct usp_executor *usp_executor, void *user_data)
{
	return prv_exec_list_common(usp_executor, usp_executor->encode_full, user_data);
}

/**
 * @brief Set a setting value
 *
 * @param[in] id The setting ID
 * @param[in] value The value to set
 * @param[in] value_len The length of the value in bytes
 *
 * @retval 0 on success
 * @retval -ENOENT if the setting ID does not exists
 * @retval -ENOEXEC if setting the new value failed
 */
static int prv_exec_set(uint16_t id, uint8_t *value, uint8_t value_len)
{
	if (!user_settings_exists_with_id(id)) {
		return -ENOENT;
	}
	int ret = user_settings_set_with_id(id, value, value_len);
	if (ret < 0) {
		return -ENOEXEC;
	}
	return 0;
}

/**
 * @brief Set a setting default value
 *
 * @param[in] id The setting ID
 * @param[in] value The default value to set
 * @param[in] value_len The length of the default value in bytes
 *
 * @retval 0 on success
 * @retval -ENOENT if the setting ID does not exists
 * @retval -ENOEXEC if setting the new default value failed
 */
static int prv_exec_set_default(uint16_t id, uint8_t *value, uint8_t value_len)
{
	if (!user_settings_exists_with_id(id)) {
		return -ENOENT;
	}
	int ret = user_settings_set_default_with_id(id, value, value_len);
	if (ret < 0) {
		return -ENOEXEC;
	}
	return 0;
}

/**
 * @brief Restore user settings to their default values
 *
 * @retval 0 (always)
 */
static int prv_exec_restore(void)
{
	user_settings_restore_defaults();
	return 0;
}

/**
 * @brief Execute a LIST_SOME command
 *
 * Iterate over all setting ID's provided, encode them and write each one as a response.
 *
 * @param[in] usp_executor The executor
 * @param[in] num_ids The number of setting ID's provided
 * @param[in] ids The setting ID's provided
 * @param[in] user_data The user data to pass to the write_response function
 *
 * @retval 0 on success
 * @retval -ENOENT setting ID does not exists
 * @retval -ENOMEM if the resp_buffer is to small to fit the encoded response
 * @retval -EIO if writing the response failed
 */
static int prv_exec_list_some(struct usp_executor *usp_executor, uint8_t num_ids, uint16_t *ids,
			      void *user_data)
{
	for (int i = 0; i < num_ids; i++) {
		int ret =
			prv_exec_get_common(usp_executor, ids[i], usp_executor->encode, user_data);
		if (ret < 0) {
			return ret;
		}
	}
	return 0;
}

/**
 * @brief Execute a LIST_SOME_FULL command
 *
 * Iterate over all setting ID's provided, encode them and write each one as a response.
 *
 * @param[in] usp_executor The executor
 * @param[in] num_ids The number of setting ID's provided
 * @param[in] ids The setting ID's provided
 * @param[in] user_data The user data to pass to the write_response function
 *
 * @retval 0 on success
 * @retval -ENOENT setting ID does not exists
 * @retval -ENOMEM if the resp_buffer is to small to fit the encoded response
 * @retval -EIO if writing the response failed
 */
static int prv_exec_list_some_full(struct usp_executor *usp_executor, uint8_t num_ids,
				   uint16_t *ids, void *user_data)
{
	for (int i = 0; i < num_ids; i++) {
		int ret = prv_exec_get_common(usp_executor, ids[i], usp_executor->encode_full,
					      user_data);
		if (ret < 0) {
			return ret;
		}
	}
	return 0;
}

/* will this return the number of bytes parsed? negative error code otherwise? This way you can have
 * a buffer holding multiple commands in a row and this will always parse 1 command ant tell the
 * user where it finished
 *
 * TODO: Implement the idea in the above comment */
int usp_executor_parse_and_execute(struct usp_executor *usp_executor, uint8_t *buffer, size_t len,
				   void *user_data)
{
	int ret;
	struct user_settings_protocol_command cmd = {0};

	/* decode command first */
	ret = usp_executor->decode_command(buffer, len, &cmd);
	if (ret < 0) {
		return ret;
	}

	switch (cmd.type) {
	case USPC_GET: {
		return prv_exec_get(usp_executor, cmd.id, user_data);
	}
	case USPC_GET_FULL: {
		return prv_exec_get_full(usp_executor, cmd.id, user_data);
	}
	case USPC_LIST: {
		return prv_exec_list(usp_executor, user_data);
	}
	case USPC_LIST_FULL: {
		return prv_exec_list_full(usp_executor, user_data);
	}
	case USPC_SET: {
		return prv_exec_set(cmd.id, cmd.value, cmd.value_len);
	}
	case USPC_SET_DEFAULT: {
		return prv_exec_set_default(cmd.id, cmd.value, cmd.value_len);
	}
	case USPC_RESTORE: {
		return prv_exec_restore();
	}
	case USPC_LIST_SOME: {
		return prv_exec_list_some(usp_executor, cmd.value_len / 2, (uint16_t *)cmd.value,
					  user_data);
	}
	case USPC_LIST_SOME_FULL: {
		return prv_exec_list_some_full(usp_executor, cmd.value_len / 2,
					       (uint16_t *)cmd.value, user_data);
	}

	default: {
		/* We should not end up here. If the decoder does not support a command type, it
		 * should return an error above */
		__ASSERT(0, "How did we end up here?");
		return -ENOTSUP;
	}
	}
}
