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
PxSkimmer::PxSkimmer(PxAccessRead* surfaceAccess, const QImage* medium) :
    mSurfaceAccess(surfaceAccess),
    mRefPixels(!medium ? nullptr : reinterpret_cast<const QRgb*>(medium->bits())),
    mMask((0b1 << surfaceAccess->bpc()) - 1)
{
    if(mSurfaceAccess->type() == EncType::Relative)
        Q_ASSERT(medium);
}

//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
quint8 PxSkimmer::referenceChannelValue()
{
    const QRgb& refPixel = mRefPixels[mSurfaceAccess->index()];

    switch(mSurfaceAccess->channel())
    {
        case Channel::Red:
            return qRed(refPixel);
        case Channel::Green:
            return qGreen(refPixel);
        case Channel::Blue:
            return qBlue(refPixel);

        default:
            qCritical("Illegal channel in rotation");
    }

    return 0; // Never reached
}

//Public:
bool PxSkimmer::isAtLimit() { return mSurfaceAccess->atEnd(); }

quint8 PxSkimmer::next()
{
    if(isAtLimit())
    {
        qWarning("attempted to skim pixels when at the limit.");
        return 0;
    }

    quint8 chunk;

    switch(mSurfaceAccess->type())
    {
        case EncType::Absolute:
            chunk = mSurfaceAccess->channelValue() & mMask;
            break;

        case EncType::Relative:
            chunk = Qx::distance(referenceChannelValue(), mSurfaceAccess->channelValue()) & mMask;
            break;

        default:
            qCritical("unhandled encoding type.");
    }

    mSurfaceAccess->nextChannel();
    return chunk;
}

/*! @endcond */
}
