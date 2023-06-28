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
PxWeaver::PxWeaver(PxAccessWrite* canvasAccess) :
    mCanvasAccess(canvasAccess),
    mClearMask(~((0b1 << mCanvasAccess->bpc()) - 1))
{
    fill();
}

//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
void PxWeaver::fill()
{
    mBuffer[Channel::Red] = mCanvasAccess->red();
    mBuffer[Channel::Green] = mCanvasAccess->green();
    mBuffer[Channel::Blue] = mCanvasAccess->blue();
    mBuffer[Channel::Alpha] = mCanvasAccess->alpha();
}

//Public:
void PxWeaver::weave(quint8 chunk)
{
    if(mCanvasAccess->atEnd())
        return;

    Channel ch = mCanvasAccess->channel();

    switch(mCanvasAccess->strat())
    {
        case EncStrat::Direct:
        {
            quint8& val = mBuffer[ch];
            val = (val & mClearMask) | chunk;
            break;
        }

        case EncStrat::Displaced:
        {
            quint8& val = mBuffer[ch];
            if(val > 127)
                val -= chunk;
            else
                val += chunk;
            break;
        }

        default:
            qCritical("unhandled encoding strat.");
    }

    // Flush if going to next pixel
    bool flushing = mCanvasAccess->pixelExhausted();
    if(flushing)
        flush();

    // Advance channels, fill after flushing if not at end
    if(mCanvasAccess->nextChannel() && flushing)
        fill();
}

void PxWeaver::flush()
{
    if(mCanvasAccess->atEnd())
        return;

    mCanvasAccess->pixel() = qRgba(mBuffer[Channel::Red],
                             mBuffer[Channel::Green],
                             mBuffer[Channel::Blue],
                             mBuffer[Channel::Alpha]);
}

/*! @endcond */
}
