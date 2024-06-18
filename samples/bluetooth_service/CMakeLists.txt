# COPYRIGHT NOTICE: (c) 2022 Irnas. All rights reserved.

cmake_minimum_required(VERSION 3.20)

# create compile_commands.json for clang
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(user-settings-sample-basic)

zephyr_compile_options(-fdiagnostics-color=always)

zephyr_include_directories(src)
target_sources(app PRIVATE src/main.c)
