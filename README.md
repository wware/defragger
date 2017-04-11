Memory defragmentation, Veracode Hackathon 10-5/7
====

Since I first started working with Veracode's internal Buildbot installation, the compare step (where we compare the results of a baseline run to the results of a candidate run) has suffered problems with memory. These problems most visibly manifest with the Twitch List, which has a large number of test cases and produces an unusually large volume of results. The compare step is organized as a collection of worker processes all running on a single machine.

In March and April of 2016, a first step was made to resolve the memory problem. Previously a large list of data structures was constructed and copies of the list were handed to each worker process. The first fix was to allow each worker process to construct only the portions of the list that it needed to do its work. For the first time, it was possible for the Twitch List to run to completion.

In early 2017, for some reason that I don’t know, we ran into trouble again. We were producing more results because now we included source code snippets, but when I disabled the snippets code there were still problems. In any event, memory requirements had increased again and once again the Twitch List compare step was suffering memory failures.

After much debugging and tinkering, I discovered that memory can fail in either
of two ways.
* The obvious way is a failure of garbage collection. Python uses reference counting to decide when an object is still in scope. When an object’s reference count reaches zero, it is deallocated. This system can fail if there are cyclic references among two or more objects, each object keeping the next object alive, while the scope in which they were created is done with them. Initially I suspected a cyclical reference, indeed I could hardly imagine any other reason why memory usage should accumulate over the duration of the compare step.
* The second way memory can fail is memory fragmentation. Python makes a large number of small memory allocations and deallocations. The operating system would ideally pool all the deallocated memory blocks together and be able to hand them out again on the next allocation request. But it may not be smart enough to do that. It’s not practical to move the freed memory because objects in use would need to move too, and you can’t hope to follow all the references to them throughout the code that’s running. But you can notice that freed blocks are contiguous and recombine them into larger blocks, and that helps.

When a Python process ends, all this is sorted out. All the memory is freed, no
more references need to be tracked, and you’re back to a nice clean slate of
memory allocation.

The deeper fix during March of 2017, done with fragmentation in mind, was to replace the long-lived Python processes (each processing a sequence of test cases) with a lot of short-lived Python processes, each handling a single test case.

![Alt text](https://raw.github.com/wware/defragger/master/compareMemUsage.png)

This is a typical graph of memory usage now. This is running on a machine with 64 gig of RAM (which would show as 68700 on the vertical axis).

Later, a further refinement was devised. The number of worker processes is normally assigned based on CPU resources, one task per available CPU core. As the memory gets to various levels of gigabytes, the permitted number of workers is reduced so that fewer new processes are started. When the existing processes complete and memory is available again, the normal number of workers is permitted again. 

Further work
----

I had previous experience with memory fragmentation in a very [different context](https://github.com/wware/stuff/tree/master/hack-malloc), several years ago. This was some hacking I did with FreeRTOS, an embedded controller operating system. Based on this work, I began work on a defragmentation layer to go between the Python interpreter and the operating system, as a [Hackathon project](https://github.com/wware/defragger).

Due to the brief duration of Hackathon, my work on this has been rushed and is not yet finished. I had hoped to demonstrate it as a stretch goal, but it is not ready. It’s pretty close, and I hope to finish it soon and try deploying it in the Buildbot staging stack to see if it makes a significant difference. But for now it is bumping into some problems with building the Python executable on top of the defragmentation layer.
