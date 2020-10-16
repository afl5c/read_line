# read_line

A small libreadline replacement. Inspired by [antirez/linenoise](https://github.com/antirez/linenoise)
except read_line is even smaller (~100 LOC) and simpler.

## Problem

The library libreadline is famously large and annoying to link. For example, on Linux
you have to install libreadline, libncurses, and libtinfo just to link it. All this just
for the ability to read a line with history and navigation.

## Solution

This implements a single function, read_line(), which reads a line while automatically
handling history and navigation (using Posix terminal control codes).

Also I hereby put this in the public domain, so you can use this for whatever.

## Technical Details

### Entering Raw Mode

For some reason, [linenoise][1] sets flags explicitly. [Most][2] [other][3] examples are
derived from linenoise, so they do the same thing. However, this is totally unecessary
because there is a function called [cfmakeraw()][4] that will set the flags for you. 

[1]:https://github.com/antirez/linenoise/blob/master/linenoise.c#L232
[2]:https://viewsourcecode.org/snaptoken/kilo/02.enteringRawMode.html
[3]:http://www.cs.uleth.ca/~holzmann/C/system/ttyraw.c
[4]:https://man7.org/linux/man-pages/man3/termios.3.html

However, even that seems like an oversight because if you read the docs,  most of the flags
are for canonical-mode-style behavior, so simply zeroing the termios struct should
suffice. So that's what I do here.

### Other Code Savings

Linenoise supports various rare terminals. I do not; I've just tested this on some Linuxes,
MacOS, and Raspberry Pi. (Note that Windows' console supports history and navigation natively).

Linenoise and other libraries set an atexit() handler to reset the tty attributes in case of
an interrupt. In my testing, this doesn't seem to be necessary, so I don't do it.

I use C++ which provides &lt;string&gt; and &lt;vector&gt;, so I don't have to manually
implement string or array manipulation. This also allows for unlimited lengths (in theory)
so I don't have to set MAX_LINE/MAX_HISTORY constants. I store the history as a static
variable, so I don't need a global history variable either.

Linenoise supports other stuff like multiline inputs, completions, and suggestions. I do not.

### Navigation Support

The following navigation keys are supported

 - &uarr; - Go up in history
 - &darr; - Go down in history
 - &rarr; - Go right
 - &larr; - Go left 
 - ctrl-a - Go to start
 - ctrl-e - Go to end
 - double-press esc - Erase Line

The exact history behavior is slighly different than libreadline. The first history entry is a blank line
and over-navigation will just rotate instead of sounding a bell.

A double escape clears the line, which makes it consistent with Windows-style console behavior.

For these reasons, I would actually argue that the navigation scheme here is actually better than libreadline.




