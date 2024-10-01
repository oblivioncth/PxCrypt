// Unit Includes
#include "encdec.h"

// Qt Includes
#include <QSize>

// Qx Includes
#include <qx/core/qx-algorithm.h>

namespace PxCryptPrivate
{

//-Namespace Functions-------------------------------------------------------------------------------------------------
QImage standardizeImage(const QImage& img)
{
    /* This isn't made entirely clear by the QImage/QColor documentation, but in order to correctly manipulate
     * an image's pixles via QImage::bits() and QRgb (as shown in the QImage example), the image must be in a format
     * in which the pixels are 32-bits wide, ordered as AARRGGBB. The QRgb documentation makes it clear that this is the
     * QRgb layout, but then elsewhere makes it sound like you can use these values with qRed(), qGreen(), regardless of format
     * when accessing pixel data directly; however, although the red(), green(), blue() and alpha() helpers are byte-order
     * agnostic, they are not format agnostic. You can only read/assign QRgb values in a format agnostic manner by using
     * QImage::pixel() and QImage::setPixel(), but those both have much more overhead as stated in the QImage docs.
     *
     * Because of this, all images are standardized to the ARGB layout to ensure the correct channels are accessed during
     * encoding/decoding. This of course introduces some performance loss due to the potential need to convert the whole image,
     * but it's less than the overhead of using QImage::pixel()/QImage::setPixel() on every pixel. It is possible to check the
     * input format and read color channels in reverse if the format uses BGRA, but this would only cover a small few formats
     * and conversion for standardization needs to occur for non 32-bit, specifically 8-bits per channel, formats anyway so mind
     * as well handle channel ordering standardization through conversion as well.
     */

    QImage std = img; // Because of Qt's CoW system this occurs almost no penalty if the format is already acceptable
    QImage::Format fmt = std.format();
    if(fmt != QImage::Format_ARGB32 && fmt != QImage::Format_RGB32)
        std.convertTo(std.hasAlphaChannel() ? QImage::Format_ARGB32 : QImage::Format_RGB32);

    return std;
}

}
