#define OS_PID       0

#define	STDIN        0
#define STDOUT       1
#define STDERR       2

#define FD_PIC       5

#define FD_VIDEO     10
#define FD_KEYBOARD  11

#define FD_TTY0      20
#define FD_TTY1      21
#define FD_TTY2      22
#define FD_TTY3      23
#define FD_TTY4      24
#define FD_TTY5      25

#define FD_FIFO      100

#define FD_FILE      1000

// Syscalls based on http://docs.cs.up.ac.za/programming/asm/derick_tut/syscalls.html
#define READ         3
#define WRITE        4
#define OPEN         5
#define CLOSE        6
#define MKFIFO       7
#define PCREATE      8
#define PRUN         9
#define PDUP2        10
#define GETPID       11
#define WAITPID      12
#define PTICKS       13
#define PNAME        14
#define PSTATUS      15
#define PPRIORITY    16
#define PGID         17
#define PGETPID_AT   18
#define KILL         19


#define SYSR_ERR     -1
#define SYSR_BLOCK   -2

#define KERNEL_BUFFER_SIZE 16
#define	KERNEL_RETURN      (KERNEL_BUFFER_SIZE - 1)

#ifndef kernel_buffer
int kernel_buffer[KERNEL_BUFFER_SIZE];
#endif