# Makefile from Efiedit.
# Efiedit is free software: see COPYING

CC=gcc
#CFLAGS=-g -Wall
CFLAGS=-Wall -fcommon

objects=main.o read_settings.o write_settings.o ini.o
headers=efiedit.h ini.h

all:	efiedit

efiedit:	$(objects)
	${CC} ${CFLAGS} -o $@ $^
	
main.o:	main.c main.h $(headers)

$(objects):	%.o: %.c $(headers)
		${CC} ${CFLAGS} -c $< -o $@

clean:
	-rm -f *.o
