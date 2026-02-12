# Tests

This directory contains comprehensive unit tests for epio, including coverage of the APIs and operation of the supported PIO instructions.  This also serves as tests for the apio (runtime C PIO assembler) which is primarily used to construct PIO programs for testing.  


## To Do

Additional function:
- DMA

PIO instructions to test:
- PUSH/PULL
- MOV
- IRQ
- SET

Test epio_wait_tx_fifo() - won't be triggered by a PIO, would be triggered by some other program (or DMA channel) pushing to the TX FIFO.  But there's an argument for a bunch of integration testing.