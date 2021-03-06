PROJECT = MagicCubeServer

SOURCES := $(shell ls *.cpp)
OBJECTS := $(patsubst %.cpp, build/%.o, $(SOURCES))
ifeq ($(shell uname),Linux)
	# compiler
	CC = g++
	INC_DIR = ../include/osx
	CC_FLAGS = -O2 -Wall -fno-strict-aliasing -std=c++11 -I $(INC_DIR) -I ../include

	# linker
	LD = g++
	LIB_DIR = ../lib/linux
	LIBS = -lCubeCommon -lNetworkCommon -levent -lm -ldl -lpthread -lstdc++ -lc
	LINKER_FLAGS = -L $(LIB_DIR) -L ../CubeCommon/build -L ../NetworkCommon/build
else

ifeq ($(shell uname),Darwin)
	# compiler
	INC_DIR = ../include/osx
	CC_FLAGS = -O2 -Wall -fno-strict-aliasing -std=c++11 -I $(INC_DIR) -I ../include

	# linker
	LIB_DIR = ../lib/osx
	LIBS = -lCubeCommon -lNetworkCommon -levent -lc++ -lm -lSystem
	LINKER_FLAGS = -L $(LIB_DIR) -L ../CubeCommon/build -L ../NetworkCommon/build
else
	# Unsupported operating system.
	CC = echo && echo "******** Unsupported operating system! ********" && echo && exit 1 ||
endif

endif

.PHONY: all
all:
	-mkdir build
	make $(PROJECT)

build/%.o: %.cpp %.h Config.h ../NetworkCommon/NetworkConfig.h
	$(CC) -c $(CC_FLAGS) -o $@ $<

$(PROJECT): $(OBJECTS)
	$(CC) $(LINKER_FLAGS) -o build/$(PROJECT) $(OBJECTS) $(LIBS)

.PHONY: clean
clean:
	-rm build/*
	-rm -rf build
