#include "kernel.h"
#include <limits.h>
/* import our font that's in the object file we've created above */
extern char _binary_font_psf_start;
extern char _binary_font_psf_end;
 
uint16_t *unicode;
extern uint32_t* fb;
extern int height;
extern int pixelwidth;
extern int pitch;
/* import our font that's in the object file we've created above */
extern char _binary_font_start;
PSF_font* font;
#define PIXEL uint32_t   /* pixel pointer */

// The next two functions were copied as-is from OSDev
// I do not understand them
void psf_init()
{
    uint16_t glyph = 0;
    /* cast the address to PSF header struct */
    font = (PSF_font*)&_binary_font_psf_start;
    /* is there a unicode table? */
    if (font->flags) {
        unicode = '\0';
        return; 
    }
 
    /* get the offset of the table */
    unsigned char *s = (
    (unsigned char*)&_binary_font_psf_start +
      font->headersize +
      font->numglyph * font->bytesperglyph
    );
    /* allocate memory for translation table */
    unicode = calloc(USHRT_MAX, 2);
    while(s>(unsigned char*)_binary_font_psf_end) {
        uint16_t uc = (uint16_t)(s[0]);
        if(uc == 0xFF) {
            glyph++;
            s++;
            continue;
        } else if(uc & 128) {
            /* UTF-8 to unicode */
            if((uc & 32) == 0 ) {
                uc = ((s[0] & 0x1F)<<6)+(s[1] & 0x3F);
                s++;
            } else
            if((uc & 16) == 0 ) {
                uc = ((((s[0] & 0xF)<<6)+(s[1] & 0x3F))<<6)+(s[2] & 0x3F);
                s+=2;
            } else
            if((uc & 8) == 0 ) {
                uc = ((((((s[0] & 0x7)<<6)+(s[1] & 0x3F))<<6)+(s[2] & 0x3F))<<6)+(s[3] & 0x3F);
                s+=3;
            } else
                uc = 0;
        }
        /* save translation */
        unicode[uc] = glyph;
        s++;
    }
}
void putchar(
    /* note that this is int, not char as it's a unicode character */
    unsigned short int c,
    /* cursor position on screen, in characters not in pixels */
    int cx, int cy,
    /* foreground and background colors, say 0xFFFFFF and 0x000000 */
    uint32_t fg, uint32_t bg)
{
    /* cast the address to PSF header struct */
    PSF_font *font = (PSF_font*)&_binary_font_psf_start;
    /* we need to know how many bytes encode one row */
    int bytesperline=(font->width+7)/8;
    /* unicode translation */
    if(unicode != NULL) {
        c = unicode[c];
    }
    /* get the glyph for the character. If there's no
       glyph for a given character, we'll display the first glyph. */
    unsigned char *glyph =
     (unsigned char*)&_binary_font_psf_start +
     font->headersize +
     (c>0&&c<font->numglyph?c:0)*font->bytesperglyph;
    /* calculate the upper left corner on screen where we want to display.
       we only do this once, and adjust the offset later. This is faster. */
    int offs =
        (cy * font->height * pitch) +
        (cx * (font->width + 1) * sizeof(PIXEL));
    /* finally display pixels according to the bitmap */
    int x,y, line;
    for(y=0; y<font->height; y++){
        line = offs;
        for(x=0; x<font->width; x++){
            *((uint32_t*) (fb + line)) = glyph[x/8] & (0x80 >> (x & 7)) ? fg : bg;
            line += sizeof(PIXEL);
        }
        glyph += bytesperline;
        offs +=pitch;
    }
}

extern int cursorY;
extern int cursorX;

void print(char* text, int len) {
    for (int i = 0; i<len; i++){
        if (text[i] == ' '){
            cursorX += 1;
            continue;
        }
        putCharAuto(text[i], 0xFFFFFF, 0x000000);
    }
    cursorY += 1;
    cursorX = 0;
}

void putCharAuto(unsigned short c, uint32_t fg, uint32_t bg) {
    int maxCharX = 27;
    int maxCharY = height/font->width-5;
    //putchar(c, cursorX, cursorY, 0x00FF00, 0xFF00FF);
    putchar(c, cursorX, cursorY, fg, bg);
    cursorX = cursorX+1;
    if (cursorX>=maxCharX){
        cursorX=0;
        cursorY+=1;
    }
    if (cursorY>=maxCharY) {
        cursorY = 0;
    }
}
