CXX = g++

CXXFLAGS = -Wall -Wextra -O2

INCLUDES = -I/usr/include/libudev

LIBS = -ludev

SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = rd_watcher

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

