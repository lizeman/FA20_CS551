1. `myar -t case.a`

same as `ar -t case.a`

2. `myar -tv case.a` `myar -vt case.a`

same as `ar -tv case.a`

3. `myar -x case.a` `myar -xo case.a`

4. `myar -x case.a one`
   `myar -x case.a one unexist_file two unexist_file_two`

Special cases: what if file is existed and can not be write.

5. `myar -d case.a one`
   `myar -d case.a one unexist_file two unexist_file_two`
