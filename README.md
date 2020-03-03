# phyllo-cpp

[Phyllo](https://github.com/ethanjli/phyllo) is a point-to-point communication protocol suite and application framework specification designed for use with embedded systems.

Phyllo provides a specification for high-throughput + reliable + asynchronous message exchange between exactly two (2) peers. Phyllo also provides a specification for applications to communicate with each other across a pair of hosts (which may be either an embedded device or a regular computer) or distributed across multiple hosts.

Phyllo-cpp is a reference implementation in C++11 as a library for use on Arduino-compatible embedded systems such as the Arduino Due, Arduino Nano Every, and Teensy. The library can be used on extremely constrained embedded systems (8-bit microcontrollers with as little as 2 kB of RAM) such as the Arduino Uno and Arduino Micro, though with tighter constraints on data payload size.


## Design

Use of the phyllo stack is done by polling such that at most one newly-received data unit is made available (as an object member) after each poll, so that other code can perform arbitrary operations on received data units.

The phyllo stack does not allocate or use any heap memory - all memory is either on the stack or (for certain buffers) is held in a compile-time object.


## Functionality and Limitations

Because phyllo-cpp's development is driven by the needs of other ongoing projects, its development prioritizes the need of those projects. Here are phyllo-cpp's functionalities and limitations in order of higher priority to lower priority.

Currently, phyllo-cpp does:

- Implement ChunkedStreamLink.
- Implement FrameLink.
- Implement DatagramLink.
- Implement ValidatedDatagramLink.
- Partially implement ReliableBufferLink.
- Implement DocumentLink.
- Implement Pub-Sub Framework's MessageLink and DocumentLink and EndpointHandler and Router.
- Provide a polling-based interface for serial I/O.

Currently, phyllo-cpp does not yet:

- Fully implement ReliableBufferLink.
- Have moderate test coverage.
- Implement a way to report errors at the protocol level in DatagramLink or above.
- Provide documentation of library APIs.
- Have a first alpha release.
- Implement robust fault detection and handling in the transport or application layers.
- Have high test coverage.
- Guarantee portability to any platform besides the supported platforms listed below.


## Supported Platforms

The following platforms are actively tested with the library.

Arduino-compatible ARM-based microcontroller boards support full capabilities of the library:

- Arduino Due
- Teensy 4.0
- Teensy 3.6
- Teensy LC

Arduino-compatible AVR-based microcontroller boards:

- Arduino Nano Every
- Arduino Uno: to leave sufficient RAM for application software, stream chunks are limited to 80 bytes long, though this can be decreased depending on application needs to free up more RAM.
- Arduino Micro: same constraints as with the Uno.


## Performance

Preliminary benchmarks were measured on an echo task to serialize a Python tuple using MsgPack, send it to the microcontroller over USB serial with the minimal transport stack and minimal application stack, parse the msgpack payload, serialize the data into a msgpack payload, send it back to the Python library, and parse it (Intel Core i7-7700HQ CPU @ 2.80 GHz running KDE Neon; phyllo stack with threaded serial I/O adapter). In these benchmarks, the rate-limiting factor was always the microcontroller.

- Measured throughputs:
    - Teensy 4.0
        - Very large test payload: ~300 echoes/sec, ~72 kBps
        - Large test payload: ~760 echoes/sec, ~46 kBps
        - Small test payload: ~1.1k echoes/sec, ~24 kBps
    - Arduino Due
        - Very large test payload: ~150 echoees/sec, ~36 kBps
        - Large test payload: ~430 echoes/sec, ~26 kBps
        - Small test payload: ~900 echoes/sec, ~19 kBps
    - Teensy 3.6
        - Large test payload: ~140 echoes/sec, ~8 kBps
        - Small test payload: ~160 echoes/sec, ~3 kBps
    - Teensy LC
        - Large test payload: ~120 echoes/sec, ~7 kBps
        - Small test payload: ~140 echoes/sec, ~3 kBps
    - Arduino Micro
        - Large test payload: ~160 echoes/sec, ~9 kBps
        - Small test payload: ~300 echoes/sec, ~6 kBps
    - Arduino Uno
        - Large test payload: ~60 echoes/sec, ~3.5 kBps
    - Small test payload: ~120 echoes/sec, ~2.5 kBps
- Test payloads:
    - Very large, 240 bytes written over serial each way: `('hello', True, None, 0.125, b'\x00\x01\x02\ x03\x04', 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 1 59, 160, 161, 162, 163, 164, 165, 166)`
    - Large, 59 bytes written over serial each way: `('hello', True, None, 0.125, b'\x00\x01\x02\x03\x04', 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25)`
    - Small, 20 bytes written over serial each way: `('hello', 123, 456, 789)`


## Related Projects

The following libraries also provide network protocol stacks to enable reliable message exchange on embedded devices:

- [PJON](https://www.pjon.org) ([Github](https://github.com/gioblu/PJON)) enables communication between two or more devices, potentially on a network, over any of *a variety of media and transports*. PJON appears to only support receiving data or handling errors from the protocol stack using callback functions, thus inverting control so that data handlers are called rather than calling.
- [RadioHead](https://www.airspayce.com/mikem/arduino/RadioHead/) is a network protocol stack designed to provide to enable communication between two or more devices, potentially on a network, over *any of a variety of data radios* and other transports.
