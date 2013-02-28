#
# Makefile for timetable_creator
# make all
#      debug
#      release
#      windows
#      clean - removes obj files + binaries
#      rm    - rm -rf on debug, release, windows directories
#

CC = gcc
WCC = i586-mingw32msvc-gcc
CFLAGS = -Wall -Wextra -pedantic -Wno-long-long
DFLAGS = -ggdb3 -DDEBUG
RFLAGS = -O3 -m32 -s 
WFLAGS = -O3 -DWINDOWS

SRCS = main.c structs.c timetable.c
OBJS = $(SRCS:.c=.o)
EXECUTABLE = timetable_creator

DEBUG = debug
DEBUG_OBJS = $(addprefix $(DEBUG)/,$(OBJS))
DEBUG_EXEC = $(DEBUG)/$(EXECUTABLE)

RELEASE = release
RELEASE_OBJS = $(addprefix $(RELEASE)/,$(OBJS))
RELEASE_EXEC = $(RELEASE)/$(EXECUTABLE)

WINDOWS = windows
WINDOWS_OBJS = $(addprefix $(WINDOWS)/,$(OBJS))
WINDOWS_EXEC = $(WINDOWS)/$(addsuffix .exe,$(EXECUTABLE))

dir_guard=@mkdir -p $(@D)

all: $(DEBUG) $(RELEASE) $(WINDOWS) 


$(DEBUG): $(DEBUG_OBJS)
	$(CC) $(DEBUG_OBJS) -o $(DEBUG_EXEC)

$(DEBUG)/%.o: %.c
	$(dir_guard)
	$(CC) -c $(CFLAGS) $(DFLAGS) -o $@ $<


$(RELEASE): $(RELEASE_OBJS)
	$(CC) $+ $(RFLAGS) -o $(RELEASE_EXEC) 

$(RELEASE)/%.o: %.c
	$(dir_guard)
	$(CC) -c $(CFLAGS) $(RFLAGS) -o $@ $<


$(WINDOWS): $(WINDOWS_OBJS) 
	$(WCC) $+ -o $(WINDOWS_EXEC) 

$(WINDOWS)/%.o: %.c
	$(dir_guard)
	$(WCC) -c $(CFLAGS) $(WFLAGS) -o $@ $<


clean:
	rm -f $(DEBUG_OBJS) $(DEBUG_EXEC)
	rm -f $(RELEASE_OBJS) $(RELEASE_EXEC)
	rm -f $(WINDOWS_OBJS) $(WINDOWS_EXEC) 

rm:
	rm -rf $(DEBUG) $(RELEASE) $(WINDOWS)

.PHONY: clean
.PHONY: rm
