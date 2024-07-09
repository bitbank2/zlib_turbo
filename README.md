zlib_turbo<br>
-----------------------------------
Copyright (c) 2024 BitBank Software, Inc.<br>
Written by Larry Bank (bitbank@pobox.com)<br>
Portions copyright (c) 1995-2024 Mark Adler and Jean-loup Gailly
<br>
**I optimize other people's code for a living. This library is a good example of the kind of work I do for my commercial clients; it contains many unique and clever optimizations that allows it to perform better than anything else available. I'm happy to contribute optimized libraries to the open source community in addition to working on commercial projects. Whatever platform you're using, I can make significant improvements to your native code. Please contact me so that I can show you how.**<br>
<br>
<b>What is it?</b><br>
A library for decompressing deflated and gzip'd data.<br>

<b>Why did you write it?</b><br>
I've been writing imaging codecs for many years and I like to write 100% of the code for control of the design and to optimize performance. For PNG images, the compression is based on deflate (zlib). The zlib library is somewhat challenging to recreate, so I decided to try to strip it down to the minimum code necessary to decompress the data. I also optimized it a bit. Part of the performance of the original zlib is hurt due to all of the 'streaming' logic that can work with input and output data 1 byte at a time. My version requires the complete output buffer to be present. The C++ wrapper functions are atomic (all data in -> all data out), but with the C code you can still pass it the input data in multiple passes. My code doesn't have any external dependencies and is a single C file of about 700 lines. One caveat is that it uses unaligned reads and writes to accelerate the decoding, so the input buffer and output buffer need to be allocated 4-8 bytes larger than needed (32/64-bit systems) to allow for reads/writes past the end.<br>

Features:<br>
---------<br>
- Supports any MCU with at least 6.5K of RAM (Cortex-M4 is the simplest I've tested)
- Optimized for speed and simplicity.
- Generic C code with a C++ wrapper and no external dependencies (not even malloc).
- 50-100% faster than zlib for all jobs
- Easy gzip API too

If you find this code useful, please consider becoming a sponsor or sending a donation.

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=SR4F44J2UR8S4)

