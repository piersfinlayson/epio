Additional function:
- DMA

PIO instructions to test:
- PUSH/PULL
- MOV
- IRQ
- SET

Test epio_wait_tx_fifo() - won't be triggered by a PIO, would be triggered by some other program (or DMA channel) pushing to the TX FIFO.  But there's an argument for a bunch of integration testing.