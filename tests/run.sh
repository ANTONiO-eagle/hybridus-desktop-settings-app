#!/bin/sh
set -eu
here=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
src="${1:-$here/../source}"
app="$src/applications/settings/desktop_settings"
dir=$(mktemp -d)
trap 'rm -rf "$dir"' EXIT HUP INT TERM
sed 's/^#include "hybridus_vgm_ui.h"$/#include "host-ui-types.h"/' "$app/hybridus_vgm_ui.c" > "$dir/hybridus_vgm_ui.c"
"${CC:-cc}" -std=c11 -O1 -g -Wall -Wextra -Werror -fsanitize=undefined -fno-sanitize-recover=all \
    -I "$src" -I "$here" "$here/test-desktop.c" "$app/hybridus_vgm_model.c" "$dir/hybridus_vgm_ui.c" -o "$dir/desktop-tests"
"$dir/desktop-tests"
