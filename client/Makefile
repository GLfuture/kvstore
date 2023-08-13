CC=g++
SRC_PATH=.
SRCS=kvstore-cli.cc
TARGET=kvstore-cli
TARGET_DIR=$(SRC_PATH)/sbin
$(TARGET):$(SRC_PATH)/$(SRCS)
	$(CC) $(SRC_PATH)/$(SRCS) -o $(TARGET_DIR)/$(TARGET)
clean:
	rm $(TARGET_DIR)/$(TARGET)