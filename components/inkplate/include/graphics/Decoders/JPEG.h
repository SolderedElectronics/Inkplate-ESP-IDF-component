#ifndef DECODER_JPEG_H
#define DECODER_JPEG_H

#include <stdint.h>
#include <stdbool.h>

#include "rom/tjpgd.h"

#define TJPGD_WORKSPACE_SIZE 16688

class Inkplate;

class JPEG
{
public:
    JPEG(Inkplate *inkplate);

    bool draw(uint8_t *buf, int32_t len, int x, int y, bool invert = false, bool dither = false);

private:
    static UINT inputCallback(JDEC *jdec, BYTE *buf, UINT len);
    static UINT outputCallback(JDEC *jdec, void *bitmap, JRECT *rect);

    Inkplate     *m_inkplate;
    int           m_x;
    int           m_y;
    bool          m_invert;
    bool          m_dither;
    int64_t       m_lastYieldUs;
    uint8_t      *m_lineBuf;  // RGB line buffer (h * width * 3 bytes, dither path only)
    int           m_lineBufH; // MCU block height, set from first callback
    int           m_lineBufY; // current absolute Y offset in the image

    static JPEG *m_instance;
};

#endif
