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
    mPxSequence(surface->size(), psk),
    mChSequence(psk),
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
    quint64 index = mPxSequence.next();
    const QRgb& pixel = mPixels[index];

   switch(mType)
   {
        case EncType::Absolute:
            mBuffer[Channel::Red] = qRed(pixel) & mMask;
            mBuffer[Channel::Green] = qGreen(pixel) & mMask;
            mBuffer[Channel::Blue] = qBlue(pixel) & mMask;
            break;

        case EncType::Relative:
        {
            const QRgb& refPixel = mRefPixels[index];
            mBuffer[Channel::Red] = Qx::distance(qRed(pixel), qRed(refPixel)) & mMask;
            mBuffer[Channel::Green] = Qx::distance(qGreen(pixel), qGreen(refPixel)) & mMask;
            mBuffer[Channel::Blue] = Qx::distance(qBlue(pixel), qBlue(refPixel)) & mMask;
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

    quint8 chunk = mBuffer[mChSequence.next()];

    if(mChSequence.pixelExhausted())
    {
        if(mPxSequence.hasNext())
            advance();
        else
            mLimitReached = true;
    }

    return chunk;
}

/*! @endcond */
}
