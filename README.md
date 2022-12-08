In my private repository I'm trying to write a BASIC compiler based on Amstrad 6128 BASIC. It is in Python and produces Arm 32 and 64 bit assembler. The project is about reliving some childhood memories, learn Python, Arm Assembler and anything needed to get this project working (LINUX system calls, floating point formats, more compiler theory, etc).

This repository contains a module (work in progress) amstwin to emulate the text windows in Amstrad 6128 which the compiler will use. I started using ncurses but the colour pair model turned out to be a pain so I though I would write my own windows module and learn all about programming for the terminal on LINUX as well as ANSI Escape Sequences.

amstkey (work in progress) is a module to emulate the various keyboard input keywords inkey, inkey$, input line, input.
