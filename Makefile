
all: water_sim

LDFLAGS := -lGL -lglut

water_sim: water_sim.o
	$(CC) $^ -o $@ $(LDFLAGS)

run: water_sim
	./water_sim
