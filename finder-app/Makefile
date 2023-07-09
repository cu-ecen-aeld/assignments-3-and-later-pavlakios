CC := gcc
CROSS_COMPILE ?=
CFLAGS := -Wall -Wextra

ifeq ($(CROSS_COMPILE),)
  TARGET := writer
else
  TARGET := writer-$(CROSS_COMPILE)
  CC := $(CROSS_COMPILE)gcc
endif

SRCS := writer.c
OBJS := $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS)
