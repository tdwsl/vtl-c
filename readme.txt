VTL-C

This is a simple interpreter for VTL-2 in C. VTL-2 was originally implemented
in under 1kb of machine code, so it's a very simple language. For more
information, I'd definitely reccomend the reference manual 'VTL A Very Tiny
Language' online; it's pretty comprehensive despite it's small size.

There's just one file to compile, vtl.c, which should work on most systems.
On Linux and Unix, compile using -DTERMIOS for raw character input when
reading from '$'.

I've included some examples I've written to get a sense for it. To print the
alphabet, run vtl with 'alph.v' as its first argument.

This interpreter is doesn't really have a REPL and is more for running files.
I made it because I was having some trouble with emulating the Altair 8800, so
although think it's somewhat accurate, I'm not 100%. In particular, I'm not
sure how my Sokoban example would run on an old interpreter, being that it
reads the source text directly.

