# FalconsDeck

Qt6 BLE Robot Monitor for Steam Deck — RoboCup Middle Size League

A QtQuick application for monitoring and commanding up to 16 football robots over Bluetooth Low Energy (Nordic UART Service).

## Features

- **BLE Device Discovery**: Scan and discover Bluetooth Low Energy devices
- **Multiple Connections**: Support for up to 16 simultaneous robot connections
- **Nordic UART Service (NUS)**: Serial-over-BLE communication using standard NUS UUIDs
- **Real-time Monitoring**: Live RSSI, connection state, and data reception display
- **Touch-Optimized UI**: Designed for Steam Deck (1280x800) with touch-friendly controls
- **Independent Connection Management**: Each robot has its own `QLowEnergyController` instance

## Requirements

### Build Requirements
- **Qt 6.11 or later** with the following components:
  - Qt Core
  - Qt Quick
  - Qt Bluetooth
- **CMake 3.21 or later**
- **C++17 compatible compiler**

### Runtime Requirements
- **Linux** (tested on Arch-based systems like Steam Deck)
- **BlueZ 5.x** for Bluetooth Low Energy support
- **D-Bus** (required by Qt Bluetooth on Linux)

## Building

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build
cmake --build .

# Run
./FalconsDeck
```

### Building with a specific Qt version

If you have multiple Qt versions installed:

```bash
cmake -DCMAKE_PREFIX_PATH=/path/to/Qt/6.11.0/gcc_64 ..
```

## Architecture

### Directory Structure

```
FalconsDeck/
├── CMakeLists.txt          # Build configuration
├── src/                    # C++ source files
│   ├── main.cpp           # Application entry point
│   ├── ble/               # Bluetooth Low Energy layer
│   │   ├── BleDeviceScanner.*      # Device discovery
│   │   ├── BleRobotConnection.*    # Single robot connection
│   │   └── BleConnectionManager.*  # Multi-robot management
│   ├── models/            # Data models
│   │   ├── Robot.*                 # Robot state data
│   │   └── RobotListModel.*        # Qt model for QML
│   └── protocol/          # Communication protocol
│       └── PacketInterface.*       # Packet abstraction
├── qml/                   # QML UI files
│   ├── Main.qml           # Main application window
│   ├── ScannerView.qml    # BLE scanning interface
│   ├── RobotDashboard.qml # 4x4 robot grid view
│   └── RobotCard.qml      # Individual robot card
└── resources.qrc          # Qt resources
```

### Key Components

#### BleDeviceScanner
Wraps `QBluetoothDeviceDiscoveryAgent` to discover BLE devices. Filters for Low Energy devices and provides a list for QML consumption.

**QML Properties:**
- `scanning` (bool): Current scanning state
- `discoveredDevices` (list): Array of discovered devices with name, address, and RSSI

**QML Methods:**
- `startScan()`: Begin BLE device discovery
- `stopScan()`: Stop scanning
- `getDeviceByAddress(address)`: Retrieve device info for connection

#### BleRobotConnection
Manages a single robot connection using `QLowEnergyController`. Handles the complete Nordic UART Service lifecycle:
1. Connect to device
2. Discover services
3. Find NUS service (UUID: `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`)
4. Locate RX/TX characteristics
5. Enable notifications on TX characteristic
6. Ready for bidirectional communication

**Properties:**
- `connectionState`: Disconnected | Connecting | Connected | Ready | Error
- `robotName`: Device name
- `robotAddress`: Bluetooth address
- `rssi`: Signal strength
- `lastError`: Most recent error message

**Methods:**
- `sendData(QByteArray)`: Send data to robot (auto-chunks for BLE MTU)

#### BleConnectionManager
Orchestrates up to 16 `BleRobotConnection` instances. Provides high-level robot fleet management.

**QML Properties:**
- `robotListModel`: QAbstractListModel of all connected robots
- `connectedCount`: Number of robots in Ready state

**QML Methods:**
- `connectRobot(device)`: Create new connection
- `disconnectRobot(index)`: Disconnect specific robot
- `disconnectAll()`: Disconnect all robots
- `sendToRobot(index, data)`: Send to specific robot
- `sendToAll(data)`: Broadcast to all robots

#### RobotListModel
Qt item model exposing robot data to QML with roles:
- `name`: Robot name
- `address`: Bluetooth address
- `connectionState`: Connection status string
- `rssi`: Signal strength
- `lastData`: Last received packet (hex format)
- `lastDataTime`: Timestamp of last reception

## Nordic UART Service (NUS)

The application uses the standard Nordic UART Service for serial-over-BLE communication:

- **Service UUID**: `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`
- **RX Characteristic** (write to robot): `6E400002-B5A3-F393-E0A9-E50E24DCCA9E`
  - Properties: Write, WriteWithoutResponse
  - Used to send commands/data to the robot
- **TX Characteristic** (receive from robot): `6E400003-B5A3-F393-E0A9-E50E24DCCA9E`
  - Properties: Notify
  - Used to receive data from the robot

### MTU Handling
The application automatically chunks outgoing data into 20-byte segments (default BLE MTU) to ensure compatibility. MTU negotiation could be added for higher throughput.

## User Interface

### Scanner View
- **Scan Button**: Toggle BLE device discovery
- **Device List**: Shows discovered BLE devices with:
  - Device name
  - Bluetooth address
  - RSSI (color-coded by signal strength)
  - Connect button
- **Status**: Scanning indicator and device count

### Robot Dashboard
- **4x4 Grid**: Displays up to 16 connected robots
- **Robot Cards**: Each card shows:
  - Robot name/number
  - Connection state (color-coded border)
  - Bluetooth address
  - RSSI
  - Last received data (hex format)
  - Last data timestamp
  - Disconnect button
- **Control Panel**:
  - Text input for commands/data
  - "Send to Selected" button (sends to highlighted robot)
  - "Send to All" button (broadcasts to all connected robots)

### Color Coding
- **Green**: Ready (NUS service active, can send/receive)
- **Yellow**: Connected (connected but discovering services)
- **Cyan**: Connecting (establishing connection)
- **Red**: Error (connection failed or lost)
- **Gray**: Disconnected

## Usage

1. **Launch the application**
   ```bash
   ./FalconsDeck
   ```

2. **Scan for robots**
   - Start on Scanner View
   - Press "Start Scan"
   - Wait for robots to appear in the list

3. **Connect to robots**
   - Press "Connect" on desired robots
   - Application will switch to Dashboard automatically
   - Monitor connection progress via card color

4. **Send commands**
   - Select a robot by clicking its card
   - Type text or hex data in the input field
   - Click "Send to Selected" or "Send to All"

5. **Monitor data**
   - Received data appears in each robot's card
   - RSSI updates show connection quality
   - Timestamp shows last communication time

## Troubleshooting

### Bluetooth Adapter Not Found
- Ensure Bluetooth is enabled: `bluetoothctl power on`
- Check BlueZ is running: `systemctl status bluetooth`

### Device Not Appearing in Scan
- Ensure robot is advertising and in range
- Some devices require pairing before connecting
- Check that the device is advertising as BLE (not classic Bluetooth)

### Connection Fails
- Move closer to the robot (improve RSSI)
- Ensure robot's NUS service is active
- Check `dmesg` for kernel Bluetooth errors

### No Data Received
- Verify robot is sending notifications on TX characteristic
- Check the robot's firmware is correctly implementing NUS
- Use `bluetoothctl` or `gatttool` to test manually

## Future Enhancements

### Planned Features
- [ ] Auto-reconnect on connection loss
- [ ] Persistent robot configuration (save/load)
- [ ] Custom protocol implementation beyond raw bytes
- [ ] Robot command presets/macros
- [ ] Log data to file
- [ ] Connection statistics (uptime, packet count)
- [ ] Batch operations (connect all, configure all)
- [ ] Robot grouping/tagging

### Protocol Development
The `PacketInterface` class provides a foundation for implementing a custom protocol:
- Add message framing (length prefix, checksums)
- Define command/response structure
- Implement request/reply patterns
- Add protocol versioning

## License

This project is developed for the RoboCup Middle Size League Falcons team.

## Contributing

Contributions are welcome! Please ensure:
- Code follows Qt/C++ best practices
- QML follows QtQuick guidelines
- Test on actual BLE hardware when possible
- Document any protocol additions

## References

- [Nordic UART Service (NUS) Specification](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/libraries/bluetooth_services/services/nus.html)
- [Qt Bluetooth Documentation](https://doc.qt.io/qt-6/qtbluetooth-index.html)
- [RoboCup Middle Size League](https://wiki.robocup.org/wiki/Middle_Size_League)
