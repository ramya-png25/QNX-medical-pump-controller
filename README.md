# High-Precision Medical Pump Controller (QNX RTOS)

## 📌 Project Overview
This project simulates a **Safety-Critical Medical Device** developed on the **QNX Neutrino Real-Time Operating System (RTOS)**. The system ensures that a precise dose of fluid is delivered at exact 500ms intervals. 

The core focus of this project is **Reliability** and **Fault Tolerance**, featuring a dedicated **Watchdog Timer** to detect and handle software freezes or deadlocks.

## 🛠️ Key Technical Concepts
- **Deterministic Timing:** Uses `timer_create` and **QNX Pulses** instead of standard `sleep()` to ensure microsecond-level precision.
- **Microkernel IPC:** Implements `MsgReceive()` for efficient, non-blocking communication between the Kernel and the Application.
- **Watchdog Mechanism:** A secondary high-priority thread that monitors the "Heartbeat" of the pump controller.
- **Fail-Safe Logic:** Automatically transitions the system to a **[SAFE STATE]** (Halt) if the pump thread fails to respond within 1 second.

## 🚀 How it Works
1. **The Pump Thread:** Runs every 500ms triggered by an OS Pulse. It simulates fluid delivery and updates a global `heartbeat` counter.
2. **The Watchdog Thread:** Acts as a supervisor. It checks the `heartbeat` counter every 1000ms.
3. **Fault Simulation:** After 10 successful cycles, the code intentionally enters an infinite loop to simulate a "Software Hang."
4. **Safety Action:** The Watchdog detects the freeze, triggers an alert, and safely shuts down the system.

## 📸 Simulation Output
![Output Screenshot](output_screenshot.png)

*The screenshot shows 9 successful cycles followed by a simulated freeze and the Watchdog successfully triggering the Safe State.*

## 💻 Tech Stack
- **Operating System:** QNX Neutrino RTOS
- **Language:** Embedded C (POSIX Threads)
- **Development Environment:** QNX Momentics IDE
- **Target:** QNX Virtual Machine (x86_64)

## 📄 How to Run
1. Open **QNX Momentics IDE**.
2. Create a new **QNX C Project**.
3. Replace the source code with `medical_pump.c`.
4. Build and Run on a QNX Target/VM.
