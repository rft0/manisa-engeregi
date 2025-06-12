CC = clang
CFLAGS = 

SRC_DIR = src
OBJ_DIR = obj
SRCS = \
	$(wildcard $(SRC_DIR)/*.c) \
	$(wildcard $(SRC_DIR)/parser/*.c) \
	$(wildcard $(SRC_DIR)/utils/*.c) \
	$(wildcard $(SRC_DIR)/diag/*.c) \
	$(wildcard $(SRC_DIR)/vm/builtins/*.c) \
	$(wildcard $(SRC_DIR)/vm/objects/*.c) \
	$(wildcard $(SRC_DIR)/vm/*.c) \



OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

INC_FLAGS = -I$(SRC_DIR)/include

TARGET_DIR = bin
TARGET_BIN = me
TARGET = $(TARGET_DIR)/$(TARGET_BIN)

LDLIBS = -lm

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(TARGET_DIR)
	$(CC) -o $@ $(OBJS) $(LDLIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(INC_FLAGS) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	$(TARGET)

clean:
	rm -rf $(OBJ_DIR) bin
	rm -f $(TARGET)

.PHONY: all clean run