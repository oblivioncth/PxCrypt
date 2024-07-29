// Unit Include
#include "canvas.h"

namespace PxCryptPrivate
{

//===============================================================================================================
// Canvas
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------------
//Public:
Canvas::Canvas(QImage* image, const Frame& frame, const QImage* refImage) :
    mAccess(image, frame.traverser(), frame.bpc(), refImage),
    mTranslator(mAccess)
{
    Q_ASSERT(image && !image->isNull());
}

//-Destructor---------------------------------------------------------------------------------------------------
//Public:
Canvas::~Canvas() { close(); }

//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
void Canvas::_reset() { mAccess.reset(); }

//Protected:
qint64 Canvas::readData(char* data, qint64 maxlen)
{
    if(maxlen == 0) // QIODevice doc's say this input can be used to perform "post-reading operations"
        return mAccess.atEnd() ? -1 : 0;

    if(mAccess.atEnd())
    {
        qWarning("Attempt to read at end of canvas!");
        return -1;
    }

    qint64 i;
    for(i = 0; i < maxlen && !mAccess.atEnd(); i++)
    {
        char& byte = data[i];
        if(!mTranslator.skimByte(reinterpret_cast<quint8&>(byte)))
            break; // prevents i++
    }

    return i;
}

qint64 Canvas::skipData(qint64 maxSize)
{
    Q_ASSERT(maxSize >= 0);
    return mAccess.skip(maxSize);
}

qint64 Canvas::writeData(const char* data, qint64 len)
{
    if(mAccess.atEnd())
    {
        qWarning("Attempt to write at end of canvas!");
        return -1;
    }

    qint64 i;
    for(i = 0; i < len && !mAccess.atEnd(); i++)
    {
        quint8 byte = data[i];
        if(!mTranslator.weaveByte(byte))
            break; // prevents i++
    }

    // Always ensure data is current if Unbuffered is used
    if(openMode().testFlag(QIODevice::Unbuffered))
        mAccess.flush();

    return i;
}

//Public:
bool Canvas::isSequential() const { return true; }

bool Canvas::open(OpenMode mode)
{
    // Only some modes are supported
    if(mode.testAnyFlags(Append | Truncate | Text | Unbuffered | NewOnly | ExistingOnly))
        qCritical("Unsupported open mode!");

    // Prepare for access
    _reset();

    // Base implementation
    return QIODevice::open(mode);
}

void Canvas::close()
{
    mAccess.flush();
    return QIODevice::close();
}

bool Canvas::atEnd() const { return mAccess.atEnd(); }

}
