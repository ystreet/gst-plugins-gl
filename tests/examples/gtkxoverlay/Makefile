SRC=main.cpp
OBJ=$(SRC:.cpp=.o)
EXE=gtkxoverlay

CPP=g++
CPPFLAGS=-Wall -Werror
LDFLAGS=
LIBS=-lgstreamer-0.10
RM=rm

ifeq ($(OS),Windows_NT)
# Put Windows-specific stuff here (usefull for mingw32)
REXE=$(EXE).exe
CPPFLAGS=-I../../../../libxml2-2.6.30+.win32/include \
         -I../../../../libiconv/include \
		 -I../../../../glib/include \
		 -I../../../../gstreamer/include \
		 -I../../../../gtk+/include \
		 -I../../../../pango/include \
		 -I../../../../cairo/include \
		 -I../../../../atk/include
#ld tool from mingw needs dll too
LDFLAGS+=-L../../../../glib/lib \
	     -L../../../../gstreamer/lib \
		 -L../../../../gtk+/lib \
		 -L../../../../glib/bin \
	     -L../../../../gstreamer/bin/bin
LIBS+=-lglib-2.0 -lgmodule-2.0 -lgobject-2.0 -lgthread-2.0 -lgstinterfaces-0.10 -lgtk-win32-2.0 -lgdk-win32-2.0
else
# Put Linux-specific stuff here
REXE=$(EXE)
CPPFLAGS+=-I/usr/include/gstreamer-0.10 \
          -I/usr/include/glib-2.0 \
          -I/usr/lib/glib-2.0/include \
          -I/usr/include/libxml2 \
          -I/usr/include/gtk-2.0 \
          -I/usr/lib/gtk-2.0/include \
          -I/usr/include/cairo \
          -I/usr/include/pango-1.0 \
          -I/usr/include/atk-1.0
LIBS+=-lgstinterfaces-0.10 -lglib-2.0 -lgmodule-2.0 -lgobject-2.0 -lgthread-2.0 -lgtk-x11-2.0 -lgdk-x11-2.0
endif


.PHONY : all
all: $(REXE)

$(REXE): $(OBJ)
	 $(CPP) $(OBJ) $(LDFLAGS) $(LIBS) -o $@

.PHONY : clean
clean:
	-$(RM) $(OBJ) core
