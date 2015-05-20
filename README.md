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

_shareFiles.tgz_ : files that can/should be placed in ~/share/Bedit. Default messages, keywords, etc.

_Makefile_ : Straight forward build file for make... XBedit is the default target.

_XBedit.cpp_ : X specific main program.  Is just about 100% functional, though some of the keys
             used seem arbitrary now.  See "messages.txt" (in shareFiles.tgz) for an explanation of
             keys and some descriptions.
             
_XView.h, XView.cpp_ : X specific View data/methods for drawing in an X11 window/display

_CBedit.cpp_ : NCurses specific main program.  Only some of the commands work.  It appears that getting
             Ctrl keys working was much more difficult, but I can't remember how much was working
             before I attempted to fix it on Linux.  If you run it, F5 can still quit.
             
_CursesView.h, CursesView.cpp_ : NCurses specific View data/methods.  Drawing is a little more tedious
                               in the terminal.

_Bedit.h, Bedit.cpp_ : Configuration, TextModel and file related functions. Most of the complex text
                       manipulation for both editors is here.

Control.cpp : General (parametrized) controller.  Manages the text-model and calls the View-specific
              methods when needed.

_Print.h, Print.cpp_ : Printing functions for PostScript and HTML pages.
HTMLize.cpp : Stand-alone version of conversion to HTML, useful in generating source listings/trees.

_BufferList.h, BufferList.cpp_ : List of open-files/buffers for multiple tabs.

_CircularStack.cpp_ : A "circular" (overwriting) implementation of a stack.  Used to track undo actions
