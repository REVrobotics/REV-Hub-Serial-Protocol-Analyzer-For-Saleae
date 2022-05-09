#ifndef SERIAL_ANALYZER_H
#define SERIAL_ANALYZER_H

#include <Analyzer.h>
#include "SerialAnalyzerResults.h"
#include "SerialSimulationDataGenerator.h"

class SerialAnalyzerSettings;
class SerialAnalyzer : public Analyzer2
{
  public:
    SerialAnalyzer();
    virtual ~SerialAnalyzer();
    virtual void SetupResults();
    virtual void WorkerThread();

    virtual U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );
    virtual U32 GetMinimumSampleRateHz();

    virtual const char* GetAnalyzerName() const;
    virtual bool NeedsRerun();


#pragma warning( push )
#pragma warning(                                                                                                                           \
    disable : 4251 ) // warning C4251: 'SerialAnalyzer::<...>' : class <...> needs to have dll-interface to be used by clients of class

  protected: // functions
    U64 SerialAnalyzer::ReadBytes(U64 num_bytes);

  protected: // vars
    std::auto_ptr<SerialAnalyzerSettings> mSettings;
    std::auto_ptr<SerialAnalyzerResults> mResults;
    AnalyzerChannelData* mSerial;

    SerialSimulationDataGenerator mSimulationDataGenerator;
    bool mSimulationInitilized;

    // Serial analysis vars:
    U32 mSampleRateHz;
    U32 mSamplesPerBit;
    BitState mBitLow;
    BitState mBitHigh;

#pragma warning( pop )
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer();
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );

#endif // SERIAL_ANALYZER_H
