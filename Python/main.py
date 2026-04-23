import sys
import re
import time
import serial
from collections import deque
from PyQt6.QtWidgets import QApplication, QMainWindow, QVBoxLayout, QHBoxLayout, QWidget, QLabel, QPushButton
from PyQt6.QtGui import QLinearGradient, QColor, QBrush, QIcon
from PyQt6.QtCore import QThread, pyqtSignal
import pyqtgraph as pg

class SerialWorker(QThread):
    # Signal to emit (SpO2, HeartRate) to the main GUI thread
    data_received = pyqtSignal(float, float)

    def __init__(self, port='COM3', baudrate=115200):
        super().__init__()
        self.port = port
        self.baudrate = baudrate
        self.is_running = True

    def run(self):
        try:
            # Using 'with' ensures the serial port is properly closed when exiting
            with serial.Serial(self.port, baudrate=self.baudrate, timeout=1) as ser:
                print(f"Connected to: {ser.name}")

                while self.is_running:
                    if ser.in_waiting > 0:
                        # Read line, handle possible garbage bytes with errors='ignore'
                        line = ser.readline().decode('utf-8', errors='ignore').rstrip()

                        # Regex to find SpO2 and Heart Rate numbers in the string
                        match = re.search(r"SpO2\s*=\s*([\d.]+).*?Heart Rate\s*=\s*([\d.]+)", line)

                        if match:
                            spo2 = float(match.group(1))
                            hr = float(match.group(2))
                            # Emit the parsed values to the GUI
                            self.data_received.emit(spo2, hr)
                        else:
                            # Print lines that don't match the expected format for debugging
                            if line:
                                print(f"Raw data: {line}")

        except Exception as e:
            print(f"Serial Error: {e}")

    def stop(self):
        self.is_running = False
        self.wait()


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("NIMBL - Live Biometrics Monitor")
        self.resize(800, 600)
        self.setWindowIcon(QIcon('Python/logo_dark.png'))

        # Set Dark Mode window style
        self.setStyleSheet("background-color: #121212; color: #ffffff;")

        # Data storage setup (store the last 100 points for a sliding window)
        self.max_points = 100
        self.x_data = deque(maxlen=self.max_points)
        self.spo2_data = deque(maxlen=self.max_points)
        self.hr_data = deque(maxlen=self.max_points)

        # Global stats tracking for lifetime averages
        self.total_spo2_sum = 0.0
        self.total_hr_sum = 0.0
        self.total_readings = 0

        # Main UI layout
        central_widget = QWidget()
        layout = QVBoxLayout()
        central_widget.setLayout(layout)
        self.setCentralWidget(central_widget)

        # PyQtGraph visual configuration
        pg.setConfigOptions(antialias=True)
        pg.setConfigOption('background', '#121212')
        pg.setConfigOption('foreground', '#d3d3d3')

        # 1. Setup SpO2 Header & Plot
        spo2_header_layout = QHBoxLayout()
        spo2_title = QLabel("Blood Oxygen Saturation (SpO2 %)")
        spo2_title.setStyleSheet("font-size: 18px; font-weight: bold; color: #ffffff;")
        self.spo2_stats_label = QLabel("Avg: -- | Min: -- | Peak: --")
        self.spo2_stats_label.setStyleSheet("font-size: 16px; font-weight: bold; color: #ff4444;")

        spo2_header_layout.addWidget(spo2_title)
        spo2_header_layout.addStretch()
        spo2_header_layout.addWidget(self.spo2_stats_label)
        layout.addLayout(spo2_header_layout)

        # Setup SpO2 Gradient Fill
        spo2_grad = QLinearGradient(0, 100, 0, 80)
        spo2_grad.setColorAt(0.0, QColor(255, 68, 68, 100)) # Top is semi-transparent red
        spo2_grad.setColorAt(1.0, QColor(255, 68, 68, 0))   # Bottom is fully transparent
        spo2_brush = QBrush(spo2_grad)

        spo2_time_axis = pg.DateAxisItem(orientation='bottom')
        self.spo2_plot = pg.PlotWidget(axisItems={'bottom': spo2_time_axis})
        self.spo2_plot.showGrid(x=True, y=True, alpha=0.3)
        self.spo2_plot.setYRange(80, 100)  # Standard healthy range view
        # Stack two curves to create a glow effect (thick transparent line + thin solid line)
        self.spo2_glow = self.spo2_plot.plot(pen=pg.mkPen(color=(255, 68, 68, 50), width=12))
        self.spo2_curve = self.spo2_plot.plot(pen=pg.mkPen(color='#ff4444', width=3), fillLevel=80, brush=spo2_brush)
        layout.addWidget(self.spo2_plot)

        # 2. Setup Heart Rate Header & Plot
        hr_header_layout = QHBoxLayout()
        hr_title = QLabel("Heart Rate (BPM)")
        hr_title.setStyleSheet("font-size: 18px; font-weight: bold; color: #ffffff;")
        self.hr_stats_label = QLabel("Avg: -- | Min: -- | Peak: --")
        self.hr_stats_label.setStyleSheet("font-size: 16px; font-weight: bold; color: #44ff44;")

        hr_header_layout.addWidget(hr_title)
        hr_header_layout.addStretch()
        hr_header_layout.addWidget(self.hr_stats_label)
        layout.addLayout(hr_header_layout)

        # Setup Heart Rate Gradient Fill
        hr_grad = QLinearGradient(0, 120, 0, 50)
        hr_grad.setColorAt(0.0, QColor(68, 255, 68, 100)) # Top is semi-transparent green
        hr_grad.setColorAt(1.0, QColor(68, 255, 68, 0))   # Bottom is fully transparent
        hr_brush = QBrush(hr_grad)

        hr_time_axis = pg.DateAxisItem(orientation='bottom')
        self.hr_plot = pg.PlotWidget(axisItems={'bottom': hr_time_axis})
        self.hr_plot.showGrid(x=True, y=True, alpha=0.3)
        self.hr_plot.setYRange(50, 120)  # Standard resting HR range view
        # Stack two curves to create a glow effect
        self.hr_glow = self.hr_plot.plot(pen=pg.mkPen(color=(68, 255, 68, 50), width=12))
        self.hr_curve = self.hr_plot.plot(pen=pg.mkPen(color='#44ff44', width=3), fillLevel=50, brush=hr_brush)
        layout.addWidget(self.hr_plot)

        # Setup Reset Button
        self.reset_button = QPushButton("Reset Data")
        self.reset_button.setStyleSheet("background-color: #333333; color: #ffffff; font-size: 16px; font-weight: bold; padding: 10px; border-radius: 5px;")
        self.reset_button.clicked.connect(self.reset_data)
        layout.addWidget(self.reset_button)

        # 3. Start Background Serial Thread
        self.worker = SerialWorker(port='COM3', baudrate=115200)
        # Connect the thread's signal to our update function
        self.worker.data_received.connect(self.update_plots)
        self.worker.start()

    def update_plots(self, spo2, hr):
        self.x_data.append(time.time())
        self.spo2_data.append(spo2)
        self.hr_data.append(hr)

        # Update global sums for lifetime average calculation
        self.total_spo2_sum += spo2
        self.total_hr_sum += hr
        self.total_readings += 1

        # Update stats text
        if self.spo2_data:
            self.spo2_stats_label.setText(
                f"Avg: {self.total_spo2_sum / self.total_readings:.1f}% | Min: {min(self.spo2_data):.1f}% | Peak: {max(self.spo2_data):.1f}%")
            
        if self.hr_data:
            self.hr_stats_label.setText(
                f"Avg: {self.total_hr_sum / self.total_readings:.1f} BPM | Min: {min(self.hr_data):.1f} BPM | Peak: {max(self.hr_data):.1f} BPM")

        # Update graph data (main line and glow line)
        x_list = list(self.x_data)
        self.spo2_glow.setData(x_list, list(self.spo2_data))
        self.spo2_curve.setData(x_list, list(self.spo2_data))
        
        self.hr_glow.setData(x_list, list(self.hr_data))
        self.hr_curve.setData(x_list, list(self.hr_data))

    def reset_data(self):
        # Clear the arrays
        self.x_data.clear()
        self.spo2_data.clear()
        self.hr_data.clear()

        # Reset lifetime counters
        self.total_spo2_sum = 0.0
        self.total_hr_sum = 0.0
        self.total_readings = 0

        # Clear labels and graphs
        self.spo2_stats_label.setText("Avg: -- | Min: -- | Peak: --")
        self.hr_stats_label.setText("Avg: -- | Min: -- | Peak: --")
        self.spo2_glow.setData([], [])
        self.spo2_curve.setData([], [])
        self.hr_glow.setData([], [])
        self.hr_curve.setData([], [])

    def closeEvent(self, event):
        # Gracefully stop the serial thread before closing
        self.worker.stop()
        event.accept()


if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())