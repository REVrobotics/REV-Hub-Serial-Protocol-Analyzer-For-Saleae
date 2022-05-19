# REV Hub Serial Protocol Analyzer
  
## Usage
This analyzer has only been tested with the UART version of the protocol, not the RS-485 version. However, the RS-485 version should work fine as well. See https://support.saleae.com/protocol-analyzers/analyzer-user-guides/using-async-serial/decode-rs-232-rs-485-and-rs-422.

1. Clone this repository locally
2. Open the Logic 2 Extensions panel, click the "three-dots" menu icon, select "Load Existing Extension..." and select
   the extension.json file from the local clone of this repository
3. Set up an Async Serial analyzer with default settings (or two if you want to analyze traffic in both directions when using UART)
4. Set up a REV Hub Serial Protocol analyzer for each Async Serial analyzer (for firmware 1.8.2, the DEKAInterfaceFirstId should be set to 4096)
