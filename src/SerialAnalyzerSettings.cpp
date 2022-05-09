#include "SerialAnalyzerSettings.h"

#include <AnalyzerHelpers.h>
#include <sstream>
#include <cstring>

#pragma warning( disable : 4800 ) // warning C4800: 'U32' : forcing value to bool 'true' or 'false' (performance warning)

SerialAnalyzerSettings::SerialAnalyzerSettings()
    : mInputChannel( UNDEFINED_CHANNEL ),
      mBitRate( 460800 )
{
    mInputChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
    mInputChannelInterface->SetTitleAndTooltip( "Input Channel", "REV Hub protocol" );
    mInputChannelInterface->SetChannel( mInputChannel );

    mBitRateInterface.reset( new AnalyzerSettingInterfaceInteger() );
    mBitRateInterface->SetTitleAndTooltip( "Bit Rate (Bits/s)", "Specify the bit rate in bits per second." );
    mBitRateInterface->SetMax( 100000000 );
    mBitRateInterface->SetMin( 1 );
    mBitRateInterface->SetInteger( mBitRate );

    AddInterface( mInputChannelInterface.get() );
    AddInterface( mBitRateInterface.get() );

    // AddExportOption( 0, "Export as text/csv file", "text (*.txt);;csv (*.csv)" );
    AddExportOption( 0, "Export as text/csv file" );
    AddExportExtension( 0, "text", "txt" );
    AddExportExtension( 0, "csv", "csv" );

    ClearChannels();
    AddChannel( mInputChannel, "Serial", false );
}

SerialAnalyzerSettings::~SerialAnalyzerSettings()
{
}

bool SerialAnalyzerSettings::SetSettingsFromInterfaces()
{
    mInputChannel = mInputChannelInterface->GetChannel();
    mBitRate = mBitRateInterface->GetInteger();
    ClearChannels();
    AddChannel( mInputChannel, "Serial", true );

    return true;
}

void SerialAnalyzerSettings::UpdateInterfacesFromSettings()
{
    mInputChannelInterface->SetChannel( mInputChannel );
    mBitRateInterface->SetInteger( mBitRate );
}

void SerialAnalyzerSettings::LoadSettings( const char* settings )
{
    SimpleArchive text_archive;
    text_archive.SetString( settings );

    const char* name_string; // the first thing in the archive is the name of the protocol analyzer that the data belongs to.
    text_archive >> &name_string;
    if( strcmp( name_string, "SaleaeAsyncSerialAnalyzer" ) != 0 )
        AnalyzerHelpers::Assert( "SaleaeAsyncSerialAnalyzer: Provided with a settings string that doesn't belong to us;" );

    text_archive >> mInputChannel;
    text_archive >> mBitRate;
   
    ClearChannels();
    AddChannel( mInputChannel, "Serial", true );

    UpdateInterfacesFromSettings();
}

const char* SerialAnalyzerSettings::SaveSettings()
{
    SimpleArchive text_archive;

    text_archive << "SaleaeAsyncSerialAnalyzer";
    text_archive << mInputChannel;
    text_archive << mBitRate;
    return SetReturnString( text_archive.GetString() );
}
