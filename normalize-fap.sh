#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
if [ "$#" -ne 2 ] || [ ! -f "$1" ] || [ -e "$2" ] || [ -L "$2" ]; then
    echo "Usage: $0 BUILT_FAP NEW_OUTPUT_FAP" >&2
    exit 2
fi
arm-none-eabi-objcopy --remove-section=.gnu_debuglink "$1" "$2"
sha256sum "$2"
