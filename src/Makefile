#compiler setup
CXX = g++
CXXFLAGS = -std=c++14 -march=native -pthread
LDFLAGS = -Llib -lalloc431 -lpthread



COMMON= common/utils.h common/cxxopts.h common/get_time.h common/quick_sort.h common/parallel.h common/allocator.h
ALL= dynamic_task_scheduling triangle_counting

all : $(ALL)

% : %.cpp $(COMMON)
	$(CXX) $(CXXFLAGS)  $< $(LDFLAGS) -o $@


$(COMMON):

.PHONY : clean

clean:
	rm -f *.o *.obj $(ALL)

