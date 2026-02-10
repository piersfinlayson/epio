// MIT License
//
// Copyright (c) 2026 Piers Finlayson <piers@piers.rocks>
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

// RP2350 boot block

// Boot block structure
typedef struct {
    unsigned long  start_marker;          // 0xffffded3, start marker
    unsigned char  image_type_tag;        // 0x42, image type
    unsigned char  image_type_len;        // 0x1, item is one word in size
    unsigned short image_type_data;       // 0b0001000000100001, RP2350, ARM, Secure, EXE
    unsigned char  type;                  // 0xff, size type, last item
    unsigned short size;                  // 0x0001, size
    unsigned char  pad;                   // 0
    unsigned long  next_block;            // 0, link to self, no next block
    unsigned long  end_marker;            // 0xab123579, end marker
} __attribute__((packed)) rp2350_boot_block_t;

// Boot block
__attribute__((section(".rp2350_boot_block")))
const rp2350_boot_block_t rp2350_arm_boot_block = {
    .start_marker    = 0xffffded3,
    .image_type_tag  = 0x42,
    .image_type_len  = 0x1,
    .image_type_data = 0b0001000000100001,
    .type            = 0xff,
    .size            = 0x0001,
    .pad             = 0,
    .next_block      = 0,
    .end_marker      = 0xab123579
};
