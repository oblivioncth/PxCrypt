#ifndef PX_WEAVER_H
#define PX_WEAVER_H

// Qt Includes
#include <QImage>

// Project Includes
#include "px_access_write.h"

namespace PxCrypt
{
/*! @cond */

class PxWeaver
{
//-Instance Variables------------------------------------------------------------------------------------------------------
private:
    PxAccessWrite* mCanvasAccess;
    quint8 mClearMask;
    quint8 mBuffer[4];

//-Constructor---------------------------------------------------------------------------------------------------------
public:
    PxWeaver(PxAccessWrite* canvasAccess);

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    void fill();

public:
    void weave(quint8 chunk);
    void flush();
};

/*! @endcond */
}

#endif // PX_WEAVER_H
