/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct VariableItem VariableItem;
typedef struct VariableItemList VariableItemList;
typedef struct DialogMessage DialogMessage;
typedef struct DialogsApp DialogsApp;
typedef void (*VariableItemChangeCallback)(VariableItem*);
typedef enum { AlignCenter, AlignTop } Align;
typedef struct {
    VariableItemList* variable_item_list;
    VariableItem* hybridus_sync_row;
    DialogsApp* dialogs;
} DesktopSettingsApp;

void hybridus_vgm_rows_add(DesktopSettingsApp* app, uint8_t sync_row_index);
void hybridus_vgm_show_save_result(DesktopSettingsApp* app);
VariableItem* variable_item_list_add(VariableItemList*,const char*,uint8_t,VariableItemChangeCallback,void*);
VariableItem* variable_item_list_get(VariableItemList*,uint8_t);
void variable_item_set_current_value_index(VariableItem*,uint8_t);
void variable_item_set_current_value_text(VariableItem*,const char*);
uint8_t variable_item_get_current_value_index(VariableItem*);
void* variable_item_get_context(VariableItem*);
DialogMessage* dialog_message_alloc(void);
void dialog_message_set_header(DialogMessage*,const char*,uint8_t,uint8_t,Align,Align);
void dialog_message_set_text(DialogMessage*,const char*,uint8_t,uint8_t,Align,Align);
void dialog_message_set_buttons(DialogMessage*,const char*,const char*,const char*);
typedef enum { DialogMessageButtonCenter } DialogMessageButton;
DialogMessageButton dialog_message_show(DialogsApp*,const DialogMessage*);
void dialog_message_free(DialogMessage*);
