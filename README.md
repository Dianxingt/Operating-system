# The project is intended to build a operating system like Unix. Written in C language and x86.

The features we finished:

Process (PCB) and its scheduling based on RR algorithm (Time slicing).

Three Terminals and their switching, including Backup Video Memory support.

Intel Procotol Interrupt descriptior table, including interrupts, exceptions and system calls. And the Main feature Privilege Switching (Kernel and User)

File System: continous storage of file. Root blocks, inode blocks, content blocks.

Signal systems, this is like User level interrupt for user to trigger kernel-level functions. This sounds like system calls, but interrupt is pretty useful
for its timeliness.

Two levels of Page frames for Mapping virtual memory to physical memory.

