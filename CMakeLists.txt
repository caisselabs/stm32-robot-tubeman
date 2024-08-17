#
# Copyright (c) 2024 Michael Caisse
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
cmake_minimum_required(VERSION 3.25)

project(
    stm32-robot-tubeman
    VERSION 0.1.0
    LANGUAGES CXX C ASM
    DESCRIPTION
        "blinky led using senders on an STM32 dev board")

include(cmake/get_cpm.cmake)
cpmaddpackage("gh:intel/cicd-repo-infrastructure#dev")

add_versioned_package("gh:intel/cpp-std-extensions#a5fd19f")
add_versioned_package("gh:intel/cpp-baremetal-concurrency#fef18ca")
add_versioned_package("gh:intel/cpp-baremetal-senders-and-receivers#7bd0c45")
add_versioned_package("gh:intel/generic-register-operation-optimizer#bc3d92a")
add_versioned_package("gh:caisselabs/stm32-groov#f1f39f7")
add_versioned_package("gh:intel/compile-time-init-build#19cadcf")


add_executable(servo_test
	startup/startup_gcc.s
	src/servo_test.cpp
	src/setup.cpp
      )
# the detection used by baremetal concurrency results in determining
# that this target is hosted. Just force with define.
target_compile_definitions(servo_test PUBLIC SIMULATE_FREESTANDING)
target_link_libraries_system(servo_test PUBLIC async stm32-groov)

