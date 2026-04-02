/*
TJpg_Decoder.cpp

Created by Bodmer 18/10/19
*/

#include <string.h>
#include "TJpg_Decoder.h"

// Global instance (declared extern in header)
TJpg_Decoder TJpgDec;

TJpg_Decoder::TJpg_Decoder()
{
    thisPtr = this;
}

TJpg_Decoder::~TJpg_Decoder()
{
}

void TJpg_Decoder::setSwapBytes(bool swapBytes)
{
    _swap = swapBytes;
}

void TJpg_Decoder::setJpgScale(uint8_t scaleFactor)
{
    switch (scaleFactor)
    {
    case 1:  jpgScale = 0; break;
    case 2:  jpgScale = 1; break;
    case 4:  jpgScale = 2; break;
    case 8:  jpgScale = 3; break;
    default: jpgScale = 0; break;
    }
}

void TJpg_Decoder::setCallback(SketchCallback sketchCallback)
{
    tft_output = sketchCallback;
}

uint16_t TJpg_Decoder::jd_input(JDEC *jdec, uint8_t *buf, uint16_t len)
{
    TJpg_Decoder *thisPtr = TJpgDec.thisPtr;
    (void)jdec;

    if (thisPtr->jpg_source == TJPG_ARRAY)
    {
        if (thisPtr->array_index + len > thisPtr->array_size)
            len = thisPtr->array_size - thisPtr->array_index;

        if (buf)
            memcpy(buf, thisPtr->array_data + thisPtr->array_index, len);

        thisPtr->array_index += len;
    }

    return len;
}

uint16_t TJpg_Decoder::jd_output(JDEC *jdec, void *bitmap, JRECT *jrect)
{
    TJpg_Decoder *thisPtr = TJpgDec.thisPtr;
    (void)jdec;

    int16_t  x = jrect->left   + thisPtr->jpeg_x;
    int16_t  y = jrect->top    + thisPtr->jpeg_y;
    uint16_t w = jrect->right  + 1 - jrect->left;
    uint16_t h = jrect->bottom + 1 - jrect->top;

    return thisPtr->tft_output(x, y, w, h, (uint16_t *)bitmap, jdec->_dither, jdec->_invert);
}

JRESULT TJpg_Decoder::drawJpg(int32_t x, int32_t y, const uint8_t jpeg_data[], uint32_t data_size,
                               bool dither, bool invert)
{
    JDEC    jdec;
    JRESULT jresult;

    jpg_source  = TJPG_ARRAY;
    array_index = 0;
    array_data  = jpeg_data;
    array_size  = data_size;

    jpeg_x = x;
    jpeg_y = y;

    jdec.swap = _swap;

    jresult = jd_prepare(&jdec, jd_input, workspace, TJPGD_WORKSPACE_SIZE, 0);

    if (jresult == JDR_OK)
        jresult = jd_decomp(&jdec, jd_output, jpgScale, dither, invert);

    return jresult;
}

JRESULT TJpg_Decoder::getJpgSize(uint16_t *w, uint16_t *h, const uint8_t jpeg_data[], uint32_t data_size)
{
    JDEC    jdec;
    JRESULT jresult;

    *w = 0;
    *h = 0;

    jpg_source  = TJPG_ARRAY;
    array_index = 0;
    array_data  = jpeg_data;
    array_size  = data_size;

    jresult = jd_prepare(&jdec, jd_input, workspace, TJPGD_WORKSPACE_SIZE, 0);

    if (jresult == JDR_OK)
    {
        *w = jdec.width;
        *h = jdec.height;
    }

    return jresult;
}
