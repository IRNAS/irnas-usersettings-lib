# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

# create compile_commands.json for clang
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(test_user_settings)

# Set CMake path variables for convenience
set(LIB_DIR ../../library)

file(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})

# add fancy_z_test
add_subdirectory(../common common)

# add test target
target_include_directories(app PRIVATE ${LIB_DIR}/user_settings)

target_include_directories(app PRIVATE ${LIB_DIR}/protocol/binary)
target_include_directories(app PRIVATE ${LIB_DIR}/protocol)
target_sources(
  app PRIVATE ${LIB_DIR}/protocol/binary/user_settings_protocol_binary.c)
