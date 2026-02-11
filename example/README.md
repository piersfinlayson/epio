# epio Example

This example demonstrates how to use `epio` to build and run the same PIO program on a real RP2350 and emulated in a hosted environment.

It toggles GPIO0 at around 30Hz.

The emulated version logs the programmed PIO configuration, steps the PIOs and checks the state of the GPIO is as expected at key points.

See [files](#files) below for the example's source code.

## Hosted (Emulated)

To build and run the emulated version, from the root of the repository:

```bash
make run-hosted-example
```

## WASM

To build the WASM version, from the root of the repository:

```bash
make run-wasm-example
```

## Firmware

### Build

To build the firmware, from the root of the repository:

```bash
make -f example/firmware.mk
```

If the arm-none-eabi toolchain is not in `/usr/bin`, set `TOOLCHAIN`:

```bash
export TOOLCHAIN=/path/to/arm-none-eabi-toolchain
make -f example/firmware.mk
```

You can download the toolchain from [ARM](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) or install it using your package manager, e.g. `apt install gcc-arm-none-eabi`.

### Run

Use either `build/example.elf` or `build/example.bin` to flash the RP2350.

If you have `picotool` installed, you can build and flash using the UF2:

```bash
make -f example/firmware.mk flash
```

## Logging

The firmware provides RTT trace which can be viewed using a debug (SWD) probe.  For example, using `probe-rs` to flash and connect to the RTT:

```bash
probe-rs run --chip rp2350 build/example.elf
```

The emulated build logs to stdout, so you can just run the binary:

```bash
./build/example
```

## Files

The example program is made up of the following files:

- [`firmware_main.c`](firmware_main.c) - Example RP2350 code using `apio` to build and run a simple PIO program.
- [`hosted_main.c`](hosted_main.c) - Example hosted code using `epio` to emulate the PIO from `firmware_main.c`, and check PIO operation.
- [`wasm_main.c`](wasm_main.c) - Example WASM code using `epio` to emulate the PIO from `firmware_main.c`, and check PIO operation.
- [`include.h`](include.h) - Common includes and definitions for the firmware and hosted examples.
- [`index.html`](index.html) - WASM example HTML page.
- [`vector.c`](vector.c) - Firmware reset vectors.
- [`boot.c`](boot.c) - Firmware boot block.
- [`linker.ld`](linker.ld) - Firmware Llinker script.
- [`firmware.mk`](firmware.mk) - Makefile for building the firmware version.
- [`hosted.mk`](hosted.mk) - Makefile for building the hosted version.
- [`wasm.mk`](wasm.mk) - Makefile for building the WASM version.
