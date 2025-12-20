CFLAGS=-w4 -bt=dos -fo=.obj -mc

objects = esfm.obj esfmlib.obj

.c.obj: .AUTODEPEND
	wcl -c -q $[*

esfm.exe: $(objects)
	wcl /l=dos /q -lr -fe=$^. $(objects)

