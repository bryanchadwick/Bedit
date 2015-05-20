
# LD_FLAGS = -L/usr/X/lib  # Solaris!!
LD_FLAGS = -L/usr/lib   # Linux

XBedit : XBedit.cpp Control.cpp Bedit.o Print.o XView.o BufferList.cpp
	$(LINK.cc) $(LD_FLAGS) -o XBedit XBedit.cpp Bedit.o XView.o Print.o \
	 -lX11

CBedit : CBedit.cpp Control.cpp Bedit.o CursesView.o Print.o BufferList.cpp
	$(LINK.cc) -o CBedit CBedit.cpp Bedit.o CursesView.o Print.o -lcurses

Bedit.o : Bedit.cpp Bedit.h CircularStack.cpp
	$(LINK.cc) -c -o Bedit.o Bedit.cpp

XView.o : XView.cpp XView.h Bedit.h
	$(LINK.cc) -c -o XView.o XView.cpp

CursesView.o : CursesView.cpp CursesView.h Bedit.h
	$(LINK.cc) -c -o CursesView.o CursesView.cpp

Print.o : Print.cpp Print.h Bedit.h
	$(LINK.cc) -c -o Print.o Print.cpp

HTMLize: Print.o Bedit.o HTMLize.cpp
	$(LINK.cc) -o HTMLize HTMLize.cpp Print.o Bedit.o

clean: 
	rm -f XBedit CBedit Bedit.o XView.o CursesView.o Print.o
