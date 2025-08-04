CC = gcc
CFLAGS_DEBUG = -Wall -Wextra -Wpedantic -g
CFLAGS_RELEASE = -Wall -Wextra -O3
SDL2_INCLUDE_PATH = $(shell pkg-config --cflags SDL2_ttf)
SDL2_LIB_PATH = $(shell pkg-config --libs SDL2_ttf)
LDFLAGS = -lm

SERVER_DIR = server
BUILD_DIR = build

CLIENT_SRC = $(wildcard *.c)
SERVER_SRC = $(wildcard $(SERVER_DIR)/*.c)

CLIENT_OBJ = $(CLIENT_SRC:%.c=$(BUILD_DIR)/%.o)
SERVER_OBJ = $(SERVER_SRC:$(SERVER_DIR)/%.c=$(BUILD_DIR)/%.o)

CLIENT_EXEC = $(BUILD_DIR)/client
SERVER_EXEC = $(BUILD_DIR)/server

all: release

$(CLIENT_EXEC): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) $(SDL2_INCLUDE_PATH) -o $@ $^ $(SDL2_LIB_PATH) $(LDFLAGS)

$(SERVER_EXEC): $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SDL2_INCLUDE_PATH) -c $< -o $@

$(BUILD_DIR)/%.o: $(SERVER_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

debug: CFLAGS = $(CFLAGS_DEBUG)
debug: $(CLIENT_EXEC) $(SERVER_EXEC)

release: CFLAGS = $(CFLAGS_RELEASE)
release: $(CLIENT_EXEC) $(SERVER_EXEC)


clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean debug release

