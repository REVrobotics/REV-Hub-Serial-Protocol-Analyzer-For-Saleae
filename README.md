# REV Hub Serial Protocol Analyzer
  
## Usage
This analyzer has only been tested with the UART version of the protocol, not the RS-485 version. However, the RS-485 version should work fine as well. See Saleae's [Decode RS-232, RS-485, & RS-422](https://support.saleae.com/protocol-analyzers/analyzer-user-guides/using-async-serial/decode-rs-232-rs-485-and-rs-422) page.

1. Set up an Async Serial analyzer (or two if you want to analyze traffic in both directions when using UART) at 460800 baud, with otherwise default settings
2. Set up a REV Hub Serial Protocol analyzer for each Async Serial analyzer (for firmware 1.8.2, the DEKAInterfaceFirstId should be set to 4096)
