## `my_mallc()`


1. my implmentation still returns a valid address when reuqest size=0.
2. Know core dump issue:  
    One tricky corner case: say if my program stores the starting point of the heap of my process by `void begin=sbrk(0);`  , before any heap increment. 
    Then if I invoke `my_free(begin);` , core dump would be triggered since the program tried to access the address before the lowest heap address. (Because my_free() needs to check the magic number, and I can not store global variable.)
