# Bedit
The (not) famous Unix/X11 text/code editor

Bryan Chadwick.

****************************************************
**   BEdit (The B-Editor, where B is for Bryan)   **
****************************************************

This is a reasonably sized editor that I created (mostly just for fun) for Unix, originally
  developed on Solaris, circa 2003.  I started with an NCurses version to allow me to edit
  sources in a terminal with syntax/key-commands that I was used to (I used a Mac back then).

I then extended it to also inlude an X11 version with tabs, command execution in buffers, and
  printing to Post-Script and HTML.  And, at some point I attempted to fix NCurses breakages on
  Linux, but I don't remember how far I got.

The code is rather dense, and comments are sparse, so I'll give an overview of what I can
  glean/remember from the source.

shareFiles.tgz : files that can/should be placed in ~/share/Bedit. Default messages, keywords, etc.

Makefile : Straight forward build file for make... XBedit is the default target.

XBedit.cpp : X specific main program.  Is just about 100% functional, though some of the keys
             used seem arbitrary now.  See "messages.txt" (in shareFiles.tgz) for an explanation of
             keys and some descriptions.
             
XView.h, XView.cpp : X specific View data/methods for drawing in an X11 window/display

CBedit.cpp : NCurses specific main program.  Only some of the commands work.  It appears that getting
             Ctrl keys working was much more difficult, but I can't remember how much was working
             before I attempted to fix it on Linux.  If you run it, F5 can still quit.
             
CursesView.h, CursesView.cpp : NCurses specific View data/methods.  Drawing is a little more tedious
                               in the terminal.

Bedit.h, Bedit.cpp : 
Control.cpp : General controller.  Manages the text-model and calls the View-specific methods when
              needed.

Print.h, Print.cpp : Printing functions for PostScript and HTML pages.
HTMLize.cpp : Stand-alone version of conversion to HTML, useful in generating source listings/trees.

BufferList.h, BufferList.cpp : The main text-model implementation.  Most of the complex text
                               manipulation for both editors is here.
CircularStack.cpp : A "circular" (overwriting) implementation of a stack.  Used to track undo actions
