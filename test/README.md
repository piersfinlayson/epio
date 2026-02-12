To test:

Function:
GPIOs

PIO instructions:
- PUSH/PULL
- MOV
- IRQ
- SET

Test epio_wait_tx_fifo() - won't be triggered by a PIO, would be triggered by some other program (or DMA channel) pushing to the TX FIFO.