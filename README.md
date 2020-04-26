# portapack-havoc-modified
I added gps simulator and analog tv demodulator (mainly PAL).
I also implemented Bluetooth receiver.
The bluetooth receiver can show you mac address of BLE devices around you.
However, the result isn't always correct.
Because I have only filtered with preamble and address, without crc.
If I use crc, no result will be shown, so I have to bypass crc test.
I am thinking some radio parameter can be improved to reduce the noise, so that more result can pass crc.

I improved little bit for btle.
Now it can pass crc, however the mac passed crc is wrong. 

I added nrf24l01 demodulation, tested with crazyradio pa. Only 250KPS mode is supported for now.

I improved pocsag tramitter and receiver. Now they can be switched to Positive or Negative phase.
