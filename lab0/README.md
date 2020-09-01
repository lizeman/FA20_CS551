## Konwn Issues

For provided `ppmcvt` demo, it does not check whether input scaling factor is less than 0, thus leading to potential runtime `core dumped`.
In contrast, my program would check this condition for argument `-t` and `-n`.
