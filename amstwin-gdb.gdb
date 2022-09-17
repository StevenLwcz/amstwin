tty /dev/pts/1
so ../gdb-python-blog/auto.py
tui new-layout debug1 auto 1 src 2 status 0 cmd 1
layout debug1
b main
b print_wrap
r
