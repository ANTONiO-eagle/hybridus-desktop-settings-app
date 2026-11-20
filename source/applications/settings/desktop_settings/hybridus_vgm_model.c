/* SPDX-License-Identifier: GPL-3.0-only */
#include "hybridus_vgm_model.h"

static const uint8_t choices[] = {32, 64, 32, 2, 2, 2};
static const uint8_t shifts[] = {11, 5, 0};

uint8_t hybridus_vgm_choices(HybridusVgmField field) {
    return (unsigned)field < HybridusVgmFieldCount ? choices[field] : 0;
}

uint8_t hybridus_vgm_value(const HybridusVgmSettings* settings, HybridusVgmField field) {
    if((unsigned)field <= HybridusVgmBlue) {
        return (settings->foreground >> shifts[field]) & (choices[field] - 1);
    }
    switch(field) {
    case HybridusVgmInvert: return settings->invert != 0;
    case HybridusVgmSync: return settings->sync != 0;
    case HybridusVgmBezel: return settings->bezel != 0;
    default: return 0;
    }
}

bool hybridus_vgm_change(HybridusVgmSettings* settings, HybridusVgmField field, uint8_t value) {
    if(value >= hybridus_vgm_choices(field)) return false;
    if((unsigned)field <= HybridusVgmBlue) {
        const uint16_t mask = (choices[field] - 1) << shifts[field];
        settings->foreground = (settings->foreground & ~mask) | ((uint16_t)value << shifts[field]);
        settings->sync = 0;
    } else if(field == HybridusVgmInvert) {
        settings->invert = value;
    } else if(field == HybridusVgmSync) {
        settings->sync = value;
    } else if(field == HybridusVgmBezel) {
        settings->bezel = value;
    }
    return true;
}
