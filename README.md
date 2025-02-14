# Device Simulator

This is a device simulator for the system design before the hardware is available. It generates random data and sends it to the server.

## Dependencies

- CMake
- C++17
- g++
- Boost (system, uuid)
- Paho MQTT C
- Paho MQTT C++

### Install dependencies

```bash
$ sudo apt-get install cmake g++ libboost-all-dev libpaho-mqtt-dev libpaho-mqttpp-dev
```

## Build

### Build Configuration

Create a build directory and generate build configuration files.
```bash
$ cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

### Build the project

Build and generate the executable file.
```bash
$ cmake --build build
```

### Run the project

Run the executable file.
```bash
$ ./build/DeviceSimulator
```

## Usage

Example usage:
```bash
$ ./build/DeviceSimulator --min 100 --max 1000 --interval 1 --broker-address mqtt://127.0.0.1:1883 --topic local/sim --id SIM-123
```

For more information, run:
```bash
$ ./build/DeviceSimulator --help
```
