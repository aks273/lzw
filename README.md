# LZW
An implementation of lzw decoding, written in C.

Files are usually encoded using 8-bit (256) codes, which can represent e.g. the ASCII codes or RGB codes. 

To take advantage of repetition that could take place in a file, LZW compression uses 12-bit (4096) codes, numbers 257-4096 are built up as the file is read.

For example, a file could look like the following:

```
AABABCBC...
```

This file would have the following first four codes:
```
257:  AA
258:  AB
259:  BA
260:  ABC // We find that AB is already in the dictionary, so add another character on.
261:  CB
```
and when encoded would look like the following:
```
A,A,B,AB,C,B...
```


When the 4096th element of the dictionary is filled in, the dictionary is reset to only contain 256 characters and the rest are to be built up again.
