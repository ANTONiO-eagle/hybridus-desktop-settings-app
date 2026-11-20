/* SPDX-License-Identifier: GPL-3.0-only */
#include "hybridus_vgm_ui.h"
#include <targets/f7/hybridus_sdk/hybridus_companion_sdk.h>
#include <stdio.h>

static const char* const shell_colors[] = {"Auto", "Black", "White", "Transparent"};
static const char* const names[] = {
    "VGM Red", "VGM Green", "VGM Blue", "VGM Invert", "VGM Sync", "VGM Bezel"};

static void set_row(VariableItem* row, HybridusVgmField field, uint8_t value) {
    variable_item_set_current_value_index(row, value);
    if(field <= HybridusVgmBlue) {
        char label[4];
        snprintf(label, sizeof(label), "%u", (unsigned)value);
        variable_item_set_current_value_text(row, label);
    } else {
        variable_item_set_current_value_text(row, value ? "On" : "Off");
    }
}

static void change_field(VariableItem* row, HybridusVgmField field) {
    DesktopSettingsApp* app = variable_item_get_context(row);
    HybridusVgmSettings settings = {0};
    hybridus_vgm_settings_get(&settings);
    if(hybridus_vgm_change(&settings, field, variable_item_get_current_value_index(row))) {
        hybridus_vgm_settings_set(&settings);
    }
    set_row(row, field, hybridus_vgm_value(&settings, field));
    if(field <= HybridusVgmBlue) {
        set_row(app->hybridus_sync_row, HybridusVgmSync, settings.sync != 0);
    }
}

#define FIELD_CALLBACK(name) \
    static void on_##name(VariableItem* row) { change_field(row, HybridusVgm##name); }
FIELD_CALLBACK(Red)
FIELD_CALLBACK(Green)
FIELD_CALLBACK(Blue)
FIELD_CALLBACK(Invert)
FIELD_CALLBACK(Sync)
FIELD_CALLBACK(Bezel)
#undef FIELD_CALLBACK

static const VariableItemChangeCallback callbacks[] = {
    on_Red, on_Green, on_Blue, on_Invert, on_Sync, on_Bezel};

static void shell_set_row(VariableItem* row, uint8_t color) {
    if(color >= sizeof(shell_colors) / sizeof(*shell_colors)) color = 0;
    variable_item_set_current_value_index(row, color);
    variable_item_set_current_value_text(row, shell_colors[color]);
}

static void shell_changed(VariableItem* row) {
    const uint8_t color = variable_item_get_current_value_index(row);
    if(hybridus_color_set(color)) {
        shell_set_row(row, color);
    } else {
        shell_set_row(row, hybridus_color_get());
    }
}

void hybridus_vgm_rows_add(DesktopSettingsApp* app, uint8_t sync_row_index) {
    VariableItemList* list = app->variable_item_list;
    VariableItem* row = variable_item_list_add(list, "Shell Color", 4, shell_changed, NULL);
    shell_set_row(row, hybridus_color_get());
    HybridusVgmSettings settings = {0};
    hybridus_vgm_settings_get(&settings);
    for(HybridusVgmField field = HybridusVgmRed; field < HybridusVgmFieldCount; field++) {
        row = variable_item_list_add(list, names[field], hybridus_vgm_choices(field), callbacks[field], app);
        set_row(row, field, hybridus_vgm_value(&settings, field));
    }
    variable_item_list_add(list, "VGM Save", 1, NULL, NULL);
    // Array addresses may change during insertion. Resolve after the last add;
    // it stays valid only until the start scene clears the list.
    app->hybridus_sync_row = variable_item_list_get(list, sync_row_index);
}

void hybridus_vgm_show_save_result(DesktopSettingsApp* app) {
    const bool saved = hybridus_vgm_settings_save();
    DialogMessage* message = dialog_message_alloc();
    dialog_message_set_header(message, "VGM Save", 64, 4, AlignCenter, AlignTop);
    dialog_message_set_text(
        message, saved ? "Saved on Flipper" : "Could not save",
        64, 28, AlignCenter, AlignCenter);
    dialog_message_set_buttons(message, NULL, "OK", NULL);
    dialog_message_show(app->dialogs, message);
    dialog_message_free(message);
}
