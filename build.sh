#!/usr/bin/env bash

set -eu

die() {
	echo -e "\033[31m$1\033[0m"
	exit 1
}

info() {
	echo -e "\033[33m$1\033[0m"
}

run() {
	info "$ $1"
	eval "$1"
}

# Ensure that ./build exists and is a directory
if test -e build; then
	if ! test -d build; then
		die "Error: ./build must be a directory. Aborting."
	fi
else
	run 'mkdir ./build'
fi

run 'cd ./build' || die "Cannot cd into ./build"
run 'cmake -DCMAKE_BUILD_TYPE=Release ..'
run 'make'

