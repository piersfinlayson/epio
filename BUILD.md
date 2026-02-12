# Build

See [requirements](#requirements) for the tools you need to build `epio` and the example firmware.

## Build

### libepio

From the root of the repository:

```bash
make
```

The built library will be at `build/libepio.a`.

### Examples

See [the example README](example/README.md).

## Requirements

### Compilation

- make
- gcc
- cmake (tests: cmocka)
- emscripten (wasm only)
- python3 (wasm only)
- arm-none-eabi-gcc (example firmware build only)

#### Linux

```bash
sudo apt-get update
sudo apt-get install -y build-essential gcc-arm-none-eabi emscripten python3
```

#### macOS

```bash
brew install make gcc arm-none-eabi-gcc emscripten python3
```

### Documentation

- doxygen

#### Linux

```bash
sudo apt-get update
sudo apt-get install -y doxygen
```

#### macOS

```bash
brew install doxygen
```

