# epio - An RP2350 PIO Emulator

While RP2350 PIO programs are conceptually simple and easy to write, testing their precise behaviour is hard:

- They operate at high speed, with limited visibility into their internal state.
- When modifying, it is difficult to prevent them from regressing without time consuming testing on real hardware.  This is particularly problematic for testing multiple hardware revisions and configuration.

`epio` solves both of these problems, by providing a cycle-accurate emulator for RP2350 PIO programs, that can run on non-RP2350 hosts, including CI runners.  This allows you to test and verify your PIO programs in a convenient and deterministic way, without needing to rely on real hardware.

It provides a `wasm` build, allowing you to run and visualise PIO programs in the browser, with a simple JavaScript API. 

To make it easy to setup, `epio` integrates with [`apio`](https://github.com/piersfinlayson/apio), a runtime PIO assembler and disassembler.  `apio` allows you to write your PIO programs with C macros directly in your RP2350 firmware (avoding the need for a separate `pioasm` step), and run them both on real RP2350 hardware and emulated using `epio` on non-RP2350 hosts. 

## Usage

See [the example](example/README.md) for a complete bare-metal RP2350 firmware with a PIO, using `apio` and `epio`.  This example can be built and run on both a real RP2350 and emulated on non-RP2350 hosts.  Here is a snippet that demonstrates how to use `epio` to run a PIO program assembled with `apio`:

```c
// Call the firmware main function, which returns instead of entering an
// infinite loop.  This firmware_main() function uses apio to set up a PIO
// program, in both the firmware and emulation builds.
firmware_main();

// Create an epio instance and configure it from the apio state.
epio_t *epio = epio_from_apio();

// Step the epio instance - cycling every enabled PIO SM exactly one cycle. 
epio_step_cycles(epio, 1);

// Check the state of the GPIOs
uint64_t gpio_states = epio_read_pin_states(epio);

// ...
```

## Features

- Cycle-accurate emulation of RP2350 PIO programs.
- Auto-configureing if used in conjunction with `apio`.
- Supports emulating all 12 PIO state machines running simultaneously.
- Single, multi-step modes, and run until supported conditions are met.
- Supports internal PIO IRQs.
- Supports GPIOBASE=0 and 16, and up to 48 GPIOs to support both RP2350A and B.
- Provides an SRAM API, so tests can simulate reading and writing to the RP2350's SRAM, based on PIO RX/TX FIFOs.
- WASM build, allowing you to run emulated firmware and visualise PIO programs and GPIO states in the browser.

## API

Start [here](https://piers.rocks/epio/topics.html).

## Requirements

In order to use `epio` for emulation, you can either use `apio` to set up your PIO programs, or manually configure the `epio` instance to match the state of your PIOs.

You must stub out any RP2350-specific non-PIO functionality that your program relies on when operating in the emulator, such as direct hardware register access, SDK usage, interrupts, etc.  Direct hardware register, SRAM, and other peripheral access will cause faults on the host.

## Limitations

There are currently some limitations in `epio`'s PIO emulation.  See the comment at the top of [`epio.c`](src/epio.c) for details.

If you need a feature that isn't implemented yet, please raise an issue or submit a PR.

## WASM

!!!

## License

MIT License, see [LICENSE](LICENSE).
