# Linux/Qt-Adaptation of the ADS/C++ sample #16: Dynamic retrieval of ADS type information

This is based on the code from [TwinCAT 3 ADS-DLL C++: Sample 16](https://infosys.beckhoff.com/content/1033/tc3_adsdll2/124833547.html?id=2564205560515071521) and can be used with the official [Beckhoff ADS lib](https://github.com/Beckhoff/ADS).

Copy this repository next to the `example` directory and follow the build instructions in their `README.md`.

You'll need Qt 6 (developed with 6.8.2) including the `Core5Compat` module (for decoding Windows-1252 character sets)

The UI is extremely spartanic (similar to the original) and hardly useful on larger PLCs. I have not thoroughly tested whether all the navigation (parent, sibling, child, next) really always works as intended - it's the same as in the original example.

This repo is primarily useful as a reference for how to access and parse the ADS data-type information.

## Extensions

There are a few additional features in this version:
- target IP, net ID and port are specified on the command line or can be set as environment variables:
  - `ADS_TARGET_IP`, e.g. `192.168.1.100`
  - `ADS_TARGET_NETID`, e.g. `192.168.1.100.1.1`
  - `ADS_TARGET_PORT` optional defaults to 851
- the retrieved symbol/data-type schema is locally cached in a `symbol.cache` file, so you can save the PLC interaction after the first run; if you want to refresh the data, remove the cache file
- the parsed type-information is dumped to the `datatypes.json` file for reference

