// Copyright (C) 2026 Piers Finlayson <piers@piers.rocks>
//
// MIT License

// epio - A PIO emulator
//
// epio WASM exports

#ifndef EPIO_WASM_H
#define EPIO_WASM_H

#ifdef EPIO_WASM
#include <emscripten.h>
#define EPIO_EXPORT EMSCRIPTEN_KEEPALIVE
#else
#define EPIO_EXPORT
#endif

#endif // EPIO_WASM_H