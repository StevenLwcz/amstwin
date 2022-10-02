#!/usr/bin/python3

import time, sys

def change_colour(c, r, g, b):
    sys.stdout.write(f"\033]4;{c};rgb:{r:02x}/{g:02x}/{b:02x}\x1b\\")
    sys.stdout.flush()

change_colour(1, 0, 255, 0)      # strings and {, &&, etc
change_colour(2, 0, 255, 50)     # types (int, char)
change_colour(5, 0, 255, 200)    # numeric literals
change_colour(6, 0, 255, 200)    # comments
change_colour(12, 0, 255, 150)   # keywords
# change_colour(14, 0, 255, 100)   # border
# change_colour(255, 0, 255, 100)   # variables
# change_colour(255, 0, 255, 100)   # functions
# change_colour(255, 0, 255, 100)   # line numbers


# for i in range(0, 256):
    # print(f"{i} ", end='')
    # change_colour(i, 0, 255, 255)  # nothing
