zephyr_library_include_directories(.)
zephyr_library_sources(${CMAKE_CURRENT_SOURCE_DIR}/user_settings_list.c)
zephyr_library_sources(${CMAKE_CURRENT_SOURCE_DIR}/user_settings.c)
zephyr_library_sources_ifdef(CONFIG_USER_SETTINGS_SHELL
                             ${CMAKE_CURRENT_SOURCE_DIR}/user_settings_shell.c)
zephyr_library_sources_ifdef(CONFIG_USER_SETTINGS_JSON
                             ${CMAKE_CURRENT_SOURCE_DIR}/user_settings_json.c)
