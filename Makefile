
all: water_sim

CXX := clang++
LDFLAGS := -lGL -lglut
CXXFLAGS := --std=c++11 -g

water_sim: water_sim.o
	$(CXX) $^ -o $@ $(LDFLAGS)

dbg: water_sim
	gdb --args ./water_sim
run: water_sim
	./water_sim
