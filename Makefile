# ==========================================
# Project Settings
# ==========================================
CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -I./src
LDFLAGS = -lcrypt

# Directories
SRC_DIR  = src
TEST_DIR = $(SRC_DIR)/tests
OBJ_DIR  = obj
BIN_DIR  = bin

# ==========================================
# Target Names
# ==========================================
SERVER_TARGET    = server
CLIENT_TARGET    = client
AUTH_TEST_TARGET  = auth_test
STORE_TEST_TARGET = store_test

# ==========================================
# Object Files
# ==========================================

# Core logic objects used by multiple targets
AUTH_OBJ  = $(OBJ_DIR)/auth.o
STORE_OBJ = $(OBJ_DIR)/store.o
COMMON_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(wildcard $(SRC_DIR)/common*.c)) $(STORE_OBJ)

# Primary application objects
SERVER_OBJ = $(OBJ_DIR)/server.o
CLIENT_OBJ = $(OBJ_DIR)/client.o

# Test driver objects
AUTH_TEST_DRIVER_OBJ  = $(OBJ_DIR)/test_auth.o
STORE_TEST_DRIVER_OBJ = $(OBJ_DIR)/test_store.o

# ==========================================
# Build Rules
# ==========================================

# Default: Build everything
all: $(BIN_DIR)/$(SERVER_TARGET) $(BIN_DIR)/$(CLIENT_TARGET) $(BIN_DIR)/$(AUTH_TEST_TARGET) $(BIN_DIR)/$(STORE_TEST_TARGET)

# Link Server: Needs server code, auth logic, and store logic
$(BIN_DIR)/$(SERVER_TARGET): $(SERVER_OBJ) $(AUTH_OBJ) $(COMMON_OBJS) | $(BIN_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)

# Link Client
$(BIN_DIR)/$(CLIENT_TARGET): $(CLIENT_OBJ) $(COMMON_OBJS) | $(BIN_DIR)
	$(CC) -o $@ $^

# Link Auth Test: Links the test driver with the actual auth logic
$(BIN_DIR)/$(AUTH_TEST_TARGET): $(AUTH_TEST_DRIVER_OBJ) $(AUTH_OBJ) | $(BIN_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)

# Link Store Test: Links the test driver with the actual store logic
$(BIN_DIR)/$(STORE_TEST_TARGET): $(STORE_TEST_DRIVER_OBJ) $(STORE_OBJ) | $(BIN_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)

# --- Compilation Rules ---

# Compile standard source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile test driver files
$(OBJ_DIR)/test_%.o: $(TEST_DIR)/test_%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create directories if they don't exist
$(OBJ_DIR) $(BIN_DIR):
	mkdir -p $@

# ==========================================
# Utility Commands
# ==========================================

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Run All Tests
test: $(BIN_DIR)/$(AUTH_TEST_TARGET) $(BIN_DIR)/$(STORE_TEST_TARGET)
	@echo "Running Auth Tests..."
	@./$(BIN_DIR)/$(AUTH_TEST_TARGET)
	@echo "\nRunning Store Tests..."
	@./$(BIN_DIR)/$(STORE_TEST_TARGET)

test-auth: $(BIN_DIR)/$(AUTH_TEST_TARGET)
	@echo "Running Auth Tests..."
	@./$(BIN_DIR)/$(AUTH_TEST_TARGET)

test-store: $(BIN_DIR)/$(STORE_TEST_TARGET)
	@echo "\nRunning Store Tests..."
	@./$(BIN_DIR)/$(STORE_TEST_TARGET)
	
# Individual Run Commands
run-server: $(BIN_DIR)/$(SERVER_TARGET)
	./$(BIN_DIR)/$(SERVER_TARGET)

run-client: $(BIN_DIR)/$(CLIENT_TARGET)
	./$(BIN_DIR)/$(CLIENT_TARGET)

run-auth: $(BIN_DIR)/$(AUTH_TEST_TARGET)
	./$(BIN_DIR)/$(AUTH_TEST_TARGET)

run-store: $(BIN_DIR)/$(STORE_TEST_TARGET)
	./$(BIN_DIR)/$(STORE_TEST_TARGET)

.PHONY: all clean test run-server run-client run-auth run-store