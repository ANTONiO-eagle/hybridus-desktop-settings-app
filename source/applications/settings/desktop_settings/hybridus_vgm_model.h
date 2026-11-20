/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Observed F7 Hybridus UI 0.7 host ABI, not a stock Unleashed API. */
typedef struct {
    uint16_t foreground;
    uint8_t invert;
    uint8_t sync;
    uint8_t bezel;
} HybridusVgmSettings;

_Static_assert(sizeof(HybridusVgmSettings) == 6, "VGM settings size");
_Static_assert(offsetof(HybridusVgmSettings, invert) == 2, "VGM invert offset");
_Static_assert(offsetof(HybridusVgmSettings, sync) == 3, "VGM sync offset");
_Static_assert(offsetof(HybridusVgmSettings, bezel) == 4, "VGM bezel offset");

typedef enum {
    HybridusVgmRed,
    HybridusVgmGreen,
    HybridusVgmBlue,
    HybridusVgmInvert,
    HybridusVgmSync,
    HybridusVgmBezel,
    HybridusVgmFieldCount,
} HybridusVgmField;

uint8_t hybridus_vgm_choices(HybridusVgmField field);
uint8_t hybridus_vgm_value(const HybridusVgmSettings* settings, HybridusVgmField field);
bool hybridus_vgm_change(HybridusVgmSettings* settings, HybridusVgmField field, uint8_t value);
