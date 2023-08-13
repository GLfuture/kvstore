CC=g++
SRC_PATH=./app
CXX_STANDARD=-std=c++20
SRCS=$(SRC_PATH)/*.cc
TARGET=kvstore
TARGET_DIR=./sbin
$(TARGET):$(SRCS)
	$(CC) $(SRCS) -o $(TARGET_DIR)/$(TARGET) $(CXX_STANDARD)
clean:
	rm $(TARGET_DIR)/$(TARGET)