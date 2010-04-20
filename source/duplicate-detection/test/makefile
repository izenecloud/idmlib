GPP :=g++

GCC :=/usr/bin/gcc

CC_OPTIONS :=  -O3 -g -rdynamic -Wall -c -fmessage-length=0 


LIBS := -lpthread /home/Kevin/izenelib/lib/libprocmeminfo.so /home/Kevin/wiselib/lib/libwiselib.so /usr/local/lib/libboost_unit_test_framework-gcc41-mt-1_36.so /usr/local/lib/libboost_thread-gcc41-mt-1_36.so /usr/local/lib/libboost_filesystem-gcc41-mt-1_36.so /usr/local/lib/libboost_date_time-gcc41-mt.so /usr/local/lib/libboost_serialization-gcc41-mt.so

INC_PATH := -I  /usr/local/include/boost-1_36/ -I ../../../include/ -I /home/Kevin/izenelib/include/ -I ../../ -I /home/Kevin/la-manager/include/ -I /home/Kevin/wiselib/include/
#-I $(IZENELIB)/include/

OBJS := $(patsubst %.cpp, %.o, $(wildcard *.cpp) ) 
DUP_OBJS := $(patsubst %.cc, %.o, $(wildcard ../*.cc)) $(patsubst %.cpp, %.o, $(wildcard ../*.cpp)) 
#./t_main.o


all: algo dupd


.cc.o:
	$(GPP) $(CC_OPTIONS) $(INC_PATH)  -o"./$@" "$<" 

.cpp.o:
	$(GPP) $(CC_OPTIONS) $(INC_PATH)  -o"./$@" "$<" 

algo: t_algo.o t_master_suite.o $(DUP_OBJS)
	$(GPP)  -g -o "algo" $^ $(LIBS) && rm -f $(OBJS)

dupd: $(OBJS) $(DUP_OBJS)
	$(GPP)  -g -o "dupd"  $(OBJS) $(LIBS) 

clean:
	rm -f $(OBJS) $(DUP_OBJS) 
