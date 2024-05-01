from bluepy.btle import Scanner, DefaultDelegate
from PyQt5.QtWidgets import QApplication, QLabel, QGridLayout, QVBoxLayout, QWidget
from PyQt5.QtWebEngineWidgets import QWebEngineView 
from PyQt5.QtCore import QThread, pyqtSignal
import sys
import folium
from PyQt5.QtCore import Qt

packet_data = None

origin_set = False

count = 0

global_latitude, global_longitude, global_altitude, global_speed_ns, global_speed_ew, global_speed_vert, global_height_above_takeoff, global_time = 0, 0, 0, 0, 0, 0, 0, 0

class ScanDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)

    def handleDiscovery(self, dev, isNewDev, isNewData):
        global packet_data
        for (adtype, desc, value) in dev.getScanData():
            if value.startswith('000d00'):
                packet_data = value
                self.packet_decode(value)
                break
    
    def packet_decode(self, packet):
        global latitude, longitude

        # Split the packet string into 2-character strings within a list
        packet_list = [packet[i:i+2] for i in range(0, len(packet), 2)]

        speed_ns_hex = packet_list[5]
        speed_ew_hex = packet_list[6]
        speed_vert_hex = packet_list[7]
        lat_hex = packet_list[8:12]
        lon_hex = packet_list[12:16]
        alt_hex = packet_list[16:18]
        hat_hex = packet_list[20:22]
        time_hex = packet_list[24:26]

        # Convert the hexadecimal values to decimal
        speed_ns_val = int(speed_ns_hex, 16)
        speed_ew_val = int(speed_ew_hex, 16)
        speed_vert_val = int(speed_vert_hex, 16)
        lat_val = int("".join(lat_hex), 16)
        lon_val = int("".join(lon_hex), 16)
        alt_val = int("".join(alt_hex), 16)
        hat_val = int("".join(hat_hex), 16)
        time_val = int("".join(time_hex), 16)

        # int8_t convertSpeed(float speed, bool& multiplier_flag) { 

        #     int8_t res;

        #     if ( (abs(speed)/0.25) <= 127 ) {
        #         res = speed / 0.25;
        #         multiplier_flag = 0;
        #     }
        #     if ( (abs(speed)/0.25) > 127 && abs(speed) < 127) {
        #         if (speed < 0) {
        #             res = (speed + (127*0.25)) / 0.75;
        #         }
        #         else {
        #             res = (speed - (127*0.25)) / 0.75;
        #         }
        #         multiplier_flag = 1;
        #     }
        #     if (speed >= 127) {
        #         res = 127;
        #     }

        #     return res;
        # }       

        if lat_val > 2147483647:
            lat_val = lat_val - 4294967296
        if lon_val > 2147483647:
            lon_val = lon_val - 4294967296
        lat_val = round(lat_val / 10000000, 9)
        lon_val = round(lon_val / 10000000, 9)

        alt_val = (alt_val - 1000) * 0.5



        print(f"Latitude: {lat_val}, Longitude: {lon_val}")

        global global_latitude, global_longitude, global_altitude, global_speed_ns, global_speed_ew, global_speed_vert, global_height_above_takeoff, global_time
        global_speed_ns = speed_ns_val
        global_speed_ew = speed_ew_val
        global_speed_vert = speed_vert_val
        global_latitude = lat_val
        global_longitude = lon_val
        global_altitude = alt_val
        global_height_above_takeoff = hat_val
        global_time = time_val


class BluetoothScannerThread(QThread):
    packet_received = pyqtSignal(str)

    def run(self):
        scanner = Scanner().withDelegate(ScanDelegate())
        while True:
            scanner.clear()
            scanner.start()
            scanner.process(0.5)
            scanner.stop()
            if packet_data is not None:
                print("Emitting packet_received signal") # Debugging line
                self.packet_received.emit(packet_data)

