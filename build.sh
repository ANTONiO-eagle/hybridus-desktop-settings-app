#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
kit=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
if [ "$#" -ne 1 ] || [ -e "$1" ] || [ -L "$1" ]; then
    echo "Usage: $0 NEW_BUILD_DIRECTORY" >&2
    exit 2
fi
repo=https://github.com/DarkFlippers/unleashed-firmware.git
base=9c6327764c18fc8376b5941aa8bde79fe681fe8f
git clone --depth 1 --branch unlshd-095 --no-recurse-submodules "$repo" "$1"
cd "$1"
[ "$(git rev-parse HEAD)" = "$base" ]
git submodule update --init --recursive --depth 1 --jobs "${SUBMODULE_JOBS:-4}"
git submodule status | sed -E 's/^ ([a-f0-9]+) ([^ ]+).*/\2 \1/' | LC_ALL=C sort > .git/hybridus-pinned.actual
LC_ALL=C sort "$kit/submodules-pinned.txt" | diff - .git/hybridus-pinned.actual
cp -R "$kit/source/applications" "$kit/source/targets" "$kit/source/assets" .
# This overlay describes the FAP ABI only. Do not use firmware/distribution targets.
FBT_NO_SYNC=1 ./fbt -j"${BUILD_JOBS:-2}" fap_desktop_settings
out=build/f7-firmware-DC/.extapps/desktop_settings.fap
test -s "$out"
sha256sum "$out"
