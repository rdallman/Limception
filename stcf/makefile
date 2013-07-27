
CC = gcc
CFLAGS = -lpthread -g -lm -w
OBJECTS = stcf_scheduler.c

stcf_scheduler.exe : $(OBJECTS)
	$(CC) $(OBJECTS) -o stcf_scheduler.exe $(CFLAGS)

%.o : %.c
	$(CC)  $(CFLAGS) -c $<
