#include "SerialAnalyzer.h"
#include "SerialAnalyzerSettings.h"
#include <AnalyzerChannelData.h>


SerialAnalyzer::SerialAnalyzer() : Analyzer2(), mSettings( new SerialAnalyzerSettings() ), mSimulationInitilized( false )
{
    SetAnalyzerSettings( mSettings.get() );
    UseFrameV2();
}

SerialAnalyzer::~SerialAnalyzer()
{
    KillThread();
}

void SerialAnalyzer::SetupResults()
{
    // Unlike the worker thread, this function is called from the GUI thread
    // we need to reset the Results object here because it is exposed for direct access by the GUI, and it can't be deleted from the
    // WorkerThread

    mResults.reset( new SerialAnalyzerResults( this, mSettings.get() ) );
    SetAnalyzerResults( mResults.get() );
    mResults->AddChannelBubblesWillAppearOn( mSettings->mInputChannel );
}

void SerialAnalyzer::WorkerThread()
{
    mSampleRateHz = GetSampleRate();

    mBitHigh = BIT_HIGH;
    mBitLow = BIT_LOW;

    mSerial = GetAnalyzerChannelData( mSettings->mInputChannel );

    if( mSerial->GetBitState() == mBitLow )
        mSerial->AdvanceToNextEdge();

    // These two lines copied from https://github.com/saleae/SampleAnalyzer/blob/85d92300c0172c63f28e505fa53122d14c57c207/src/SimpleSerialAnalyzer.cpp#L34-L35
    mSamplesPerBit = mSampleRateHz / mSettings->mBitRate;
    U32 samples_to_first_center_of_first_data_bit = U32( 1.5 * double( mSampleRateHz ) / double( mSettings->mBitRate ) );

    for( ;; )
    {
        // we're starting high.  (we'll assume that we're not in the middle of a byte.

        mSerial->AdvanceToNextEdge();

        bool parity_error = false;
        bool framing_error = false;
        bool mp_is_address = false;

        mSerial->Advance( samples_to_first_center_of_first_data_bit );

        U8 first_byte = ReadBytes(1);
        U8 second_byte = ReadBytes(1);
        U16 length = ReadBytes(2);
        U16 payload_len = length - 11;

        U8 dest_addr = ReadBytes(1);
        U8 src_addr = ReadBytes(1);
        U8 msg_num = ReadBytes(1);
        U8 ref_num = ReadBytes(1);
        U16 packet_type = ReadBytes(2);
        for (U16 i = 0; i < payload_len; i++)
        {
            // Read one byte at a time, because more than 8 bytes won't fit into a U64
        }
        ReadBytes(payload_len);
        U8 check_val = ReadBytes(1);

        mResults->CommitResults();

        CheckIfThreadShouldExit();

        if( mSerial->GetBitState() == mBitLow )
            mSerial->AdvanceToNextEdge();
    }
}

U64 SerialAnalyzer::ReadBytes(U64 num_bytes)
{
    U64 data = 0;
    DataBuilder data_builder;
    data_builder.Reset( &data, AnalyzerEnums::LsbFirst, 8 * num_bytes );

    for( U64 i = 0; i < num_bytes; i++ )
    {
        U64 frame_starting_sample = mSerial->GetSampleNumber();
        U64 byte_data = 0;
        DataBuilder byte_data_builder;
        byte_data_builder.Reset( &data, AnalyzerEnums::LsbFirst, 8 );

        for ( U8 y = 0; i < 8; i++)
        {
            byte_data_builder.AddBit( mSerial->GetBitState() );
            data_builder.AddBit( mSerial->GetBitState() );

            //let's put a dot exactly where we sample this bit:
            mResults->AddMarker( mSerial->GetSampleNumber(), AnalyzerResults::Dot, mSettings->mInputChannel );

            mSerial->Advance( mSamplesPerBit );
        }

       // ok now record the value!
       // note that we're not using the mData2 or mType fields for anything, so we won't bother to set them.
       Frame frame;
       frame.mStartingSampleInclusive = frame_starting_sample;
       frame.mEndingSampleInclusive = mSerial->GetSampleNumber();
       frame.mData1 = byte_data;
       frame.mFlags = 0;

       mResults->AddFrame( frame );

       FrameV2 framev2;

       framev2.AddByte( "data", byte_data );
       mResults->AddFrameV2( framev2, "data", frame_starting_sample, mSerial->GetSampleNumber() );
       ReportProgress( frame.mEndingSampleInclusive );
    }
    return data;
}

bool SerialAnalyzer::NeedsRerun()
{
    return false;

    // ok, lets see if we should change the bit rate, base on mShortestActivePulse

    U64 shortest_pulse = mSerial->GetMinimumPulseWidthSoFar();

    if( shortest_pulse == 0 )
        AnalyzerHelpers::Assert( "Alg problem, shortest_pulse was 0" );

    U32 computed_bit_rate = U32( double( mSampleRateHz ) / double( shortest_pulse ) );

    if( computed_bit_rate > mSampleRateHz )
        AnalyzerHelpers::Assert( "Alg problem, computed_bit_rate is higer than sample rate" ); // just checking the obvious...

    if( computed_bit_rate > ( mSampleRateHz / 4 ) )
        return false; // the baud rate is too fast.
    if( computed_bit_rate == 0 )
    {
        // bad result, this is not good data, don't bother to re-run.
        return false;
    }

    U32 specified_bit_rate = mSettings->mBitRate;

    double error = double( AnalyzerHelpers::Diff32( computed_bit_rate, specified_bit_rate ) ) / double( specified_bit_rate );

    if( error > 0.1 )
    {
        mSettings->mBitRate = computed_bit_rate;
        mSettings->UpdateInterfacesFromSettings();
        return true;
    }
    else
    {
        return false;
    }
}

U32 SerialAnalyzer::GenerateSimulationData( U64 minimum_sample_index, U32 device_sample_rate,
                                            SimulationChannelDescriptor** simulation_channels )
{
    if( mSimulationInitilized == false )
    {
        mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), mSettings.get() );
        mSimulationInitilized = true;
    }

    return mSimulationDataGenerator.GenerateSimulationData( minimum_sample_index, device_sample_rate, simulation_channels );
}

U32 SerialAnalyzer::GetMinimumSampleRateHz()
{
    return mSettings->mBitRate * 4;
}

const char* SerialAnalyzer::GetAnalyzerName() const
{
    return "REV Hub Protocol (RS-485/UART)";
}

const char* GetAnalyzerName()
{
    return "REV Hub Protocol (RS-485/UART)";
}

Analyzer* CreateAnalyzer()
{
    return new SerialAnalyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
    delete analyzer;
}
