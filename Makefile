CXX ?= c++
CXXFLAGS ?= -std=c++11 -Wall -Wextra -Werror -pedantic -O2
ARDUINO_CLI ?= arduino-cli
INCLUDES = -Ilibraries/PalmControl/src

.PHONY: test demo compile clean
test: build/control-test
	./build/control-test

demo: build/demo
	./build/demo

build/control-test: tests/control_test.cpp libraries/PalmControl/src/PalmControl.h
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

build/demo: tools/demo.cpp libraries/PalmControl/src/PalmControl.h
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

compile:
	$(ARDUINO_CLI) compile --fqbn arduino:avr:uno --libraries libraries firmware/PalmTransmitter
	$(ARDUINO_CLI) compile --fqbn arduino:avr:uno --libraries libraries firmware/PalmReceiver
	$(ARDUINO_CLI) compile --fqbn arduino:avr:uno firmware/SensorCalibration

clean:
	rm -rf build
