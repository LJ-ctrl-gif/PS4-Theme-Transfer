# OpenOrbis PS4 toolchain path must be supplied by the build environment.
OO_PS4_TOOLCHAIN ?= $(OO_PS4_TOOLCHAIN)
CXX := $(OO_PS4_TOOLCHAIN)/bin/prospero-clang++
TARGET := ps4-theme-transfer-receiver.elf
SOURCES := src/main.cpp
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra

.PHONY: all clean check-toolchain

all: check-toolchain $(TARGET)

check-toolchain:
	@test -x "$(CXX)" || (echo "OpenOrbis compiler not found: $(CXX)"; exit 1)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET)
