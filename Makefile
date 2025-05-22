CC = gcc
CFLAGS = 

SRC_DIR = src
OBJ_DIR = obj
SRCS = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/**/*.c)

OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

INC_FLAGS = -I$(SRC_DIR)/include

TARGET_DIR = bin
TARGET_BIN = out
TARGET = $(TARGET_DIR)/$(TARGET_BIN)

all: $(TARGET) run

$(TARGET): $(OBJS)
	@mkdir -p $(TARGET_DIR)
	$(CC) -o $@ $(OBJS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(INC_FLAGS) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	$(TARGET)

clean:
	rm -rf $(OBJ_DIR) bin
	rm -f $(TARGET)

.PHONY: all clean run