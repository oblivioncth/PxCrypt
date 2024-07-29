// Unit Include
#include "data_translator.h"

namespace PxCryptPrivate
{
/*! @cond */ //TODO: Doxygen bug, this should be needed because namespace is excluded
//===============================================================================================================
// DataTranslator
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------------
//Public:
DataTranslator::DataTranslator(PxAccess& access) :
    mAccess(access)
{}

//-Instance Functions--------------------------------------------------------------------------------------------
//Private:
template<typename F>
    requires Qx::defines_call_for_s<F, void, int, int>
bool DataTranslator::translate(F procedure)
{
    int byteBitIdx = 0;
    quint8 bpc = mAccess.bpc();

    while(byteBitIdx < 8 && !mAccess.atEnd())
    {
        // Determine how many bits can be used
        int remaining = 8 - byteBitIdx;
        int available = bpc - mAccess.bitIndex();
        int processing = std::min(remaining, available);

        // Perform procedure
        procedure(processing, byteBitIdx);

        // Update state
        byteBitIdx += processing;

        // Move access forward
        mAccess.advanceBits(processing);
    }

    return byteBitIdx == 8;
}

void DataTranslator::weaveBits(quint8 bits, int count)
{
    int chBitIdx = mAccess.bitIndex();

    // Align bits with destination start
    bits <<= chBitIdx;

    // Update bits
    quint8& val = mAccess.bufferedValue();
    if(mAccess.hasReferenceCanvas()) // Relative method
    {
        if(mAccess.originalValue() > 127)
            val -= bits;
        else
            val += bits;
    }
    else // Absolute method
    {
        quint8 clearMask = ~(((0b1 << count) - 1) << chBitIdx);
        val = (val & clearMask) | bits;
    }
}

quint8 DataTranslator::skimBits(int count)
{
    int chBitIdx = mAccess.bitIndex();
    quint8 keepMask = ((0b1 << count) - 1) << chBitIdx;

    quint8 bits;
    if(mAccess.hasReferenceCanvas()) // Relative method
        bits = Qx::distance(mAccess.referenceValue(), mAccess.constBufferedValue()) & keepMask;
    else // Absolute method
        bits = mAccess.constBufferedValue() & keepMask;

    // Drop already processed bits
    return bits >> chBitIdx;
}

//Public:
bool DataTranslator::weaveByte(quint8 byte)
{
    return translate([byte, this](int weaving, int alreadyWoven){
        // Extract bits
        quint8 extractMask = (1 << weaving) - 1;
        quint8 bits = (byte >> alreadyWoven) & extractMask;

        // Weave bits into canvas
        weaveBits(bits, weaving);
    });
}

bool DataTranslator::skimByte(quint8& byte)
{
    // Clear return buffer
    byte = 0;

    return translate([&byte, this](int skimming, int alreadySkimmed){
        // Skim bits
        quint8 bits = skimBits(skimming);

        // Merge into byte
        byte |= bits << alreadySkimmed;
    });
}

/*! @endcond */

}
