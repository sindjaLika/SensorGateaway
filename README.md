# SensorGateaway
This project implements a comprehensive Sensor Monitoring System designed to simulate and manage multiple temperature sensor nodes via a central sensor gateway. The system uses multi-threading, inter-process communication, and TCP connections to efficiently collect, process, and store real-time temperature data. It was developed as part of the Operating Systems course in the Bachelor of Electronics and ICT Engineering Technology program at KU Leuven.

## Project Overview
The system consists of two key components:

Sensor Nodes (Clients): Simulated in software, each sensor node generates temperature data and transmits it to the sensor gateway using TCP connections.

Sensor Gateway (Server): The core of the project, the gateway is responsible for:
Managing Connections: Handling multiple sensor node connections concurrently.
Data Processing: Reading sensor data, performing calculations like running averages, and identifying extreme temperatures.
Data Storage: Saving collected data in a CSV file for future analysis.
Logging Events: Recording system events (e.g., connections, disconnections, and temperature alerts) into a log file.
System Architecture

Main Process launches three threads:
Connection Manager: Listens for incoming sensor node connections over TCP, handles incoming data, and ensures each sensor node has a dedicated processing thread.
Data Manager: Processes incoming data, computes running averages, and determines if temperatures are too hot or cold.
Storage Manager: Writes temperature data to data.csv for persistent storage.

Log Process:
Runs as a child process of the main server process.
Uses pipes for inter-process communication to receive log events from the main process.
Logs events like new sensor connections, temperature alerts, and data insertions in gateway.log.
Shared Data Structure (sbuffer): Facilitates communication between threads. Ensures thread-safe access when reading and writing sensor data.

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
