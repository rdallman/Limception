CC = gcc
CFLAGS = -lpthread -g -lm -w
OBJECTS = exp_scheduler.c

exp_scheduler.exe : $(OBJECTS)
	$(CC) $(OBJECTS) -o exp_scheduler.exe $(CFLAGS)

%.o : %.c
	$(CC)  $(CFLAGS) -c $<
