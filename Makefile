# for C++ define  CC = g++
CC = g++
CFLAGS = -g -Wall -Weffc++ -fPIC -m64 -std=c++11
LFLAGS = -L$(ROOTSYS)/lib -L$(O2_ROOT)/lib
INC =	-I$(ROOTSYS)/include  -I$(O2_ROOT)/include -I$(O2_ROOT)/include/  -I./
TGT =	libCont.so
DICT=	ContDict.cxx
DICTO=	ContDict.o

SRC = 	o2cont.cxx


HDR =	$(SRC:.cxx=.h) $(O2_ROOT)/include/DetectorsBase/Track.h

OBJ = 	$(SRC:.cxx=.o)


.PHONY: depend clean

all: 	$(TGT)
	@echo creating libAlg.so

$(TGT):	$(OBJ) $(DICTO)
	$(CC) $(CFLAGS)  -shared -o $(TGT) $(OBJ) $(DICTO) `root-config --ldflags` $(LFLAGS)

# pull in dependency info for *existing* .o files
-include $(OBJ:.o=.d)

%.o : %.cxx
	$(CC) $(CFLAGS) $(INC) -c $<  -o $@
	$(CC) -MM $(CFLAGS) $(INC) -c $*.cxx > $*.d
	@cp -f $*.d $*.d.tmp
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

clean:
	rm -f *.o *~ *.so *.d *Dict.{h,cxx} *.pcm

$(DICT): $(HDR) ContLinkDef.h
	rootcint -f $@ -c $(INC) $(HDR) $^


depend: $(SRC)
	makedepend $(INC) $^

# DO NOT DELETE THIS LINE -- make depend needs it
