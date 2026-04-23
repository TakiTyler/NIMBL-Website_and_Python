# NIMBL: Base Station Dashboard & Project Website

## Overview
**NIMBL** (Non-Invasive Module for Biomonitoring Life) is a comprehensive, low-cost, and low-power wireless pulse oximeter designed as a Senior Design Project for UCF ECE (Group 30).

This repository houses the frontend components of the project:
1.  **Project Website:** An HTML/CSS landing page showcasing our project overview, team members, and major deliverables (reports, presentations, and demo videos).
2.  **Base Station Dashboard:** A Python-based desktop application that receives, parses, and plots real-time patient biometric data wirelessly transmitted from our custom wearable device.

## NIMBL Libraries

*   [SparkFun_MAX32664_Library](https://github.com/TakiTyler/NIMBL-SparkFun_MAX32664_Library)
*   [HT1621_LCD_Library](https://github.com/TakiTyler/NIMBL-HT1621_LCD_Library)
*   [CC1101_RF_Library](https://github.com/TakiTyler/NIMBL-CC1101_RF_Library)
*   [Website_and_Python](https://github.com/TakiTyler/NIMBL-Website_and_Python)

## Python Dashboard (`main.py`)
The base station software acts as the receiver's user interface. It reads incoming serial data via a connected RF base station (or directly via UART for debugging) and displays it in a clean, modern UI.

### Features
*   **Real-Time Biometric Plotting:** Utilizes `pyqtgraph` to graph Blood Oxygen Saturation (SpO2%) and Heart Rate (BPM) live, maintaining a sliding window of the most recent 100 data points.
*   **Statistical Tracking:** Automatically calculates and displays live statistics, including Lifetime Average, Minimum, and Peak values for both metrics.
*   **Asynchronous Serial Reading:** Implements a dedicated `QThread` (`SerialWorker`) to handle UART communication via `pyserial`. This prevents UI freezing and ensures no packets are dropped while parsing regex-matched payloads.
*   **Modern UI/UX:** Built with PyQt6 featuring a dark mode aesthetic, custom linear gradient chart fills, data-line glow effects, and a responsive layout.

### Installation & Requirements
To run the Python dashboard, you will need Python 3.7+ and the following dependencies:

```bash
pip install PyQt6 pyqtgraph pyserial
```

### Usage
1. Ensure your RF base station (or evaluation board) is connected to your PC via USB.
2. By default, the script targets `COM3` at a baud rate of `115200`. (Update the `SerialWorker` initialization in `main.py` if your port differs).
3. Run the dashboard:
```bash
python main.py
```

## Project Website (`index.html`)
The website serves as the central hub for our project documentation and progress tracking. It includes embedded PDFs of our divide-and-conquer strategy, Critical Design Review (CDR), conference paper, and final video demonstrations.

### Viewing the Site
You can view the live project website here: https://maverick.eecs.ucf.edu/seniordesign/fa2025sp2026/g30/. Alternatively, simply open `index.html` in any modern web browser to view the site locally.

## The NIMBL Ecosystem Repositories
This repository is part of a larger ecosystem of software and hardware designed for this project:
*   SparkFun_MAX32664_Library
*   HT1621_LCD_Library
*   CC1101_RF_Library
*   Website_and_Python (This Repo)

## Group 30 Team Members
*   **Ethan Denner** - Electrical Engineering
*   **Kai Ford** - Electrical Engineering
*   **Tyler Takimoto** - Computer Engineering
*   **Benjamin Will** - Electrical Engineering

*&copy; 2026 NIMBL Project - Group 30. All rights reserved.*
