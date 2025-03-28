// Unit Include
#include "canvas.h"

namespace PxCryptPrivate
{

//===============================================================================================================
// Canvas
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------------
//Public:
Canvas::Canvas(QImage& image, const QByteArray& psk) :
    mSize(image.size()),
    mMetaAccess(image, !psk.isEmpty() ? psk : DEFAULT_SEED),
    mPxAccess(image, mMetaAccess),
    mTranslator(mPxAccess)
{}

//-Destructor---------------------------------------------------------------------------------------------------
//Public:
Canvas::~Canvas() { close(); }

//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
void Canvas::_reset() { mPxAccess.reset(); }

//Protected:
qint64 Canvas::readData(char* data, qint64 maxlen)
{
    if(maxlen == 0) // QIODevice doc's say this input (0) can be used to perform "post-reading operations"
        return mPxAccess.atEnd() ? -1 : 0;

    if(mPxAccess.atEnd())
    {
        qWarning("Attempt to read at end of canvas!");
        return -1;
    }

    qint64 i;
    for(i = 0; i < maxlen && !mPxAccess.atEnd(); i++)
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
    return mPxAccess.skip(maxSize);
}

qint64 Canvas::writeData(const char* data, qint64 len)
{
    if(mPxAccess.atEnd())
    {
        qWarning("Attempt to write at end of canvas!");
        return -1;
    }

    qint64 i;
    for(i = 0; i < len && !mPxAccess.atEnd(); i++)
    {
        quint8 byte = data[i];
        if(!mTranslator.weaveByte(byte))
            break; // prevents i++
    }

    // Always ensure data is current if Unbuffered is used
    if(openMode().testFlag(QIODevice::Unbuffered))
        mPxAccess.flush();

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
    Encoding e = static_cast<Encoding>(mMetaAccess.enc());
    if(e == Encoding::Relative)
        Q_ASSERT(mPxAccess.hasReferenceImage());
    else
        mPxAccess.setReferenceImage(nullptr); // Force-clear reference when it's not needed
    _reset();

    // Base implementation
    return QIODevice::open(mode);
}

void Canvas::close()
{
    mPxAccess.flush();
    return QIODevice::close();
}

bool Canvas::atEnd() const { return mPxAccess.atEnd(); }

Canvas::metavalue_t Canvas::bpc() const { return mMetaAccess.bpc(); }
Canvas::Encoding Canvas::encoding() const { return static_cast<Encoding>(mMetaAccess.enc()); }

void Canvas::setBpc(metavalue_t bpc) { mMetaAccess.setBpc(bpc); }
void Canvas::setEncoding(Encoding enc) { mMetaAccess.setEnc(enc); }
void Canvas::setReference(const QImage* ref) { mPxAccess.setReferenceImage(ref); }

}
