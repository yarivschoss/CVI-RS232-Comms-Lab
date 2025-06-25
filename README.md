# 🔄 RS‑232 Communication Lab – LabWindows/CVI

[![Toolchain](https://img.shields.io/badge/LabWindows--CVI-2020%2B-blue)](#building)
[![Language](https://img.shields.io/badge/C-ANSI-green)](#source-files)
[![License](https://img.shields.io/badge/License-MIT-lightgrey)](LICENSE)

> **Course:** CVI Programming & Instrumentation (Afeka College, 2025)  
> **Author:** Yariv Shossberger

---

## 📑 Table of Contents
1. [Project Overview](#project-overview)  
2. [Key Features](#key-features)  
3. [GUI Panels](#gui-panels)  
4. [Communication Modes](#communication-modes)  
5. [Building & Running](#building--running)  
6. [Repository Layout](#repository-layout)  

---

## Project Overview
This LabWindows/CVI application demonstrates **bidirectional RS‑232 data exchange** using two **virtual COM ports (COM1 ↔ COM2)** bridged by *Virtual Serial Port Tools*.  
It supports three data modes and visualises the traffic live on a digital analyser panel.

---

## Key Features
* **String I/O** – polling *or* interrupt driven, optional parity error‑correction toggle. 
* **Numeric I/O** – full‑duplex double precision numbers via receive interrupt. 
* **Binary Image Transfer** – chunked send/receive with dynamic bitmap reconstruction on a canvas. 
* **Digital Bit Analyser** – plots raw UART bits for string mode using `PlotDigitalLines`.  
* **Configurable polling rate** (timer) and on‑the‑fly parity re‑initialisation.  
* **Thread pool + locks** for safe background polling and GUI responsiveness.  

---

## GUI Panels
| Panel | Purpose |
|-------|---------|
| **COM1** | Main control: choose mode, enter data, load image, set polling & parity |
| **COM2** | Displays incoming data (string / numeric) or received image |
| **Analyzer** | Digital waveform of bits in *String* mode |

*(All panels are defined in **`EX8.uir`**.)*

---

## Communication Modes
| Mode | How it works |
|------|--------------|
| **String** | User types text → `SendCallback` writes to COM1, reads echo from COM2 and plots bits. Optional **polling**: two threads periodically write/read both ports via `comThread` and a GUI timer. |
| **Numeric** | Numeric control value formatted as ASCII, sent on COM1; COM2 receive interrupt (`readInterrupt`) updates display. |
| **Binary Image** | Selected bitmap is serialised and chunk‑written (`SendImageAsync`). COM2 interrupt (`receiveImage`) rebuilds the image and draws it on the canvas once complete. |

All modes rely on the helper `initilizeComPorts()` which opens both ports at **9600 Bd, 7E1/7O1/7N1** depending on the parity setting fileciteturn12file0.

---

## Building & Running
### Prerequisites
* **LabWindows/CVI 2019 or newer**  
* **Virtual Serial Port Tools** (or any loopback hardware) – create a bridge **COM1 ↔ COM2**  
* Windows 10/11

### Steps
1. Clone / download this repo.  
2. Open **`EX8.prj`** or workspace **`EX8.cws`** in CVI.  
3. Ensure the bridged ports are named **COM1** and **COM2** or edit `def.h` → `COM_PORT_1/2`.  
4. *Build → Run* (F5).  
5. Use the *Menu → Communication Type* to switch modes, then interact with the controls.

### Troubleshooting
* “Failed to open COMx” → Check bridge is active and not in use by other apps.  
* Image transfer stalls → Increase queue size constants `Q_SIZE` in `def.h` or lower baud.

---

## Repository Layout
```
CVI_RS232_Lab/
├── src/
│   ├─ EX8.c          # main program & callbacks
│   ├─ EX8.h
│   ├─ def.h          # shared structs, helpers, constants
├── ui/
│   └─ EX8.uir        # GUI panels and controls
├── docs/
│   └─ Targil08.pdf   # original assignment spec
└── README.md
```
