# COPYRIGHT NOTICE: (c) 2022 Irnas. All rights reserved.

cmake_minimum_required(VERSION 3.20)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(user-settings-sample-validation)

zephyr_include_directories(src)
target_sources(app PRIVATE src/main.c)
