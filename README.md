# RSdB
> What is this?
- RSdB (rim shot dB) is a general usage dB meter with an OLED display, powered by the MEMS MSM261DGT003 and a Seeed XIAO RP2040.
> How do you use it?
- Plug straight into a generic USB-C power bank, and start measuring right away!
> Why did I make this?
- Primarily to not annoy my neighbours. It's nice knowing how loud I'm playing at any given moment so I know I need to chill down on the heavy hits. Secondarily, I didn't have any major experience with PDM microphones before, so I thought this would be a great place to start.

# Firmware
Firmware can be found at main/firmware.

The microphone I chose doesn't send traditional audio signals, instead it sends a single stream of ones and zeros at roughly 2 MHz. So one bit per clock pulse. The important part here is that stream having a bunch of repeating numbers after another.

If it sends a bunch of ones, then the sound pressure is high, and sends a bunch of zeros when it's low. The XIAO generates the clock, and reads the DATA pin at the same time. Once enough bits have accumulated, a decimation filter takes over, and it applies a low-pass sinc filter across every 128 bits and collapses them into a single 16-bit PCM sample, throwing away the ultrasonic noise the oversampling introduced and leaving just the audio band. This produces 16,000 audio samples per second.

From there, the firmware squares and accumulates every sample over a one-second window, computes the RMS amplitude (a measure of average signal energy that correctly handles the fact that audio oscillates both positive and negative), converts the result to dBFS using a base-10 logarithm, and then applies a sensitivity offset derived from the microphone's datasheet to produce an approximate reading in dB SPL. This calculation part was really tricky and I just couldn't figure out the math myself. Used AI for that part.

# PCB

<hr>

<img src="assets/rsdb_rev1.png" alt="3D render" width="800"/>

<img src="assets/pcb.png" alt="PCB" width="400"/>

<hr>

## BOM

<img src="assets/jlcpcb.png" alt="JLCPCB cart" width="600"/>

| Item | Quantity | Price | Funding required | Link |
| :---: | :---: | :---: | :---: | :---: |
| PCB | 1 | 16.26\* | Yes | [JLCPCB](https://jlcpcb.com/) |
| SSD1306 128×64 Mono 0.96 Inch I2C OLED Display | 1 | - | No | None |
| Seeed Studio XIAO RP2040 | 1 | - | No | None |

\* Note that price might fluctuate based on LCSC component prices, and the availability of coupons at the time of purchase. Includes PCBA.