# Queue-Stack-OS
C program that simulates the allocation of processes in a processor, and running each process given a time constant

This is a project for univeristy, it simulates the processor and how it allocates memory for each process. This project also runs each process. Each process is added to a active queue. The first element in the queue is run for T miliseconds. After it is done, it is reintroduced in active queue, or removed from active queue if the its time is over.

Each process has a address to which it is allocated. On the hard disk, each process has a stack of information.
If too many processes are allocated, then a defragmentation is called, which reorganizes the processes in such a way that every bit of memory fits perfectly in the maximum of 4mb memory that is given.
