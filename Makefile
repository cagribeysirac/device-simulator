.PHONY: build clean run

# Build the project (Default target)
build:
	rm -rf ./build/*
	cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	cmake --build build

# Clean the build directory
clean:
	rm -rf ./build/*

# Run the project
run:
	./build/DeviceSimulator
