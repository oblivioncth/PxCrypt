// Unit Include
#include "px_weaver.h"

namespace PxCrypt
{
/*! @cond */

//===============================================================================================================
// PxWeaver
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------------
//Public:
PxWeaver::PxWeaver(QImage* canvas, QByteArrayView psk, quint8 bpc, EncType type) :
    mType(type),
    mPixels(reinterpret_cast<QRgb*>(canvas->bits())),
    mPxSequence(canvas->size(), psk),
    mChSequence(psk),
    mClearMask(~((0b1 << bpc) - 1)),
    mAtEnd(false)
{
    advance();
}

//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
void PxWeaver::advance()
{
    if(mPxSequence.hasNext())
    {
        mPxIndex = mPxSequence.next();

        QRgb& currentPixel = mPixels[mPxIndex];
        mBuffer[Channel::Red] = qRed(currentPixel);
        mBuffer[Channel::Green] = qGreen(currentPixel);
        mBuffer[Channel::Blue] = qBlue(currentPixel);
        mBuffer[Channel::Alpha] = qAlpha(currentPixel);
    }
    else
        mAtEnd = true;
}

//Public:
void PxWeaver::weave(quint8 chunk)
{
    if(!mAtEnd)
    {
        Channel ch = mChSequence.next();

        switch(mType)
        {
            case EncType::Absolute:
            {
                quint8& val = mBuffer[ch];
                val = (val & mClearMask) | chunk;
                break;
            }

            case EncType::Relative:
            {
                quint8& val = mBuffer[ch];
                if(val > 127)
                    val -= chunk;
                else
                    val += chunk;
                break;
            }

            default:
                qCritical("unhandled encoding type.");
        }

        if(mChSequence.pixelExhausted())
        {
            flush();
            advance();
        }
    }
}

void PxWeaver::flush()
{
    if(!mAtEnd)
    {
        QRgb& currentPixel = mPixels[mPxIndex];
        currentPixel = qRgba(mBuffer[Channel::Red],
                             mBuffer[Channel::Green],
                             mBuffer[Channel::Blue],
                             mBuffer[Channel::Alpha]);
    }
}

/*! @endcond */
}
