# This is a default makefile for the Zephyr project, that is supposed to be
# initialized with West and built with East.
#
# This makefile in combination with the Github actions does the following:
# * Installs python dependencies and toolchain
# * Initializes the project with West and updates it
# * Runs east release
# If the _build_ is running due to the release creation, then the following also
# happens:
# * Creates 'artifacts' folder,
# * Copies release zip files and extra release notes files into it.
#
# Downloaded West modules, toolchain and nrfutil-toolchain-manager are cached in
# CI after the first time the entire build is run.
#
# The assumed precondition is that the repo was setup with below commands:
# mkdir -p <project_name>/project
# cd <project_name>/project
# git clone <project_url> .
#
# Every target assumes that is run from the repo's root directory, which is
# <project_name>/project.

install-dep:
	east install nrfutil-toolchain-manager

project-setup:
	# Make a West workspace around this project
	east init -l .
	# Use a faster update method
	east update -o=--depth=1 -n
	east install toolchain

pre-build:
	echo "Pre-build"

# Runs on every push to the main branch
quick-build:
	east build -b native_sim samples/basic

# Runs on every PR and when doing releases
release:
	# Change east.yml to control what is built.
	east release

# Pre-package target is only run in release process.
pre-package:
	mkdir -p artifacts
	cp release/*.zip artifacts
	cp scripts/pre_changelog.md artifacts
	cp scripts/post_changelog.md artifacts

test:
	east twister -T tests --coverage --coverage-tool lcov -p native_sim

test-remote:
	# Not supported on this repository

test-report-ci:
	junit2html twister-out/twister.xml twister-out/twister-report.html

# Intended to be used by developer, use 'pip install junit2html' to install
# tooling
test-report: test-report-ci
	firefox twister-out/twister-report.html

# Twister's coverage report by default includes all Zephyr sources, which is not
# what we want. Below coverage-report-ci target removes all Zephyr sources from
# coverage.info and generates a new coverage report.
REMOVE_DIR = $(shell realpath $(shell pwd)/../zephyr)

# This target is used in CI. It differs from coverage-report target in that it
# removes "project/" from the paths in coverage.info, so that the GitHub action
# that makes the coverage report can create proper links to the source files.
coverage-report-ci:
	rm -fr twister-out/coverage
	lcov -q --remove twister-out/coverage.info "${REMOVE_DIR}/*" -o twister-out/coverage.info  --rc lcov_branch_coverage=1

# Intended to be used by developer
coverage-report: coverage-report-ci
	genhtml -q --output-directory twister-out/coverage --ignore-errors source --branch-coverage --highlight --legend twister-out/coverage.info
	firefox twister-out/coverage/index.html

# CodeChecker section
# build and check targets are run on every push to the `main` and in PRs.
# store target is run only on the push to `main`.
# diff target is run only in PRs.
#
# Important: If building more projects, make sure to create separate build
# directories with -d flag, so they can be analyzed separately, see examples
# below.
codechecker-build:
	east build -b native_sim samples/basic -d build_basic
	east build -b native_sim samples/binary_encoding -d build_binary_encoding
	east build -b nrf52840dk_nrf52840 samples/bluetooth_service -d build_bluetooth_service
	east build -b native_sim samples/callbacks -d build_callbacks
	east build -b native_sim samples/validation -d build_validation

codechecker-check:
	east codechecker check -d build_basic
	east codechecker check -d build_binary_encoding
	east codechecker check -d build_bluetooth_service
	east codechecker check -d build_callbacks
	east codechecker check -d build_validation

codechecker-store:
	east codechecker store -d build_basic
	east codechecker store -d build_binary_encoding
	east codechecker store -d build_bluetooth_service
	east codechecker store -d build_callbacks
	east codechecker store -d build_validation

# Specify build folders that you want to analyze to the script as positional
# arguments, open it to learn more.
codechecker-diff:
	scripts/codechecker-diff.sh build_basic build_binary_encoding build_bluetooth_service build_callbacks build_validation
