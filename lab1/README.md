## `my_mallc()`


1. my implmentation still returns a valid address when reuqest size=0.

2. To avoid core dump caused by attempting `my_free()` pointers which refer to range out of valid range, I use another global variable `void * heap_low_end* to record the heap low end. 
For the heap high end, it is possible to avoid use global variable, instead I use sbrk(0) everytime when entering `my_free()`.
