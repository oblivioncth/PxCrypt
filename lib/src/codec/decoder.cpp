// Unit Includes
#include "pxcrypt/codec/decoder.h"
#include "codec/decoder_p.h"

namespace PxCrypt
{

/*! @cond */

//===============================================================================================================
// DecoderPrivate
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------
//Protected:
DecoderPrivate::DecoderPrivate() :
    mPsk()
{}

//-Destructor---------------------------------------------------------------------------------------------------
//Public:
DecoderPrivate::~DecoderPrivate() {};

/*! @endcond */

//===============================================================================================================
// Decoder
//===============================================================================================================

/*!
 *  @class Decoder <pxcrypt/codec/decoder.h>
 *
 *  @brief The Encoder class is a base class from which all PxCrypt decoders derive in order to
 *  skim and decrypt data from the color channels of an encoded image.
 */

//-Constructor---------------------------------------------------------------------------------------------------
//Protected:
/*! @cond */
Decoder::Decoder(std::unique_ptr<DecoderPrivate> d) : d_ptr(std::move(d)) {}
/*! @endcond */

//-Destructor---------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Destroys the decoder.
 */
Decoder::~Decoder() {}
// Definition here is required so that private implementation is visible from the same TU when this is deleted

//-Instance Functions----------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns the key the encoder is configured to use for data scrambling.
 *
 *  @sa setPresharedKey().
 */
QByteArray Decoder::presharedKey() const { Q_D(const Decoder); return d->mPsk; }

/*!
 *  Sets key used for scrambling the encoding sequence to @a key.
 *
 *  This key will be required in order to decode the resultant encoded data. An empty string results in the use
 *  of a known/default encoding sequence.
 *
 *  @sa presharedKey().
 */
void Decoder::setPresharedKey(const QByteArray& key) { Q_D(Decoder); d->mPsk = key;}

}
