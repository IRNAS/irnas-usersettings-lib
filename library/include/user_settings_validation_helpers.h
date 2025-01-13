/** @file user_settings_validation_helpers.h
 *
 * @brief Macros for defining validation functions for user settings
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2025 Irnas. All rights reserved.
 */

#ifndef USER_SETTINGS_VALIDATION_HELPERS_H
#define USER_SETTINGS_VALIDATION_HELPERS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Define a validation callback for a setting that checks if the value is in a range.
 *
 * Useful for integer types.
 *
 * @param[in] name The name of the validation function.
 * @param[in] type The type of the setting value.
 * @param[in] min The minimum value the setting can have (inclusive).
 * @param[in] max The maximum value the setting can have (inclusive).
 */
#define USER_SETTINGS_VALIDATE_CB_RANGE(name, type, min, max)                                      \
	static bool name(uint32_t id, const char *key, void *data, size_t len)                     \
	{                                                                                          \
		type val = *(type *)data;                                                          \
		const type _min = min;                                                             \
		const type _max = max;                                                             \
		if (val < _min || val > _max) {                                                    \
			LOG_ERR("Value must be in range [%d, %d]", _min, _max);                    \
			return false;                                                              \
		}                                                                                  \
		return true;                                                                       \
	}

/**
 * @brief Define a validation callback for a setting that checks the exact size of the value.
 *
 * Useful for string and bytes types.
 * For string types, the length should include the NULL terminator.
 */
#define USER_SETTINGS_VALIDATE_CB_EXACT_SIZE(name, size)                                           \
	static bool name(uint32_t id, const char *key, void *data, size_t len)                     \
	{                                                                                          \
		if (len != (size)) {                                                               \
			LOG_ERR("Value must be exactly %d bytes long", (size));                    \
			return false;                                                              \
		}                                                                                  \
		return true;                                                                       \
	}

#ifdef __cplusplus
}
#endif

#endif /* USER_SETTINGS_VALIDATION_HELPERS_H */
