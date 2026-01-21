CFLAGS=-w4 -bt=dos -fo=.obj -mc

objects = esfm.obj esfmlib.obj tui.obj helptext.obj midi.obj

objects_miditest = miditest.obj midi.obj

.c: src/

.c.obj: .AUTODEPEND
	wcl $(CFLAGS) -q -c $[*

esfm.exe: $(objects)
	wcl -l=dos -q -lr -fe=$^. $(objects)

miditest.exe: $(objects_miditest)
	wcl -l=dos -q -lr -fe=$^. $(objects_miditest)

clean: .SYMBOLIC
	rm -f *.err
	rm -f $(objects)
	rm -f esfm.exe

all: esfm.exe

