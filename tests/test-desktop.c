/* SPDX-License-Identifier: GPL-3.0-only */
#include "host-ui-types.h"
#include <targets/f7/hybridus_sdk/hybridus_companion_sdk.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct VariableItem { const char* name; unsigned count, value; char text[32]; VariableItemChangeCallback cb; void* context; };
struct VariableItemList { VariableItem* data; size_t count; };
struct DialogMessage { const char* text; };
struct DialogsApp { const char* shown; };
static HybridusVgmSettings settings;
static int calls_get, calls_set, calls_save, dialogs_live;
static bool save_ok, color_ok;
static uint8_t shell;

VariableItem* variable_item_list_add(VariableItemList* list,const char* label,uint8_t count,VariableItemChangeCallback cb,void* ctx) {
    // Force movement on every insertion, so tests catch stale item references.
    VariableItem* new_data = calloc(list->count+1, sizeof(*new_data));
    assert(new_data);
    if(list->count) memcpy(new_data,list->data,list->count*sizeof(*new_data));
    free(list->data);
    list->data = new_data;
    VariableItem* row = &new_data[list->count++];
    *row = (VariableItem){ .name=label, .count=count, .cb=cb, .context=ctx };
    return row;
}
VariableItem* variable_item_list_get(VariableItemList* list,uint8_t id) { return id<list->count ? &list->data[id] : NULL; }
void variable_item_set_current_value_index(VariableItem* row,uint8_t v) { assert(v<row->count); row->value=v; }
void variable_item_set_current_value_text(VariableItem* row,const char* t) { assert(strlen(t)<sizeof(row->text)); strcpy(row->text,t); }
uint8_t variable_item_get_current_value_index(VariableItem* row) { return row->value; }
void* variable_item_get_context(VariableItem* row) { return row->context; }
void hybridus_vgm_settings_get(HybridusVgmSettings* p) { ++calls_get; *p=settings; }
void hybridus_vgm_settings_set(const HybridusVgmSettings* p) { ++calls_set; settings=*p; }
void hybridus_vgm_settings_init(void) {}
bool hybridus_vgm_settings_save(void) { ++calls_save; return save_ok; }
uint8_t hybridus_color_get(void) { return shell; }
bool hybridus_color_set(uint8_t color) { if(color_ok) shell=color; return color_ok; }
DialogMessage* dialog_message_alloc(void) { ++dialogs_live; DialogMessage* m=calloc(1,sizeof(*m)); assert(m); return m; }
void dialog_message_set_header(DialogMessage* m,const char* s,uint8_t x,uint8_t y,Align a,Align b) { assert(m && !strcmp(s,"VGM Save") && x==64 && y==4 && a==AlignCenter && b==AlignTop); }
void dialog_message_set_text(DialogMessage* m,const char* s,uint8_t x,uint8_t y,Align a,Align b) { assert(x==64 && y==28 && a==AlignCenter && b==AlignCenter); m->text=s; }
void dialog_message_set_buttons(DialogMessage* m,const char* a,const char* b,const char* c) { assert(m && !a && !c && !strcmp(b,"OK")); }
DialogMessageButton dialog_message_show(DialogsApp* a,const DialogMessage* m) { assert(m->text); a->shown=m->text; return DialogMessageButtonCenter; }
void dialog_message_free(DialogMessage* m) { --dialogs_live; free(m); }

static void change(VariableItemList* list,size_t row,uint8_t value) { VariableItem* p=&list->data[row]; assert(p->cb); p->value=value; p->cb(p); }

int main(void) {
    const uint16_t masks[]={0xf800,0x07e0,0x001f};
    for(unsigned seed=0;seed<=UINT16_MAX;seed++) {
        for(unsigned field=0;field<3;field++) {
            const unsigned v=(seed>>2) % hybridus_vgm_choices(field);
            HybridusVgmSettings probe={(uint16_t)seed,1,1,1};
            assert(hybridus_vgm_change(&probe,field,v));
            assert(hybridus_vgm_value(&probe,field)==v);
            assert((probe.foreground & ~masks[field])==(seed & ~masks[field]));
            assert(probe.invert && !probe.sync && probe.bezel);
        }
    }
    puts("RGB565: 196608 channel transitions preserve other color/flag bits and turn Sync off");
    HybridusVgmSettings bad={0xaaaa,1,1,1};
    for(unsigned f=0;f<=HybridusVgmFieldCount;f++) assert(!hybridus_vgm_change(&bad,f,hybridus_vgm_choices(f)));
    assert(!hybridus_vgm_change(&bad,(HybridusVgmField)-1,0));
    assert(bad.foreground==0xaaaa && bad.invert && bad.sync && bad.bezel);
    puts("Input bounds: invalid field/index rejected without changes");
    VariableItemList list={0}; DialogsApp dialogs={0}; DesktopSettingsApp app={&list,NULL,&dialogs};
    settings=(HybridusVgmSettings){0xffff,0,1,0}; shell=2;
    for(unsigned i=0;i<22;i++) variable_item_list_add(&list,"Base row",1,NULL,NULL);
    hybridus_vgm_rows_add(&app,27);
    assert(list.count==30 && !strcmp(list.data[22].name,"Shell Color"));
    assert(!strcmp(list.data[27].name,"VGM Sync") && app.hybridus_sync_row==&list.data[27]);
    assert(!strcmp(list.data[29].name,"VGM Save") && !list.data[29].cb && calls_get==1 && !calls_save);
    assert(!strcmp(list.data[22].text,"White") && !strcmp(list.data[23].text,"31") && !strcmp(list.data[24].text,"63") && !strcmp(list.data[25].text,"31"));
    assert(!strcmp(list.data[27].text,"On"));
    change(&list,23,10);
    assert(!settings.sync && !strcmp(list.data[27].text,"Off") && list.data[27].value==0 && calls_set==1);
    change(&list,26,1); change(&list,28,1); change(&list,27,1);
    assert(settings.invert && settings.bezel && settings.sync && calls_set==4);
    change(&list,24,12); change(&list,25,15);
    assert(settings.foreground==((10u<<11)|(12u<<5)|15u) && !settings.sync);
    change(&list,23,99);
    assert(list.data[23].value==10 && calls_set==6);
    puts("Desktop UI: row order, render, live RGB/flags; manual edit updates Sync row after array relocation");
    change(&list,22,1); assert(shell==2 && list.data[22].value==2);
    color_ok=true; change(&list,22,1); assert(shell==1 && !strcmp(list.data[22].text,"Black"));
    assert(!calls_save); hybridus_vgm_show_save_result(&app);
    assert(!strcmp(dialogs.shown,"Could not save") && calls_save==1 && !dialogs_live);
    save_ok=true; hybridus_vgm_show_save_result(&app);
    assert(!strcmp(dialogs.shown,"Saved on Flipper") && calls_save==2 && !dialogs_live);
    puts("Shell Color: rollback on failed save; VGM Save: explicit only, success/failure messages and no dialog leak");
    free(list.data);
    return 0;
}
