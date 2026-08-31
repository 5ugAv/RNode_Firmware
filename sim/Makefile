GFX ?= $(HOME)/Arduino/libraries/Adafruit_GFX_Library
CXXFLAGS = -std=c++17 -w -DARDUINO=10819 -DNRF52840_XXAA -DBOARD_MODEL=0x46 \
           -Istubs -I. -I$(GFX)
STATES = linked nophone pairing noradio corrupt busy lowbatt

sim: sim_main.cpp $(GFX)/Adafruit_GFX.cpp
	g++ $(CXXFLAGS) $^ -o $@

states: sim
	@for s in $(STATES); do ./sim $$s $$s.pgm; done

clean:
	rm -f sim *.pgm
.PHONY: states clean
