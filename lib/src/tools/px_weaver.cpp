// Unit Include
#include "px_weaver.h"

namespace PxCrypt
{

//===============================================================================================================
// PxWeaver
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------------
//Public:
PxWeaver::PxWeaver(QImage* canvas, QStringView psk, quint8 bpc, EncType type) :
    mType(type),
    mPixels(reinterpret_cast<QRgb*>(canvas->bits())),
    mSequence(canvas->size(), psk),
    mClearMask(~((0b1 << bpc) - 1)),
    mAtEnd(false)
{
    advance();
}

//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
void PxWeaver::advance()
{
    if(mSequence.hasNext())
    {
        mPxIndex = mSequence.next();
        mChannel = 0;

        QRgb& currentPixel = mPixels[mPxIndex];
        mBuffer[0] = qRed(currentPixel);
        mBuffer[1] = qGreen(currentPixel);
        mBuffer[2] = qBlue(currentPixel);
        mBuffer[3] = qAlpha(currentPixel);
    }
    else
        mAtEnd = true;
}

//Public:
void PxWeaver::weave(quint8 chunk)
{
    if(!mAtEnd)
    {
        switch(mType)
        {
            case EncType::Absolute:
            {
                quint8& val = mBuffer[mChannel];
                val = (val & mClearMask) | chunk;
                break;
            }

            case EncType::Relative:
            {
                quint8& val = mBuffer[mChannel];
                if(val > 127)
                    val -= chunk;
                else
                    val += chunk;
                break;
            }

            default:
                qCritical("unhandled encoding type.");
        }

        if(mChannel == 2)
        {
            flush();
            advance();
        }
        else
            mChannel++;
    }
}

void PxWeaver::flush()
{
    if(!mAtEnd)
    {
        QRgb& currentPixel = mPixels[mPxIndex];
        currentPixel = qRgba(mBuffer[0], mBuffer[1], mBuffer[2], mBuffer[3]);
    }
}

}
