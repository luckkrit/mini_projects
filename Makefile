# ==========================================
# Project Settings
# ==========================================
CC      = gcc
CFLAGS  = -Wall -Wextra -std=gnu11 -I./src -pthread
LDFLAGS = -lcrypt -pthread -lrt

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
ORDER_TEST_TARGET = order_test

# ==========================================
# Object Files
# ==========================================
AUTH_OBJ  = $(OBJ_DIR)/auth.o
STORE_OBJ = $(OBJ_DIR)/store.o
ORDER_OBJ = $(OBJ_DIR)/order.o
COMMON_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(wildcard $(SRC_DIR)/common*.c)) $(STORE_OBJ) $(AUTH_OBJ) $(ORDER_OBJ)

SERVER_OBJ = $(OBJ_DIR)/server.o
CLIENT_OBJ = $(OBJ_DIR)/client.o

AUTH_TEST_DRIVER_OBJ  = $(OBJ_DIR)/test_auth.o
STORE_TEST_DRIVER_OBJ = $(OBJ_DIR)/test_store.o
ORDER_TEST_DRIVER_OBJ = $(OBJ_DIR)/test_order.o

# ==========================================
# Build Rules
# ==========================================

all: $(BIN_DIR)/$(SERVER_TARGET) $(BIN_DIR)/$(CLIENT_TARGET) $(BIN_DIR)/$(AUTH_TEST_TARGET) $(BIN_DIR)/$(STORE_TEST_TARGET) $(BIN_DIR)/$(ORDER_TEST_TARGET)

$(BIN_DIR)/$(SERVER_TARGET): $(SERVER_OBJ) $(AUTH_OBJ) $(COMMON_OBJS) | $(BIN_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)

$(BIN_DIR)/$(CLIENT_TARGET): $(CLIENT_OBJ) $(COMMON_OBJS) | $(BIN_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)

$(BIN_DIR)/$(AUTH_TEST_TARGET): $(AUTH_TEST_DRIVER_OBJ) $(AUTH_OBJ) | $(BIN_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)

$(BIN_DIR)/$(STORE_TEST_TARGET): $(STORE_TEST_DRIVER_OBJ) $(STORE_OBJ) | $(BIN_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)

$(BIN_DIR)/$(ORDER_TEST_TARGET): $(ORDER_TEST_DRIVER_OBJ) $(ORDER_OBJ) | $(BIN_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)

# --- Compilation Rules ---

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/test_%.o: $(TEST_DIR)/test_%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR) $(BIN_DIR):
	mkdir -p $@

# ==========================================
# Utility Commands
# ==========================================

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

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

test-order: $(BIN_DIR)/$(ORDER_TEST_TARGET)
	@echo "\nRunning Order Tests..."
	@./$(BIN_DIR)/$(ORDER_TEST_TARGET)
    
run-server: $(BIN_DIR)/$(SERVER_TARGET)
	./$(BIN_DIR)/$(SERVER_TARGET)

run-client: $(BIN_DIR)/$(CLIENT_TARGET)
	./$(BIN_DIR)/$(CLIENT_TARGET)

run-auth: $(BIN_DIR)/$(AUTH_TEST_TARGET)
	./$(BIN_DIR)/$(AUTH_TEST_TARGET)

run-store: $(BIN_DIR)/$(STORE_TEST_TARGET)
	./$(BIN_DIR)/$(STORE_TEST_TARGET)

run-order: $(BIN_DIR)/$(ORDER_TEST_TARGET)
	./$(BIN_DIR)/$(ORDER_TEST_TARGET)

sem-clean:
	rm -f /dev/shm/sem.store_lock

.PHONY: all clean test run-server run-client run-auth run-store run-order sem-clean