#ifndef ARTWORK_ERROR_H
#define ARTWORK_ERROR_H

// Qt Includes
#include <QString>
#include <QHash>

using namespace Qt::Literals::StringLiterals;

namespace PxCryptPrivate
{

class ArtworkError
{
//-Class Enums-------------------------------------------------------------
public:
    enum Type
    {
        NoError,
        NotMagic,
        WrongCodec,
        DataStreamError,
        IntegrityError
    };

//-Class Variables-------------------------------------------------------------
private:
    static inline const QHash<Type, QString> ERR_STRINGS{
        {NoError, u""_s},
        {NotMagic, u"Image is not a magic image (missing magic number)."_s},
        {WrongCodec, u"Image created with a different codec than specified (mismatched rendition)."_s},
        {DataStreamError, u"An error occurred while streaming data from/to the canvas."_s},
        {IntegrityError, u"A data integrity check failed while reading data."_s}
    };

//-Instance Variables-------------------------------------------------------------
private:
    Type mType;
    QString mDetails;

//-Constructor-------------------------------------------------------------
public:
    ArtworkError(Type t = NoError, const QString& d = {});

//-Instance Functions-------------------------------------------------------------
public:
    bool isValid() const;
    Type type() const;
    QString details() const;

//-Operators-------------------------------------------------------------
public:
    operator bool() const { return isValid(); }
};

}

#endif // ARTWORK_ERROR_H
