1. `myar -t case.a`

expected print msg:
```
longfile.txt
one
two
four
shakespeare.txt
```


2. `myar -tv case.a` `myar -vt case.a`

expected print msg:
```
rw-r--r-- 40834/88     45 Sep 22 21:45 2020 longfile.txt
rw-r--r-- 40834/88      4 Sep 22 15:28 2020 one
rwxr-xr-x 40834/88      4 Sep 22 21:27 2020 two
rw-r--r-- 40834/88      5 Sep 22 15:28 2020 four
rw-r--r-- 40834/88 5458199 Apr 23 14:02 2020 shakespeare.txt
```
