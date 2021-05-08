# grep
grep from Dr Dobb's toolbook of c

This version of grep was written by Allen Holub, and published in Dr Dobb's toolbook of C.

There are additional sections code, tools.c, also by Allen Holub and wildexp.c by Leor Zolman
and published in the BDSC extended source release.

There is a extensive man page in the book, which I can include in summary here.

```
The program will find a string specified by a regular expression in a file or group of
files.  The following options are recognized:

-v  All lines except those matching are printed.  A negative search.
-c  Only a count of matching lines is printed.
-l  List only file names that have matching patterns.
-n  Each line is preceded by its line number in the file.
-y  Is always on for CP/M since you can't enter a lower case string in an option string.
-e <expression> Allows a more complex search string.
-f <file> Get the search string from this file.

```
For more detail, you will need to get the book.  

The wildexp code by Leor finds all the file names on the current drive, and user number.  It will
pass the names back to grep to be included in the search queue.  The file names can be a wildcard 
file name string such as is common in cp/m.  The addition of an '!' will exclude the file name.  I.e.
grep -l printf !* .com  should find all files on the drive that are not * .com files.

# BUGS

There be plenty.  Wildexp is really touchy about how you express a file search key.  Also the '!' operator
is not working an seems to hang the machine.

Grep it self my have more than a few issues.  I have not had a chanch to ring it out completly.  I have 
found that grep is slow on my 18mhz z180, I have not tested it yet on the 7mhz z80.  The performance issue
is mostly in the streams I/O file section, and I suspect the primative string search code.
The -c option is broken.

# in use.

```
D>grep -n printf *.c
HELLO.C:003:        PRINTF("HELLO THERE\N");
TESTVER.C:029:    PRINTF("CP/M BDOS VERSION (%D.%D)\N",MAJOR,MINOR);
TESTVER.C:031:        PRINTF("NETWORK (%S%S%S)\N",SYSTEM&0X01?"[MP/M]":"",SYSTEM&0X02?"[CP/NET]":"",
TESTVER.C:036:        PRINTF("MACHINE (MCS80/Z80)\N",MACHINE);
TESTVER.C:039:        PRINTF("MACHINE (MCS86)\N");
TESTVER.C:042:        PRINTF("MACHINE (68000/Z8000)\N");
TESTVER.C:045:        PRINTF("MACHINE (?)\N");
TESTVER.C:048:    PRINTF("BDOS ADDRESS (0X%04X)\N",*BDOSADDR-6);
TESTVER.C:049:    PRINTF("BIOS ADDRESS (0X%04X)\N",*BIOSADDR-3);
TESTVER.C:051:    FPRINTF(STDOUT,"TPA SIZE (%U.%UK)\N",TPA/1024,(TPA % 1024)/100);
etc.
```
I'm provide a prebuilt binary for this program.

