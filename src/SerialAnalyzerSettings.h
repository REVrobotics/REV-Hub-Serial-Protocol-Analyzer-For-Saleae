#ifndef SERIAL_ANALYZER_SETTINGS
#define SERIAL_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

class SerialAnalyzerSettings : public AnalyzerSettings
{
  public:
    SerialAnalyzerSettings();
    virtual ~SerialAnalyzerSettings();

    virtual bool SetSettingsFromInterfaces();
    void UpdateInterfacesFromSettings();
    virtual void LoadSettings( const char* settings );
    virtual const char* SaveSettings();


    Channel mInputChannel;
    U32 mBitRate;

  protected:
    std::auto_ptr<AnalyzerSettingInterfaceChannel> mInputChannelInterface;
    std::auto_ptr<AnalyzerSettingInterfaceInteger> mBitRateInterface;
};

#endif // SERIAL_ANALYZER_SETTINGS
