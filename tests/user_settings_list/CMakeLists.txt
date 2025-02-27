# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(test_user_settings_list)

# Set CMake path variables for
set(LIB_DIR ../../library)

file(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})

# add fancy_z_test
add_subdirectory(../common common)

# add module under test
target_sources(app PRIVATE ${LIB_DIR}//user_settings/user_settings_list.c)

# add include directories from application
target_include_directories(app PRIVATE ${LIB_DIR}/user_settings)
target_include_directories(app PRIVATE ${LIB_DIR}/include)
