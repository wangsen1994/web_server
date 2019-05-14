# 简单写一个makefile文件

MAINSOURCE := main.cpp

SOUCRCE := $(wildcard *.cpp base/*.cpp)
override SOUCRCE := $(filter-out $(MAINSOURCE),$(SOUCRCE))

OBJS := $(patsubst %.cpp,%.o,$(SOUCRCE))

TARGET  := webServer
CC      := g++
LIBS    := -lpthread
INCLUDE := -I./usr/local/lib
CFLAGS  := -std=c++11 -g -Wall -O3 -D_PTHREADS
CXXFLAGS:= $(CFLAGS)

.PYTHON : objs clean veryclean rebuild all debug

all     : $(TARGET)
objs    : $(OBJS)

rebuild : veryclean all

clean   :
			find . -name '*.o' | xargs rm -f
veryclean :
			find . -name '*.o' | xargs rm -f
			find . -name $(TARGET) | xargs rm -f

debug   :
			@echo $(SOUCRCE)


$(TARGET) : $(OBJS) main.o
		$(CC) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)
