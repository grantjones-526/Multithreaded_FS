Page
Page number
1
of 6
CS 445/445G Project 1: A Multithreaded File System (100 Points)
(due at the midnight on 3/31/26)
This project is a team project, and each team can have up to 2 students. In this project, you will
implement a user-space file system simulator that models core OS-level file operations. The goal is
to understand file management from the system’s perspective by designing and manipulating kernel-
like data structures (e.g., directory structures, open file tables, file control blocks) and enforcing correct
behaviors under concurrent access.
1 Assumptions and Basic Data structures
Before we describe the tasks of this project, we make the following assumptions. The simulated file
system is implemented on main memory. More specifically, a slot of main memory of 1M is used to
simulate a disk. We assume that a data block is of 2K. Therefore, the disk has 512 blocks. In this
project, we assume the disk is a data disk, i.e., no OS installed. So the first data block is used as the
volume control block. We assume that the volume control block contains the following items:
number of blocks
size of block
a free-block count
a bit-map of free blocks
Table 1: Volume control block
We assume that the file allocation (i.e., data block allocation) is contiguous allocation. For simplicity,
our file system uses a flat directory structure to manage files. In other words, the directory contains no
subdirectories. With the assumption of contiguous allocations, the content of the directory of our file
system can be organized as a table as follows:
file name start block number file size
file1 0 2
file3 6 3
... ... ...
Table 2: Flat directory structure.
To be consistent with the contiguous allocation, the FCB contains the following items:
file size
pointer to the first data block
Table 3: File control block (FCB).
A system-wide open file table is a set of FCBs of open files. An example of a system-wide open file
table is shown in Table 4.
1
file name FCB
f1 FCB1
f3 FCB3
f4 FCB4
Table 4: System-wide open file table.
Given a specific process, its per-process open file table contains a set of file handles of files opened
by the process. For example, assume that a process has open files: f1 and f4, its per-process table will
be the one shown in Table 5.
file name handle
f1 0
f4 2
Table 5: Per-process open file table.
2 Designs (20 Points)
The following is the collection of file operations that the file system should offer:
• create()
• open()
• close()
• read()
• write()
CS 445G graduate students should consider two additional file operations:
• rename()
• delete()
For each above-mentioned file operation, perform designs to clearly address the following aspects:
• Function signatures
• Inputs
• Outputs, i.e., return value
• Data structures accessed (read/write)
• Fine-grained locking (mutex lock per shared data structures, including open count)
2
3 Implementation (70 Points)
You may use C/C++ or Java to implement this project. If Java is used, the simulation must explic-
itly demonstrate multi-threaded execution using Java threads and appropriate synchronization primitives
(e.g., synchronized, ReentrantLock). If C/C++ is used, POSIX threads (pthread) and mu-
texes should be utilized.
3.1 Required Components
Your implementation must include the following core components:
• Directory Structure: A data structure mapping file names to FCBs (also called inodes). You may
assume a single-level directory unless otherwise specified.
• File Control Block Table: Stores file metadata such as file size, data block pointers (or simulated
storage reference), and other necessary attributes.
• System-Wide Open File Table: Maintains entries for currently opened files, including:
– File offset
– Access mode
– Reference count (open count)
– Pointer to corresponding FCB
• Per-Thread Open File Table: Maps file descriptors to entries in the system-wide open file table.
• Simulated Storage: A byte array or block array representing disk storage. You must clearly
describe how data is allocated and stored.
3.2 Operation Semantics
Each file operation must:
• Follow the function signatures defined in your design section.
• Properly validate inputs and return appropriate error codes.
• Clearly update all relevant data structures.
• Maintain consistency invariants across directory, FCB, and open file tables.
For example:
• create() must allocate an FCB and insert a directory entry atomically.
3
3.3 Synchronization Requirements
The file system must be thread-safe.
You must implement fine-grained locking to protect shared data structures, including:
• Directory structure
• FCB table
• System-Wide open file table
• Open count (reference count)
• Free space or storage allocation structures
Each shared structure should have its own mutex lock. You must clearly document:
• Which locks are acquired for each file operation
• The order in which locks are acquired (to avoid deadlock)
• Why each lock is necessary
You must preserve the following invariants:
• A directory entry always references a valid FCB.
• An FCB cannot be freed while still referenced by the open file table.
• Open count updates are atomic.
• File offsets remain consistent under concurrent access.
3.4 Synchronization Demonstration
Your implementation must include a multi-threaded driver program designed to exercise concurrent
access to shared file system structures.
The goal of this driver is to demonstrate that your locking strategy correctly enforces mutual exclusion
and preserves system invariants under contention.
You must:
• Create multiple threads performing concurrent operations on the same file.
• Show that directory, FCB table, and open file table remain consistent.
• Explain what race conditions would occur if synchronization were removed.
4
4 Test Cases (10 Points)
Before stress-testing concurrency, you should first validate functional correctness in a race-free set-
ting. Design a deterministic, single-thread (or fully serialized) test suite where each test has a clear
expected outcome and checks invariants after every operation. Start with unit-style tests for individual
operations—e.g., create() adds exactly one directory entry and allocates a valid FCB; open() returns
a valid file descriptor and increments the open count of the open file table; write() increases file size
appropriately and persists data; and delete() removes the directory entry only when no open references
remain.
For each test, validate both return values and state by inspecting relevant structures (e.g., directory,
system-wide or per-thread open file tables). Include edge cases such as opening non-existent files and
deleting open files.
Once these tests pass reliably with a single thread, you can reuse them as a baseline for concur-
rency tests by running the same logical sequences under multiple threads and comparing the resulting
state/content against the race-free “golden” outputs.
CS 445G graduate students should further test the correctness of rename() and delete() operations.
On submission:
Submit your design report, source code and test report to blackboard. The design report and test re-
port may be merged into a single file. The source code should be well-documented, i.e., having good
readability.
Appendix: Pthread
If you are going to use C, you can read the following tutorial on the use of pthread library.
How to compile:
> cc file.c -lpthread
The above compilation command means the user code should be linked to pthread.lib. Without this
linking, the compiler will complain “undefined reference to” pthread functions, such as pthread create,
pthread join.
Sample code:
#include<pthread.h>
#include<stdio.h>
int sum; /* this data is shared by the threads */
void *runner(void *param); /* the thread */
int main(int argc, char *argv[])
5
{
pthread_t tid; /* the thread identifier */
pthread_attr_t attr; /* set of thread attributes */
if (argc != 2)
{
fprintf(stderr, "usage: ./a.out <positive integer>\n");
exit(0);
}
/* get default attribute */
pthread_attr_init(&attr);
/*create the thread */
pthread_create(&tid, &attr, runner, argv[1]);
printf("Main thread is busy doing something ...\n");
while (sum <= 1)
{
printf("%d ", sum);
}
/* wait for the thread to exit */
pthread_join(tid, NULL);
printf("sum = %d \n", sum);
}
void *runner(void *param)
{
int i, upper = atoi(param);
sum = 0;
for(i = 1; i <= upper; i++)
{
sum += i;
// printf("%d ", sum);
}
pthread_exit(0);
}
6