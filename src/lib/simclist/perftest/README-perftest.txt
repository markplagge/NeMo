SimCList performance test cases

===== ins.c
insert 10 000 000 (ten million) elements into a list, with element autocopy
disabled.
Compile:
    gcc -O2 -I.. -std=c99 -o ins ins.c ../simclist.c
Use:
    time ./ins


===== ext.c
insert 1 000 000 (one million) elements with element autocopy, then extracts 1
000 elements at random position (from a Uniform(0, list_size) probability
density function).
Compile:
    gcc -O2 -I.. -std=c99 -o ext ext.c ../simclist.c
Use:
    time ./ext


===== sort.c
insert 1 000 000 elements with autocopy, then sorting.

Compile:
    # for testing the default setup
    gcc -O2 -I.. -std=c99 -o sort sort.c ../simclist.c
    # for testing with threading enabled
    gcc -DSIMCLIST_WITH_THREADS -O2 -I.. -std=c99 -o sort sort.c ../simclist.c
Use:
    # generate data to insert into the list
    # e.g.  for ((i = 0; i<1000000; i++)); do echo $RANDOM; done >randdata.txt
    time ./sort < randdata.txt

