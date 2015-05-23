
all: water_sim

CXX := clang++
LDFLAGS := -lGL -lglut
CXXFLAGS := --std=c++11

water_sim: water_sim.o
	$(CXX) $^ -o $@ $(LDFLAGS)

run: water_sim
	./water_sim
