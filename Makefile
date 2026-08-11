CC := gcc

TARGET := bin/asm2.0

SRC := $(wildcard src/*.c)
OBJ := $(patsubst src/%.c,build/%.o,$(SRC))
DEP := $(OBJ:.o=.d)

WARNINGS := \
	-Wall \
	-Wextra \
	-Wpedantic \
	-Wshadow \
	-Wconversion \
	-Wsign-conversion \
	-Wcast-qual \
	-Wwrite-strings \
	-Wformat=2 \
	-Wundef \
	-Wstrict-prototypes \
	-Wold-style-definition \
	-Wimplicit-fallthrough \
	-Wlogical-op \
	-Wcast-align \
	-Wvla \
	-Wnull-dereference \
	-Wdouble-promotion \
	-Wformat-overflow=2 \
	-Wformat-truncation=2 \
	-Walloc-zero \
	-Warray-bounds=2 \
	-Wstringop-overflow=4 \
	-Wstrict-overflow=5 \
	-Wswitch-enum \
	-Wpointer-arith \
	-Winit-self

CFLAGS := -std=c11 -g $(WARNINGS)
CFLAGS += -Iinclude

# Not needed rn, but still there in case we need it later
LDLIBS := 

.PHONY: all clean run rebuild test

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p $(@D)
	@$(CC) $(OBJ) -o $@ $(LDLIBS)

build/%.o: src/%.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEP)

run: $(TARGET)
	@./$(TARGET) $(ARGS)

clean:
	@rm -rf build bin

rebuild: clean all

test:
	@./tests/run.sh