class MainWindow(QWidget):
    def __init__(self):
        super().__init__()
        self.initUI()
        self.map_html = None # Store the initial map HTML
        self.map = folium.Map(location=[0, 0], zoom_start=30)
        self.points = []

    def initUI(self):
        main_layout = QGridLayout()

        # Create a QVBoxLayout for vertical arrangement of labels on the left
        label_layout = QVBoxLayout()

        self.label = QLabel(self)
        self.label.setText("TEAM 24030\nRemote ID Drone Detection")
        label_layout.addWidget(self.label)

        # Add more labels to the left side
        self.label1 = QLabel(self)
        self.label1.setText("Label 1")
        label_layout.addWidget(self.label1)

        self.label2 = QLabel(self)
        self.label2.setText("Label 2")
        label_layout.addWidget(self.label2)

        self.label3 = QLabel(self)
        self.label3.setText("Label 3")
        label_layout.addWidget(self.label3)

        self.label4 = QLabel(self)
        self.label4.setText("Label 4")
        label_layout.addWidget(self.label4)

        self.label5 = QLabel(self)
        self.label5.setText("Label 5")
        label_layout.addWidget(self.label5)

        self.label6 = QLabel(self)
        self.label6.setText("")
        label_layout.addWidget(self.label6)

        self.label7 = QLabel(self)
        self.label7.setText("")
        label_layout.addWidget(self.label7)

        self.label8 = QLabel(self)
        self.label8.setText("Label 8")
        label_layout.addWidget(self.label8)

        self.label9 = QLabel(self)
        self.label9.setText("Label 9")
        label_layout.addWidget(self.label9)
                            
        self.label10 = QLabel(self)
        self.label10.setText("Press 'R' to reset the map.\nMap Auto-Resets approximately every 10s.")
        label_layout.addWidget(self.label10)
        
        self.label.setStyleSheet("font-size: 40px; font-weight: bold; color: #AB0520;")
        self.label1.setStyleSheet("font-size: 30px; font-weight: bold; color: #000000;")
        self.label2.setStyleSheet("font-size: 30px; font-weight: bold; color: #000000;")
        self.label3.setStyleSheet("font-size: 30px; font-weight: bold; color: #000000;")
        self.label4.setStyleSheet("font-size: 30px; font-weight: bold; color: #000000;")
        self.label5.setStyleSheet("font-size: 30px; font-weight: bold; color: #000000;")
        self.label6.setStyleSheet("font-size: 30px; font-weight: bold; color: #000000;")
        self.label7.setStyleSheet("font-size: 30px; font-weight: bold; color: #000000;")
        self.label8.setStyleSheet("font-size: 30px; font-weight: bold; color: #000000;")
        self.label9.setStyleSheet("font-size: 15px; font-weight: bold; color: #000000;")
        self.label10.setStyleSheet("font-size: 20px; font-weight: bold; color: #000000;")

        # Add the map view to the right
        self.map_view = QWebEngineView(self)
        main_layout.addLayout(label_layout, 0, 0, 1, 1) # Add the label layout to the main layout, spanning 1 column
        main_layout.addWidget(self.map_view, 0, 1, 1, 2) # Add the map view to the main layout, spanning 2 columns

        self.setLayout(main_layout)

        self.map = folium.Map(location=[0, 0], zoom_start=30)
        self.map_html = self.map._repr_html_()
        self.map_view.setHtml(self.map_html)

    def update_label(self, packet_raw, speed_ns, speed_ew, speed_vert, latitude, longitude, altitude, height_above_takeoff, time):
        
        if self.label9.text() != f"   Raw Message Data:\n   {packet_raw}":
            self.label9.setText(f"   Raw Message Data:\n   {packet_raw}")
            self.label9.setStyleSheet("font-size: 15px; font-weight: bold; color: #0000FF;")
        else:
            self.label9.setStyleSheet("font-size: 15px; font-weight: bold; color: #000000;")
        if self.label1.text() != f"\tSpeed NS:\t{speed_ns} m/s" and speed_ns < 250:
            self.label1.setText(f"\tSpeed NS:\t{speed_ns} m/s")
            self.label1.setStyleSheet("font-size: 30px; font-weight: bold; color: #0000FF;")
        else:
            self.label1.setStyleSheet("font-size: 30px; font-weight: bold; color: #000000;")
        if self.label2.text() != f"\tSpeed EW:\t{speed_ew} m/s" and speed_ew < 250:
            self.label2.setText(f"\tSpeed EW:\t{speed_ew} m/s")
            self.label2.setStyleSheet("font-size: 30px; font-weight: bold; color: #0000FF;")
        else:
            self.label2.setStyleSheet("font-size: 30px; font-weight: bold; color: #000000;")
        if self.label3.text() != f"\tSpeed Vert:\t{speed_vert} m/s":
            self.label3.setText(f"\tSpeed Vert:\t{speed_vert} m/s")
            self.label3.setStyleSheet("font-size: 30px; font-weight: bold; color: #0000FF;")
        else:
            self.label3.setStyleSheet("font-size: 30px; font-weight: bold; color: #000000;")
        if self.label4.text() != f"\tLatitude:\t{latitude}" and latitude > 30:
            self.label4.setText(f"\tLatitude:\t{latitude}")
            self.label4.setStyleSheet("font-size: 30px; font-weight: bold; color: #0000FF;")
        else:
            self.label4.setStyleSheet("font-size: 30px; font-weight: bold; color: #000000;")
        if self.label5.text() != f"\tLongitude:\t{longitude}" and (longitude > -115 or longitude < -100):
            self.label5.setText(f"\tLongitude:\t{longitude}")
            self.label5.setStyleSheet("font-size: 30px; font-weight: bold; color: #0000FF;")
        else:
            self.label5.setStyleSheet("font-size: 30px; font-weight: bold; color: #000000;")
        # if self.label6.text() != f"\tAltitude:\t\t{altitude}":
        #     self.label6.setText(f"\tAltitude:\t\t{altitude}")
        #     self.label6.setStyleSheet("font-size: 30px; font-weight: bold; color: #0000FF;")
        # else:
        #     self.label6.setStyleSheet("font-size: 30px; font-weight: bold; color: #000000;")
        # if self.label7.text() != f"\tHeight Above Takeoff:\t{height_above_takeoff}":
        #     self.label7.setText(f"\tHeight Above Takeoff:\t{height_above_takeoff}")
        #     self.label7.setStyleSheet("font-size: 30px; font-weight: bold; color: #0000FF;")
        # else:
        #     self.label7.setStyleSheet("font-size: 30px; font-weight: bold; color: #000000;")
        if self.label8.text() != f"\tTime:\t{time} cs":
            self.label8.setText(f"\tTime:\t{time} cs")
            self.label8.setStyleSheet("font-size: 30px; font-weight: bold; color: #0000FF;")
        else:
            self.label8.setStyleSheet("font-size: 30px; font-weight: bold; color: #000000;")

    def update_map(self, packet_data):
        print("update_map called with:", packet_data) # Debugging line
        # Update the label with the received packet data
        if (self.label.text() != f"Packet data received: {packet_data}" or self.map_html is None):
            self.update_label(packet_data, global_speed_ns, global_speed_ew, global_speed_vert, global_latitude, global_longitude, global_altitude, global_height_above_takeoff, global_time)

            if len(self.points) == 2:
                self.points.pop(0)
            
            self.points.append([global_latitude, global_longitude])

            folium.PolyLine(self.points, color="blue", weight=12, opacity=0.8).add_to(self.map)

            global count
            if count == 0:
                self.map.location = [global_latitude, global_longitude]
                self.map_html = self.map._repr_html_()
                self.map_view.setHtml(self.map_html)
            
            count += 1
            count = count % 10

    def keyPressEvent(self, event):

        if event.key() == Qt.Key_R:
            # Reset the map to its initial state
            self.reset_map()
            print("Map Reset")

    def reset_map(self):
        # Reset the map's location and zoom level
        self.map.location = [0, 0]
        self.map.zoom_start = 30

        self.points = []

        self.map = folium.Map(location=[0, 0], zoom_start=30)
        self.map_html = self.map._repr_html_()
        self.map_view.setHtml(self.map_html)

def main():
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()

    scanner_thread = BluetoothScannerThread()
    scanner_thread.packet_received.connect(window.update_map)
    scanner_thread.start()

    sys.exit(app.exec_())

if __name__ == "__main__":
    main()
