zlib_turbo<br>
-----------------------------------
Copyright (c) 2024 BitBank Software, Inc.<br>
Written by Larry Bank<br>
larry@bitbanksoftware.com<br>
<br>
**I optimize other people's code for a living. This library is a good example of the kind of work I do for my commercial clients; it contains many unique and clever optimizations that allows it to perform better than anything else available. I'm happy to contribute optimized libraries to the open source community in addition to working on commercial projects. Whatever platform you're using, I can make significant improvements to your native code. Please contact me so that I can show you how.**<br>
<br>
<b>What is it?</b><br>
A library for decompressing deflated and gzip'd data.<br>

<b>Why did you write it?</b><br>
I've been writing imaging codecs for many years and I like to write 100% of the code for control of the design and to optimize performance. For PNG images, the compression is based on deflate (zlib). This library is somewhat challenging to recreate, but I decided to try to strip it down to the minimum code necessary to decompress the data. Part of the performance is lost due to all of the 'streaming' logic that can work with input and output data 1 byte at a time.<br>

Features:<br>
---------<br>
- Supports any MCU with at least 6.5K of RAM (Cortex-M0+ is the simplest I've tested)
- Optimized for speed and simplicity.
- Generic C code with a C++ wrapper and no external dependencies (not even malloc).

If you find this code useful, please consider becoming a sponsor or sending a donation.

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=SR4F44J2UR8S4)

