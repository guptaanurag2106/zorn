CC = gcc
CFLAGS_DEBUG = -Wall -Wextra -Wpedantic -g -I$(COMMON_DIR)
CFLAGS_RELEASE = -Wall -Wextra -O3 -I$(COMMON_DIR) -march=native -funroll-loops -DNDEBUG
SDL2_INCLUDE_PATH = $(shell pkg-config --cflags SDL2_ttf)
SDL2_LIB_PATH = $(shell pkg-config --libs SDL2_ttf)
LDFLAGS = -lm

SERVER_DIR = server
COMMON_DIR = common
BUILD_DIR = build
SERVER_BUILD_DIR = $(BUILD_DIR)/server
COMMON_BUILD_DIR = $(BUILD_DIR)/common

# Include common directory in CFLAGS
CFLAGS += -I$(COMMON_DIR)

CLIENT_SRC = $(wildcard *.c) $(wildcard $(COMMON_DIR)/*.c)
SERVER_SRC = $(wildcard $(SERVER_DIR)/*.c) $(wildcard $(COMMON_DIR)/*.c)

CLIENT_OBJ = $(CLIENT_SRC:%.c=$(BUILD_DIR)/%.o)
SERVER_OBJ = $(SERVER_SRC:%.c=$(BUILD_DIR)/%.o)

CLIENT_EXEC = $(BUILD_DIR)/client
SERVER_EXEC = $(BUILD_DIR)/server_executable  # Changed the name here

all: release

$(CLIENT_EXEC): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) $(SDL2_INCLUDE_PATH) -o $@ $^ $(SDL2_LIB_PATH) $(LDFLAGS)

$(SERVER_EXEC): $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Create build and common build directories
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(BUILD_DIR) $(COMMON_BUILD_DIR)
	$(CC) $(CFLAGS) $(SDL2_INCLUDE_PATH) -c $< -o $@

$(SERVER_BUILD_DIR)/%.o: $(SERVER_DIR)/%.c
	@mkdir -p $(SERVER_BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(COMMON_BUILD_DIR)/%.o: $(COMMON_DIR)/%.c
	@mkdir -p $(COMMON_BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

debug: CFLAGS = $(CFLAGS_DEBUG)
debug: $(CLIENT_EXEC) $(SERVER_EXEC)

release: CFLAGS = $(CFLAGS_RELEASE)
release: $(CLIENT_EXEC) $(SERVER_EXEC)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean debug release

