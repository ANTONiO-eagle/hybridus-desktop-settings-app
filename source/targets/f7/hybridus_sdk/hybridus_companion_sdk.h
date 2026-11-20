/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <applications/settings/desktop_settings/hybridus_vgm_model.h>

/* Declarations for the binary-verified F7 host; no implementations in the FAP. */
void hybridus_vgm_settings_get(HybridusVgmSettings* settings);
void hybridus_vgm_settings_set(const HybridusVgmSettings* settings);
bool hybridus_vgm_settings_save(void);
uint8_t hybridus_color_get(void);
bool hybridus_color_set(uint8_t color);
