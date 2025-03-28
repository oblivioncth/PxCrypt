#ifndef ARTWORK_H
#define ARTWORK_H

// Qt Includes
#include <QByteArray>

// Qx Includes
#include <qx/core/qx-error.h>

// Project Includes
#include "medium_io/canvas.h"
#include "art_io/artwork_error.h"
#include "art_io/measure.h"

using namespace Qt::Literals::StringLiterals;

namespace PxCryptPrivate
{

class IArtwork
{
//-Class Aliases--------------------------------------------------------------------------------------------------------
public:
    using rendition_id_t = quint16;

//-Class Variables--------------------------------------------------------------------------------------------------------
protected:
    static constexpr QDataStream::Version STREAM_VER = QDataStream::Qt_6_0; // Shouldn't matter for this, but just in case
    static inline const QByteArray MAGIC_NUM = "PXC"_ba;

//-Constructor---------------------------------------------------------------------------------------------------------
protected:
    IArtwork();

//-Class Functions----------------------------------------------------------------------------------------------
public:
    static quint64 size(quint64 renditionSize);

//-Instance Functions----------------------------------------------------------------------------------------------
protected:
    virtual quint64 renditionSize() const = 0;

public:
    quint64 size() const;
};

template<typename DerivedT, IArtwork::rendition_id_t RenditionId>
class Artwork : IArtwork
{
//-Class Variables----------------------------------------------------------------------------------------------------------
public:
    static constexpr quint16 RENDITION_ID = RenditionId;

//-Constructor-------------------------------------------------------------------------------------------------------------
protected:
    Artwork()
    {
        // Check requirement here as workaround to delay evaluation of incomplete type DerivedT
        static_assert(measureable<DerivedT>);
    };

//-Class Functions----------------------------------------------------------------------------------------------
public:
    static ArtworkError readFromCanvas(DerivedT& art, Canvas& canvas)
    {
        // Null out return buffer
        art = DerivedT();

        // Setup stream
        QDataStream canvasStream(&canvas);
        canvasStream.setVersion(STREAM_VER);

        // Read magic
        QByteArray magic(MAGIC_NUM.size(), Qt::Uninitialized);
        canvasStream.readRawData(magic.data(), MAGIC_NUM.size());
        if(magic != MAGIC_NUM)
            return ArtworkError(ArtworkError::NotMagic);

        // Read rendition ID
        quint16 renditionId; canvasStream >> renditionId;
        if(renditionId != RENDITION_ID)
            return ArtworkError(ArtworkError::WrongCodec, u"0x%1 instead of 0x%2."_s.arg(renditionId, 2, 16, QChar(u'0'))
                                                                                    .arg(RENDITION_ID, 2, 16, QChar(u'0')));

        // Setup new artwork
        DerivedT readArt;
        Artwork& readArtBase = static_cast<Artwork&>(readArt); // TODO: WTF is this cast for???

        // Read rendition portion
        ArtworkError renditionError = readArtBase.renditionRead(canvasStream);

        // Check stream
        QDataStream::Status ss = canvasStream.status();
        if(ss != QDataStream::Ok)
            return ArtworkError(ArtworkError::DataStreamError, ENUM_NAME(ss));

        // Check for rendition error
        if(renditionError)
            return renditionError;

        // Return art
        art = readArt;
        return ArtworkError();
    }

//-Instance Functions----------------------------------------------------------------------------------------------
protected:
    virtual ArtworkError renditionRead(QDataStream& stream) = 0;
    virtual ArtworkError renditionWrite(QDataStream& stream) const = 0;

public:
    ArtworkError writeToCanvas(Canvas& canvas)
    {
        // Setup stream
        QDataStream canvasStream(&canvas);
        canvasStream.setVersion(STREAM_VER);

        // Write magic and rendition ID
        canvasStream.writeRawData(MAGIC_NUM.constData(), MAGIC_NUM.size());
        canvasStream << RENDITION_ID;

        // Write rendition portion
        ArtworkError renditionError = renditionWrite(canvasStream);

        // Check stream
        QDataStream::Status ss = canvasStream.status();
        if(ss != QDataStream::Ok)
            return ArtworkError(ArtworkError::DataStreamError, ENUM_NAME(ss));

        return renditionError;
    }
};

}

#endif // ARTWORK_H
