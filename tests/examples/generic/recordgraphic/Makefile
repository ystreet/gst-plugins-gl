SRC=main.cpp
OBJ=$(SRC:.cpp=.o)
EXE=recordgraphic

CPP=g++
CPPFLAGS=-Wall -Werror -O3 -march=i686 -msse2 -mfpmath=sse
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
		 -I../../../../glew/include
#ld tool from mingw needs dll too
LDFLAGS+=-L../../../../glib/lib \
	     -L../../../../gstreamer/lib \
		 -L../../../../glew/lib \
		 -L../../../../glib/bin \
	     -L../../../../gstreamer/bin/bin \
		 -L../../../../glew/bin
LIBS+=-lglib-2.0 -lgmodule-2.0 -lgobject-2.0 -lgthread-2.0 \
      -lopengl32 -lglu32 -lglew32 -lglew32s
else
# Put Linux-specific stuff here
REXE=$(EXE)
CPPFLAGS+=-I/usr/include/gstreamer-0.10 \
          -I/usr/include/glib-2.0 \
          -I/usr/lib/glib-2.0/include \
          -I/usr/include/libxml2
LIBS+=-lgstreamer-0.10 -lglib-2.0 -lgmodule-2.0 -lgobject-2.0 -lgthread-2.0 -lGLU -lGL -lGLEW
endif


.PHONY : all
all: $(REXE)

$(REXE): $(OBJ)
	 $(CPP) $(OBJ) $(LDFLAGS) $(LIBS) -o $@

.PHONY : clean
clean:
	-$(RM) $(OBJ) core
