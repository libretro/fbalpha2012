#ifndef _BURN_DEVICES_SKN_SSPR_H
#define _BURN_DEVICES_SKN_SSPR_H

void skns_sprite_kludge(int x, int y);
void skns_draw_sprites(UINT16 *bitmap, UINT32* spriteram_source, INT32 spriteram_size, UINT8* gfx_source, INT32 gfx_length, UINT32* sprite_regs, INT32 disable_priority);

#endif
