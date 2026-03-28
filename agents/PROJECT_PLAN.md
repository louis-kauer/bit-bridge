# Project Specification for AI Agent: "Bit Bridge" (C++ Layer 4 Load Balancer & UI)

## 1. Executive Summary & Context
**Project Name:** Bit Bridge
**Project Type:** C++ desktop application for configuring and running a TCP Layer 4 Load Balancer.
**Context:** This is a time-boxed university project with specific grading criteria. The primary requirement to pass is delivering a functional desktop application using a UI library. The Load Balancer itself acts as the technical extension (the "algorithm/mechanic" requirement) to secure a high grade in the "Complexity" category.
**Language Standard:** C++20.

### 1.1 University Grading Criteria (Development Focus)
* **40% Code Structure/Architecture:** The project **MUST** be strictly Object-Oriented (OOP). Focus on clean class design, heavy use of Smart Pointers (`std::unique_ptr`, `std::shared_ptr`), and RAII principles.
* **30% Performance & Error Resistance:** Strict exception handling is required. The program must never crash due to invalid UI inputs, missing files, or network connection drops. Use robust `try-catch` blocks.
* **10% Complexity:** Fulfilled by combining a visual UI, YAML file parsing, and asynchronous TCP networking.
* **10% Overall Impression & 10% Documentation:** Well-commented code and a structured `README.md` are required at the end.

## 2. System Architecture: "Decoupled Execution"
To minimize error susceptibility (30% of the grade) and avoid the complexities of running a UI event loop and an async networking loop simultaneously, the system is strictly decoupled. The project consists of a single codebase that operates in two distinct logical steps (or compiles into two separate executables):

1. **The UI Configuration Tool:** Built with **wxWidgets**. The user inputs the backend servers (IPs/Ports) here.
2. **The Configuration File:** The UI tool generates a Docker/Kubernetes-style **YAML file** named `bitbridge-config.yaml`.
3. **The Load Balancer (Core):** An asynchronous TCP proxy that reads the YAML file on startup and routes network traffic accordingly.

## 3. Technology Stack & Dependencies
* **C++ Version:** C++20
* **Build System:** CMake
* **UI Framework:** `wxWidgets` (Mandatory for Phase 1)
* **Configuration/Serialization:** `yaml-cpp` (For reading/writing the YAML file)
* **Networking (Phase 2):** `Boost.Asio` (For asynchronous TCP sockets)

## 4. Development Phases (Strict Order of Execution)

### Phase 1: The Configuration Tool (wxWidgets & YAML) - *Highest Priority*
This is the core of the university submission. Even if Phase 2 is not perfect, a working Phase 1 ensures a passing grade.
* **UI Design:** A `wxFrame` main window containing a list view (`wxListCtrl` or `wxGrid`) to display current backend nodes (IP address, Port). Include input fields (`wxTextCtrl`) and buttons to add/remove nodes.
* **YAML Integration:** A "Save Configuration" button that uses the `yaml-cpp` library to write the current nodes into `bitbridge-config.yaml`.
* **OOP Structure:** Build clean, encapsulated classes for the application state without strictly enforcing complex design patterns. Just ensure the UI components and the data serialization logic are neatly separated.

*Target YAML Structure Example:*
```yaml
apiVersion: v1
kind: LoadBalancerConfig
metadata:
  name: bit-bridge-cluster
spec:
  routingAlgorithm: power-of-two-choices
  listenPort: 8080
  backends:
    - ip: 192.168.1.10
      port: 80
    - ip: 192.168.1.11
      port: 80
```

### Phase 2: The Load Balancer (Layer 4) & Algorithm
Once Phase 1 compiles stably and generates the YAML file, implement the actual load balancer.
* **Startup Process:** A C++ module that uses `yaml-cpp` to read the configuration file generated in Phase 1.
* **Network Logic:** Use the Reactor pattern via `Boost.Asio`. The proxy listens on the port defined in the YAML, accepts incoming client connections, and pipes the byte streams (full-duplex) to the backends.
* **The Algorithm (Fulfilling the "at least one algorithm" requirement):** Implement **"Power of Two Choices"** (Randomized Least-Load).
  * The balancer randomly selects (`<random>`) two backend servers from the YAML list.
  * It compares their current active connections (must use `std::atomic<int>` for thread safety).
  * It routes the new TCP stream to the server with the lower load.

### Phase 3: Error Handling & Refactoring (Focus on the 30% Metric)
* Ensure invalid IP/Port inputs in the wxWidgets UI are caught and validated (using Regex or wxWidgets internal validators).
* Ensure that if `bitbridge-config.yaml` is missing when the Load Balancer starts, it triggers a clean error message (or dialog box) instead of a segmentation fault.
* Catch all `Boost.Asio` network exceptions. If a backend server drops offline, the proxy must gracefully close that specific socket without crashing the main application loop.

### Phase 4: Finalization
* Write the `README.md` (Architecture description, explanation of the Power of Two Choices algorithm, and a list of third-party libraries: wxWidgets, Boost, yaml-cpp).
* Ensure the CMake project can cleanly build for the target platform.

***

**Instructions for the AI Coding Agent:** Please begin strictly with **Phase 1**. First, write the `CMakeLists.txt` configuring C++20, wxWidgets, and yaml-cpp. Then, generate the boilerplate C++ code for the wxWidgets main window and the YAML serialization class. Wait for my feedback and confirmation that it compiles before moving on to Phase 2.