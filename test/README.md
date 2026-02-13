# Tests

This directory contains comprehensive unit tests for epio, including coverage of the APIs and operation of the supported PIO instructions.  This also serves as tests for the apio (runtime C PIO assembler) which is primarily used to construct PIO programs for testing.

100% of reachable code in the main state machine implementation, [`epio_exec.c`](/src/epio_exec.c), is covered by these tests, and the tests are designed to cover all the edge cases of the supported PIO instructions.  The tests are implemented using the cmocka unit testing framework, and can be run on a host machine without any special hardware requirements.

From the root of the repository, the tests can be run using:

```bash
make test
```

## To Do

Test epio_wait_tx_fifo() - won't be triggered by a PIO, would be triggered by some other program (or DMA channel) pushing to the TX FIFO.  But there's an argument for a bunch of integration testing.

Validate autopush and autopull 11.5.4.1 and 11.5.4.2 from the datasheet again tests and behaviour.

Fix the behaviour and then uncomment the IRQ test that require knowledge of what happens when set/clear for the same IRQ are issues in the same cycle.  See https://github.com/raspberrypi/documentation/issues/4281

## Code Coverage

To generate code coverage reports for the tests, you can use the following commands after running `make test`:

```bash
lcov --capture --directory . --output-file /tmp/coverage.info
genhtml /tmp/coverage.info --output-directory /tmp/coverage_html
open /tmp/coverage_html/index.html  # macOS specific, adjust for your OS
```
