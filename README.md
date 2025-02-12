# SensorGateaway
This project implements a comprehensive Sensor Monitoring System designed to simulate and manage multiple temperature sensor nodes via a central sensor gateway. The system uses multi-threading, inter-process communication, and TCP connections to efficiently collect, process, and store real-time temperature data. It was developed as part of the Operating Systems course in the Bachelor of Electronics and ICT Engineering Technology program at KU Leuven.

## Project Overview
The system includes two main components:

Sensor Nodes (Clients):
- Simulated nodes that generate temperature data and send it to the sensor gateway via TCP connections.

Sensor Gateway (Server):
- Manages sensor connections, processes temperature data (e.g., running averages), stores data in CSV files, and logs system events (connections, disconnections, alerts) in gateway.log.

### System Architecture
Main Process launches three threads:

- Connection Manager: Manages TCP connections and data from sensor nodes.
- Data Manager: Processes sensor data and detects extreme temperatures.
- Storage Manager: Saves data to data.csv.

Log Process:A child process that logs events from the main process using pipes for inter-process communication.

Shared Data Structure (sbuffer):Enables thread-safe data exchange between components.

## Key Features
Multi-Threading: The gateway handles multiple sensor nodes concurrently, assigning a dedicated thread to each node for efficient data processing.

Real-Time Data Processing: Temperature data from sensor nodes is processed in real-time, with running averages calculated to detect abnormal conditions (too hot or too cold).

Persistent Data Storage: All temperature readings are stored in data.csv, allowing for later analysis. Each time the server starts, a new CSV file is created.

Event Logging: The system logs important events (e.g., sensor connections, disconnections, temperature alerts) into gateway.log, providing a clear history of system operations.

Robust Error Handling: The system gracefully handles invalid sensor data, connection timeouts, and unexpected disconnections, ensuring reliable performance.

## Technologies Used
- Programming Language: C
- Network Communication: TCP sockets
- Concurrency: Multi-threading with POSIX threads (pthreads)
- Inter-Process Communication: Pipes
- Data Management: CSV file handling for data storage
- Development Environment: Linux (tested on Ubuntu), Clion IDE
- Build System: Makefile automation for compiling and testing

## Running the Project
Compile the Code: make all
Start the Sensor Gateway: ./sensor_gateway <port_number> <max_clients>
Example: ./sensor_gateway 1234 3
This will start the server on port 1234, and it will accept up to 3 sensor node connections before shutting down.

Simulate Sensor Nodes:
Use the provided sensor_node.c from the labs to simulate client-side sensor nodes sending data to the gateway.

Check Output:
Sensor data is stored in data.csv.
System logs are recorded in gateway.log.
