# FalconsDeck Examples and Testing

This document provides examples and testing guidance for FalconsDeck.

## Testing Without Physical Robots

### Using BlueZ Test Tools

You can test the BLE scanning functionality using BlueZ test tools on Linux:

```bash
# Start bluetoothd in test mode (if you have permissions)
sudo bluetoothd -d

# Use bluetoothctl to check adapter
bluetoothctl
> power on
> scan on
```

### Simulating BLE Devices

On a separate Linux machine or Raspberry Pi, you can simulate a Nordic UART Service device using Python and BlueZ:

```bash
# Install dependencies
sudo apt-get install python3-pip
pip3 install bluezero

# TODO: Add Python script to simulate NUS service
```

## Example Usage Scenarios

### Scenario 1: Single Robot Connection

1. Launch FalconsDeck
2. Click "Start Scan" in Scanner View
3. Wait for your robot to appear (e.g., "Robot-01")
4. Click "Connect" next to the robot
5. Switch to Dashboard view
6. Verify connection state is "Ready" (green border)
7. Type a test command: "HELLO"
8. Click "Send to Selected"
9. Monitor the robot card for received data

### Scenario 2: Multiple Robot Fleet

1. Launch FalconsDeck
2. Start scanning
3. Connect to multiple robots (up to 16)
4. Switch to Dashboard
5. Type broadcast command: "STATUS?"
6. Click "Send to All"
7. Watch all robot cards for responses
8. Select individual robot by clicking its card
9. Send specific command to that robot

### Scenario 3: Connection Recovery

1. Connect to several robots
2. Physically move one robot out of range or power it off
3. Observe connection state changes to "Error" or "Disconnected"
4. Bring robot back in range
5. Manually reconnect from Scanner view
   (Note: Auto-reconnect not yet implemented)

## Protocol Testing

### Sending Raw Bytes

The application currently sends text as UTF-8 bytes. To send specific byte sequences:

**From QML Console (during development):**
```javascript
// Send hex bytes
var data = new ArrayBuffer(4)
var view = new Uint8Array(data)
view[0] = 0x01
view[1] = 0x02
view[2] = 0x03
view[3] = 0x04
connectionManager.sendToRobot(0, data)
```

**Future Enhancement:** Add hex input mode in the UI for easier testing.

### Receiving Data

Data received from robots is displayed in hex format in each robot card:
- Format: `01:02:03:04`
- Updates in real-time
- Shows timestamp of last reception

## Performance Testing

### Connection Stress Test

Test maximum connection capacity:

1. Ensure 16 BLE devices are available
2. Connect to all 16 sequentially
3. Monitor system resource usage
4. Verify all reach "Ready" state
5. Send broadcast message
6. Check all devices receive it within acceptable latency

Expected behavior:
- All 16 connections should succeed
- CPU usage should remain reasonable (<50% on Steam Deck)
- Memory usage stable
- UI remains responsive

### Range Testing

Test connection stability at various distances:

1. Connect to a robot at close range (<1m)
2. Gradually increase distance
3. Monitor RSSI values in real-time
4. Note where RSSI drops below -85 dBm (weak signal)
5. Continue until connection drops
6. Document maximum reliable range

Typical ranges:
- Strong signal (>-70 dBm): 0-5m
- Moderate signal (-70 to -85 dBm): 5-10m
- Weak signal (<-85 dBm): 10-15m, may be unstable

## Debugging

### Enable Qt Bluetooth Debug Logs

Set environment variable before launching:

```bash
export QT_LOGGING_RULES="qt.bluetooth*=true"
./FalconsDeck
```

### Common Issues

**Issue: No devices appear in scan**
- Check: `bluetoothctl` shows devices?
- Check: Adapter powered on?
- Check: Robot advertising enabled?

**Issue: Connection fails after "Connecting"**
- Check: Robot has Nordic UART Service
- Check: Correct UUID: `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`
- Check: Robot not already connected to another device

**Issue: Data not received**
- Check: Notifications enabled on TX characteristic
- Check: Robot actually sending data
- Use `gatttool` to verify manually

**Issue: UI freezes during scan/connect**
- Check: Not running on main thread (should be async)
- Check: Qt event loop running
- Report as bug if reproducible

## Testing Checklist

Before deployment, verify:

- [ ] BLE adapter detected and powered on
- [ ] Scan discovers BLE devices
- [ ] Can connect to NUS-enabled device
- [ ] Connection state changes reflected in UI
- [ ] Can send data to connected robot
- [ ] Can receive data from robot
- [ ] Data displayed correctly in hex format
- [ ] Can connect to multiple robots simultaneously
- [ ] Can disconnect individual robot
- [ ] Can disconnect all robots
- [ ] Broadcast sends to all connected robots
- [ ] RSSI updates in real-time
- [ ] UI responsive during all operations
- [ ] No memory leaks during connect/disconnect cycles
- [ ] Works on Steam Deck in gaming mode
- [ ] Touch input works for all controls

## Development Testing

### Mock Robot Interface

For C++ unit testing, create a mock `BleRobotConnection`:

```cpp
class MockBleRobotConnection : public BleRobotConnection {
    Q_OBJECT
public:
    void connectToDevice(const QBluetoothDeviceInfo &device) override {
        emit connectionStateChanged();
        // Simulate successful connection after delay
        QTimer::singleShot(100, this, [this]() {
            setConnectionState(Robot::Ready);
        });
    }
    
    void sendData(const QByteArray &data) override {
        m_sentData.append(data);
        // Simulate echo response
        emit dataReceived(data);
    }
    
    QByteArray m_sentData;
};
```

### QML Test Cases

Test QML UI components with Qt Quick Test:

```qml
import QtQuick
import QtTest

TestCase {
    name: "RobotCardTests"
    
    RobotCard {
        id: card
        robotName: "Test Robot"
        connectionState: "Ready"
    }
    
    function test_colorCoding() {
        card.connectionState = "Ready"
        compare(card.border.color, "#00ff00")
        
        card.connectionState = "Error"
        compare(card.border.color, "#ff0000")
    }
}
```

## Future Test Requirements

When implementing new features, ensure:

1. **Auto-reconnect**: Test connection recovery after interruption
2. **Persistent Settings**: Test save/load of robot configurations
3. **Protocol Implementation**: Unit test packet framing/parsing
4. **Performance**: Benchmark throughput and latency
5. **Battery Impact**: Monitor power consumption on Steam Deck
6. **Error Recovery**: Test all error conditions gracefully handled

## Contributing Test Cases

When submitting PRs:
- Include test cases for new functionality
- Verify existing tests still pass
- Document any new testing procedures
- Update this file with new examples
