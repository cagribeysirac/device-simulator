# Device Simulator

This is a device simulator for the system design before the hardware is available. It generates random data and sends it to the server.

## Dependencies

- CMake
- C++17
- g++

### Install dependencies

```bash
$ sudo apt-get install cmake g++
```

## Build

### Build Configuration

Create a build directory and generate build configuration files.
```bash
$ cmake -B build
```

### Build the project

Build and generate the executable file.
```bash
$ cmake --build build
```

### Run the project

Run the executable file.
```bash
$ ./build/simulator
```

## Usage

Example usage:
```bash
$ ./build/simulator --min 100 --max 1000 --interval 1
```

For more information, run:
```bash
$ ./build/simulator --help
```
