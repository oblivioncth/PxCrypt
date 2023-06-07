// Unit Include
#include "px_skimmer.h"

namespace PxCrypt
{
/*! @cond */

//===============================================================================================================
// PxSkimmer
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------------
//Public:
PxSkimmer::PxSkimmer(const QImage* surface, const QImage* medium, QByteArrayView psk, quint8 bpc, EncType type) :
    mType(type),
    mPixels(reinterpret_cast<const QRgb*>(surface->bits())),
    mRefPixels(!medium ? nullptr : reinterpret_cast<const QRgb*>(medium->bits())),
    mSequence(surface->size(), psk),
    mMask((0b1 << bpc) - 1),
    mLimitReached(false)
{
    Q_ASSERT(bpc <= 7);
    if(mType == EncType::Relative)
        Q_ASSERT(medium);

    advance();
}

//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
void PxSkimmer::advance()
{
    quint64 index = mSequence.next();
    const QRgb& pixel = mPixels[index];
    mChannel = 0;

   switch(mType)
   {
        case EncType::Absolute:
            mBuffer[0] = qRed(pixel) & mMask;
            mBuffer[1] = qGreen(pixel) & mMask;
            mBuffer[2] = qBlue(pixel) & mMask;
            break;

        case EncType::Relative:
        {
            const QRgb& refPixel = mRefPixels[index];
            mBuffer[0] = Qx::distance(qRed(pixel), qRed(refPixel)) & mMask;
            mBuffer[1] = Qx::distance(qGreen(pixel), qGreen(refPixel)) & mMask;
            mBuffer[2] = Qx::distance(qBlue(pixel), qBlue(refPixel)) & mMask;
            break;
        }

        default:
            qCritical("unhandled encoding type.");
   }
}

//Public:
bool PxSkimmer::isAtLimit() { return mLimitReached; }

quint8 PxSkimmer::next()
{
    if(mLimitReached)
    {
        qWarning("attempted to skim pixels when at the limit.");
        return 0;
    }

    quint8 chunk = mBuffer[mChannel];

    if(mChannel == 2)
    {
        if(mSequence.hasNext())
            advance();
        else
            mLimitReached = true;
    }
    else
        mChannel++;

    return chunk;
}

/*! @endcond */
}
