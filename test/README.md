# Tests

This directory contains comprehensive unit tests for epio, including coverage of the APIs and operation of the supported PIO instructions.  This also serves as tests for the apio (runtime C PIO assembler) which is primarily used to construct PIO programs for testing.  


## To Do

Additional function:
- DMA

PIO instructions to test:
- PUSH/PULL
- MOV - make sure I cover pins wrapping (should be mod 32, then GPIOBASE applied), for both SRC, DST and PINs and PINDIRS
- IRQ
- SET


Holes to plug:
- Test cases for pins wrapping mod 32, IN and MOV

Test epio_wait_tx_fifo() - won't be triggered by a PIO, would be triggered by some other program (or DMA channel) pushing to the TX FIFO.  But there's an argument for a bunch of integration testing.

Validate autopush and autopull 11.5.4.1 and 11.5.4.2 from the datasheet again tests and behaviour.

Uncomment the IRQ test that require knowledge of what happens when set/clear for the same IRQ are issues in the same cycle.  See https://github.com/raspberrypi/documentation/issues/4281