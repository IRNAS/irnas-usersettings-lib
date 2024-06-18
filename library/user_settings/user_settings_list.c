/** @file user_settings.c
 *
 * @brief Module for handling all IoT user settings
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2022 Irnas. All rights reserved.
 */

#include "user_settings_list.h"

#include <stdio.h>
#include <string.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(user_settings_list, CONFIG_USER_SETTINGS_LOG_LEVEL);

K_HEAP_DEFINE(prv_heap, CONFIG_USER_SETTINGS_HEAP_SIZE);

static sys_slist_t prv_user_settings_list;

void user_settings_list_init(void)
{
	sys_slist_init(&prv_user_settings_list);
}

/**
 * @brief Returns the size of a settings type
 *
 * Only works for types with known fixed sizes. Will assert otherwise
 *
 * @param[in] type The type to get the size of
 * @return int The size of the type (in bytes)
 */
static int prv_type_to_size(enum user_setting_type type)
{
	switch (type) {
	case USER_SETTINGS_TYPE_BOOL:
	case USER_SETTINGS_TYPE_U8:
	case USER_SETTINGS_TYPE_I8:
		return 1;
	case USER_SETTINGS_TYPE_U16:
	case USER_SETTINGS_TYPE_I16:
		return 2;
	case USER_SETTINGS_TYPE_U32:
	case USER_SETTINGS_TYPE_I32:
		return 4;
	case USER_SETTINGS_TYPE_U64:
	case USER_SETTINGS_TYPE_I64:
		return 8;
	case USER_SETTINGS_TYPE_STR:
	case USER_SETTINGS_TYPE_BYTES:
		__ASSERT(false,
			 "String and bytes type should not be used when calling this function");
	}

	return 0;
}

struct user_setting *prv_user_settings_list_add(uint16_t id, const char *key,
						enum user_setting_type type, size_t size)
{
	void *mem;

	/* assert things about the new setting */
	__ASSERT(user_settings_list_get_by_id(id) == NULL,
		 "Setting with this ID already exists: %d", id);
	__ASSERT(user_settings_list_get_by_key(key) == NULL,
		 "Setting with this KEY already exists: %s", key);

	/* allocate space for user_setting */
	mem = k_heap_aligned_alloc(&prv_heap, 8, sizeof(struct user_setting), K_NO_WAIT);
	__ASSERT(mem,
		 "Unable to allocate %d bytes for new struct user_setting (key: %s). Consider "
		 "Increasing CONFIG_USER_SETTINGS_HEAP_SIZE",
		 sizeof(struct user_setting), key);

	/* initialize up all user_setting values */
	struct user_setting *us = mem;
	memset(us, 0, sizeof(struct user_setting));
	us->id = id;
	us->key = (char *)key;
	us->type = type;
	us->max_size = size;
	us->is_set = false;
	us->data_len = 0;
	us->default_data_len = 0;
	us->default_is_set = 0;
	us->has_changed_recently = 0;
	us->on_change_cb = NULL;

	/* allocate space for setting value */
	mem = k_heap_aligned_alloc(&prv_heap, 8, size, K_NO_WAIT);
	__ASSERT(mem,
		 "Unable to allocate %d bytes for %s setting value. Consider "
		 "Increasing CONFIG_USER_SETTINGS_HEAP_SIZE",
		 size, key);
	memset(mem, 0, size);
	us->data = mem;

	/* allocate space for setting default value */
	mem = k_heap_aligned_alloc(&prv_heap, 8, size, K_NO_WAIT);
	__ASSERT(mem,
		 "Unable to allocate %d bytes for %s setting default value. Consider "
		 "Increasing CONFIG_USER_SETTINGS_HEAP_SIZE",
		 size, key);
	memset(mem, 0, size);

	us->default_data = mem;

	/* add new struct to linked list */
	sys_slist_append(&prv_user_settings_list, &us->list_node);

	return us;
}

struct user_setting *user_settings_list_add_fixed_size(uint16_t id, const char *key,
						       enum user_setting_type type)
{
	return prv_user_settings_list_add(id, key, type, prv_type_to_size(type));
}

struct user_setting *user_settings_list_add_variable_size(uint16_t id, const char *key,
							  enum user_setting_type type, size_t size)
{
	__ASSERT(type == USER_SETTINGS_TYPE_STR || type == USER_SETTINGS_TYPE_BYTES,
		 "This function only supports string and bytes types");

	return prv_user_settings_list_add(id, key, type, size);
}

struct user_setting *user_settings_list_get_by_key(const char *key)
{
	struct user_setting *us;
	SYS_SLIST_FOR_EACH_CONTAINER(&prv_user_settings_list, us, list_node) {
		if (strcmp(key, us->key) == 0) {
			return us;
		}
	}
	return NULL;
}

struct user_setting *user_settings_list_get_by_id(const uint16_t id)
{
	struct user_setting *us;
	SYS_SLIST_FOR_EACH_CONTAINER(&prv_user_settings_list, us, list_node) {
		if (id == us->id) {
			return us;
		}
	}
	return NULL;
}

static sys_snode_t *prv_iter_list_node = NULL;
static bool iter_start = false;

void user_settings_list_iter_start(void)
{
	iter_start = true;
	prv_iter_list_node = NULL;
}

struct user_setting *user_settings_list_iter_next(void)
{
	if (prv_iter_list_node == NULL && iter_start) {
		prv_iter_list_node = sys_slist_peek_head(&prv_user_settings_list);
		iter_start = false;
	} else {
		prv_iter_list_node = sys_slist_peek_next(prv_iter_list_node);
	}

	struct user_setting *us = NULL;
	return SYS_SLIST_CONTAINER(prv_iter_list_node, us, list_node);
}

void user_settings_list_free(void)
{
	/* free data, default_data, setting struct, remove from list */

	struct user_setting *us;
	SYS_SLIST_FOR_EACH_CONTAINER(&prv_user_settings_list, us, list_node) {
		k_heap_free(&prv_heap, us->data);
		k_heap_free(&prv_heap, us->default_data);
		k_heap_free(&prv_heap, us);
	}

	user_settings_list_init();
}
