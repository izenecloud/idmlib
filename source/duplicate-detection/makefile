CXX = g++

######################################################
# if $(COMPILE_FLAGS) is given by upper makefile 
# else if no $(COMPILE_FLAGS) is given 
ifneq ($(COMPILE_FLAGS),)
CXXFLAGS += $(COMPILE_FLAGS) 
else
CXXFLAGS = -Wall -O3 -DSF1_DEBUG
endif


TARGET_PATH = ../../lib
TARGET = $(TARGET_PATH)/libsf1v5_duplicate_detection_submanager.a

INCLUDE_PATH = \
			   -I../../include \
			   -I/usr/local/include/boost-1_36 \
			   -I$(WISELIB)/include \
			   -I$(IZENELIB)/include



OBJECTS = $(patsubst %.cc, %.o, $(wildcard *.cc) ) $(patsubst %.cpp, %.o, $(wildcard *.cpp) ) 

%.o : %.cc
	$(CXX) $(INCLUDE_PATH) $(CXXFLAGS) -c -o $@ $?

%.o : %.cpp
	$(CXX) $(INCLUDE_PATH) $(CXXFLAGS) -c -o $@ $?

all: $(TARGET)

$(TARGET) : $(OBJECTS)
	ar rsuv $@ $^

clean:
	rm -rf *.o $(TARGET) 


