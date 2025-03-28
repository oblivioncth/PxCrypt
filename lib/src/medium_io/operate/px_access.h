#ifndef PX_ACCESS_H
#define PX_ACCESS_H

// Project Includes
#include "medium_io/traverse/canvas_traverser.h"

namespace PxCryptPrivate
{

class MetaAccess;

class PxAccess
{
//-Instance Variables------------------------------------------------------------------------------------------------------
private:
    QRgb* mPixels;
    const QRgb* mRefPixels;
    CanvasTraverser mTraverser;

    std::array<quint8, 4> mBuffer;
    bool mNeedFlush;

//-Constructor---------------------------------------------------------------------------------------------------------
public:
    PxAccess(QImage& canvas, MetaAccess& metaAccess);

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    // Original canvas pixel access
    QRgb& canvasPixel();
    const QRgb& constCanvasPixel() const;
    quint8 canvasRed() const;
    quint8 canvasGreen() const;
    quint8 canvasBlue() const;
    quint8 canvasAlpha() const;

    // Reference canvas pixel access
    const QRgb& referencePixel() const;
    quint8 referenceRed() const;
    quint8 referenceGreen() const;
    quint8 referenceBlue() const;

    // Buffer
    void fillBuffer();
    void flushBuffer();

public:
    // Stat
    bool hasReferenceImage() const;
    int availableBits() const;
    int bitIndex() const;
    bool atEnd() const;

    // Manipulation
    void setReferenceImage(const QImage* ref);
    void reset();
    qint64 skip(qint64 bytes);
    void advanceBits(int bitCount);
    void flush();

    // Pixel Access
    quint8& bufferedValue();
    quint8 constBufferedValue() const;
    quint8 originalValue() const;
    quint8 referenceValue() const;
};

}

#endif // PX_ACCESS_H
