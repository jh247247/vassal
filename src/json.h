#ifndef JSON_H
#define JSON_H

extern char g_jsonInBuf[2][4096];
extern unsigned int g_jsonLen[2];

extern int g_jsonFlags;

// the skip render flag allows the renderer to skip rendering if the
// buffers havent changed.
#define JSON_SETSKIPRENDERFLAG (g_jsonFlags |= 0x08)
#define JSON_CLEARSKIPRENDERFLAG (g_jsonFlags &= ~0x08)
#define JSON_GETSKIPRENDERFLAG (g_jsonFlags&0x08)

// this handles the double buffering
#define JSON_WRITEBUF (g_jsonFlags&0x01)
#define JSON_READBUF ((~g_jsonFlags)&0x01)
#define JSON_SWAPBUFS do{g_jsonFlags ^= 0x01;\
    JSON_CLEARSKIPRENDERFLAG;}while(0)



// renderflag should be set while the scene is rendering.
// this is so we don't prematurely swap the two buffers.
#define JSON_GETRENDERFLAG (g_jsonFlags&0x02)
#define JSON_GETANIMFLAG (g_jsonFlags&0x04)

void JSON_init();
int JSON_render();


#endif /* JSON_H */
