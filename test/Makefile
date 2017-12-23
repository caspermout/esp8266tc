
IDIR =../include
CC=/usr/bin/gcc
CFLAGS=-I$(IDIR) -I. -I../fatfs -Os -g -O2 -Wpointer-arith -Werror -std=c99 -Wl,-EL -fno-inline-functions 
# -mlongcalls -mtext-section-literals -nostdlib   -D__ets__ -DICACHE_FLASH


ODIR=obj
LDIR =../lib

LIBS=-lm

_DEPS = 
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = fatfs_test.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


all: checkdirs bin/fatfs_test bin/temp_test

checkdirs: bin obj obj/fatfs obj/tempcontrol

obj:
	mkdir -p $@

bin:
	mkdir -p $@

obj/fatfs:
	mkdir -p $@

obj/tempcontrol:
	mkdir -p $@


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

obj/fatfs/%.o: ../fatfs/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

obj/tempcontrol/%.o: ../tempcontrol/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

#obj/fatfs/diskio.o: ../fatfs/diskio.c $(DEPS)
#	$(CC) -c -o $@ $< $(CFLAGS)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

bin/fatfs_test: obj/fatfs/ff.o obj/fatfs/ffsystem.o obj/fatfs/ffunicode.o obj/fatfs_test_disk.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

bin/temp_test: obj/tempcontrol/sensors.o obj/tempcontrol/tempcontrol.o obj/temp_test.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean all checkdirs

clean:
	rm -f $(ODIR)/*.o $(ODIR)/*/*.o *~ bin/* core $(INCDIR)/*~


