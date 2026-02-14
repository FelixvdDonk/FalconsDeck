# FalconsDeck Quick Start Guide

Get up and running with FalconsDeck in 5 minutes.

## Prerequisites Check

```bash
# Check Qt installation
qmake6 --version  # or qmake --version

# Check CMake
cmake --version  # Need 3.21 or later

# Check Bluetooth
bluetoothctl
> power on
> scan on
```

## Build

```bash
# Clone the repository
git clone https://github.com/FelixvdDonk/FalconsDeck.git
cd FalconsDeck

# Build (automatically creates build directory)
./build.sh

# Or manually:
mkdir build && cd build
cmake ..
cmake --build .
```

## First Run

```bash
cd build
./FalconsDeck
```

## First Steps

1. **Scanner View** opens automatically
2. Click **"Start Scan"**
3. Wait for BLE devices to appear
4. Click **"Connect"** next to a device
5. Switch to **Dashboard** using the button in top toolbar
6. Monitor connection status (card turns green when ready)

## Send Your First Command

1. In Dashboard, click a robot card to select it
2. Type in the text field: `HELLO`
3. Click **"Send to Selected"**
4. Watch for response in the robot's card

## Quick Tips

- **Green border** = Ready to communicate
- **Yellow border** = Connected but initializing
- **Cyan border** = Connecting
- **Red border** = Error
- **Gray border** = Disconnected

- **RSSI**: Signal strength indicator
  - \> -70 dBm = Excellent
  - -70 to -85 dBm = Good
  - < -85 dBm = Weak (may disconnect)

## Common First-Time Issues

### "No devices found"
```bash
# Make sure Bluetooth is on
bluetoothctl
> power on
> scan on

# Check if your BLE device appears
# If it does, FalconsDeck should find it too
```

### "Connection fails"
- Make sure device has Nordic UART Service
- Check device isn't connected to another device
- Move closer to device (< 5m for first connection)

### "Can't build"
```bash
# Install Qt 6.11
# Ubuntu/Debian:
sudo apt install qt6-base-dev qt6-declarative-dev qt6-bluetooth-dev

# Arch (Steam Deck):
sudo pacman -S qt6-base qt6-declarative qt6-connectivity

# Then try build again
./build.sh
```

## Robot Requirements

Your robot must:
- Support Bluetooth Low Energy (BLE 4.0+)
- Advertise Nordic UART Service UUID: `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`
- Have RX characteristic: `6E400002-B5A3-F393-E0A9-E50E24DCCA9E` (write)
- Have TX characteristic: `6E400003-B5A3-F393-E0A9-E50E24DCCA9E` (notify)

## Next Steps

- Read [README.md](README.md) for full documentation
- Check [EXAMPLES.md](EXAMPLES.md) for testing scenarios
- See [CONTRIBUTING.md](CONTRIBUTING.md) if you want to contribute

## Support

- Issues: https://github.com/FelixvdDonk/FalconsDeck/issues
- Discussions: https://github.com/FelixvdDonk/FalconsDeck/discussions

## Steam Deck Specific

### Gaming Mode
The app should work in Gaming mode. Add it as a non-Steam game:
1. Copy FalconsDeck to `~/.local/share/applications/`
2. In Gaming Mode: Add Non-Steam Game
3. Browse to FalconsDeck binary

### Desktop Mode
Just run normally:
```bash
cd ~/FalconsDeck/build
./FalconsDeck
```

### Permissions
If you have Bluetooth permission issues:
```bash
# Add yourself to bluetooth group
sudo usermod -a -G bluetooth $USER

# Logout and login again
```

Enjoy using FalconsDeck! 🚀
