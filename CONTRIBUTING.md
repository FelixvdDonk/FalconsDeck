# Contributing to FalconsDeck

Thank you for your interest in contributing to FalconsDeck! This document provides guidelines and information for contributors.

## Getting Started

1. **Fork the repository** on GitHub
2. **Clone your fork** locally
3. **Create a branch** for your feature or bugfix
4. **Make your changes** following our coding standards
5. **Test your changes** thoroughly
6. **Submit a pull request**

## Development Setup

### Prerequisites

- Qt 6.11 or later (with Core, Quick, Bluetooth modules)
- CMake 3.21 or later
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- Linux system with BlueZ (for BLE testing)

### Building

```bash
./build.sh
```

Or manually:

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### IDE Setup

#### Qt Creator
1. Open `CMakeLists.txt` as project
2. Configure with Qt 6.11 kit
3. Build and run

#### VS Code
1. Install CMake Tools extension
2. Select Qt 6.11 kit
3. Configure and build

## Coding Standards

### C++ Style

- **Naming**:
  - Classes: `PascalCase` (e.g., `BleRobotConnection`)
  - Methods: `camelCase` (e.g., `connectToDevice`)
  - Private members: `m_camelCase` (e.g., `m_controller`)
  - Constants: `UPPER_SNAKE_CASE` (e.g., `MAX_ROBOTS`)

- **Formatting**:
  - Indent: 4 spaces (no tabs)
  - Braces: Same line for classes/methods, new line for control structures
  - Line length: Prefer <100 characters, max 120

- **Qt Specifics**:
  - Use Qt containers (`QList`, `QString`, etc.)
  - Prefer `auto` for complex types
  - Use `Q_OBJECT` macro in QObject-derived classes
  - Use `Q_PROPERTY` for QML-exposed properties
  - Use `Q_INVOKABLE` for QML-callable methods

### QML Style

- **Naming**:
  - Components: `PascalCase` (e.g., `RobotCard.qml`)
  - Properties: `camelCase` (e.g., `robotName`)
  - IDs: `camelCase` (e.g., `robotGrid`)

- **Formatting**:
  - Indent: 4 spaces
  - Group properties logically
  - Order: id, properties, signals, functions, child items

- **Best Practices**:
  - Avoid complex JavaScript logic in QML
  - Use anchors over explicit x/y positioning
  - Prefer Layouts over manual positioning
  - Keep QML files focused and single-purpose

### Example C++ Code

```cpp
class ExampleClass : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)

public:
    explicit ExampleClass(QObject *parent = nullptr);
    
    int value() const { return m_value; }
    void setValue(int value);

signals:
    void valueChanged();

private:
    int m_value{0};
};
```

### Example QML Code

```qml
import QtQuick
import QtQuick.Controls

Item {
    id: root
    
    property string title: ""
    signal clicked()
    
    Rectangle {
        anchors.fill: parent
        color: "lightblue"
        
        Text {
            anchors.centerIn: parent
            text: root.title
        }
    }
}
```

## Git Commit Messages

Follow these guidelines:

- **Format**: `<type>: <subject>`
- **Types**: 
  - `feat`: New feature
  - `fix`: Bug fix
  - `docs`: Documentation
  - `style`: Formatting, no code change
  - `refactor`: Code restructuring
  - `test`: Adding tests
  - `chore`: Maintenance

- **Subject**:
  - Use imperative mood ("Add feature" not "Added feature")
  - Don't capitalize first letter
  - No period at the end
  - Max 50 characters

- **Body** (optional):
  - Wrap at 72 characters
  - Explain *what* and *why*, not *how*

### Examples

```
feat: add auto-reconnect functionality

Implements automatic reconnection when a robot disconnects
unexpectedly. Includes exponential backoff and maximum retry limit.

Closes #42
```

```
fix: correct RSSI calculation in BleDeviceScanner

The RSSI value was being negated incorrectly, causing signal
strength to display as positive values.
```

## Pull Request Process

1. **Update documentation** if your changes affect user-facing behavior
2. **Add tests** for new functionality (when test infrastructure exists)
3. **Run existing tests** to ensure no regressions
4. **Update CHANGELOG** (if one exists)
5. **Keep PRs focused** - one feature/fix per PR

### PR Template

When submitting a PR, include:

```markdown
## Description
Brief description of changes

## Motivation
Why is this change needed?

## Testing
How was this tested?

## Checklist
- [ ] Code follows style guidelines
- [ ] Documentation updated
- [ ] Tests added/updated
- [ ] No new warnings
- [ ] Tested on target platform
```

## Areas for Contribution

### High Priority
- [ ] Auto-reconnect functionality
- [ ] Persistent robot configuration
- [ ] Protocol implementation (message framing)
- [ ] Unit tests
- [ ] UI/UX improvements

### Medium Priority
- [ ] Connection statistics dashboard
- [ ] Data logging to file
- [ ] Robot command presets
- [ ] Settings panel
- [ ] Keyboard shortcuts

### Low Priority
- [ ] Themes/customization
- [ ] Robot grouping
- [ ] Advanced BLE features (MTU negotiation)
- [ ] Windows/macOS support

## Testing

### Manual Testing

Before submitting:
1. Test on Linux with BlueZ
2. Test BLE scanning
3. Test connection to real/simulated NUS device
4. Test with multiple connections
5. Test error conditions
6. Check for memory leaks (valgrind)

### Automated Testing (Future)

We aim to add:
- Unit tests for core classes
- QML component tests
- Integration tests
- CI/CD pipeline

## Documentation

Update relevant docs when contributing:
- **README.md**: User-facing features
- **EXAMPLES.md**: Usage examples
- **Architecture comments**: Code documentation
- **QML property docs**: Component APIs

Use Doxygen-style comments for C++:

```cpp
/**
 * @brief Connects to a BLE device
 * @param device The device info from discovery
 * 
 * Initiates a connection to the specified BLE device.
 * The connection is asynchronous; monitor connectionStateChanged()
 * signal for status updates.
 */
void connectToDevice(const QBluetoothDeviceInfo &device);
```

## Questions and Support

- **Issues**: Use GitHub Issues for bugs and feature requests
- **Discussions**: Use GitHub Discussions for questions
- **Email**: Contact maintainers for sensitive topics

## Code of Conduct

### Our Standards

- Be respectful and inclusive
- Welcome newcomers
- Focus on constructive feedback
- Accept criticism gracefully
- Prioritize project success

### Unacceptable Behavior

- Harassment or discrimination
- Trolling or insulting comments
- Publishing private information
- Other unprofessional conduct

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

## Recognition

Contributors will be:
- Listed in CONTRIBUTORS.md (if you create one)
- Credited in release notes
- Thanked in commit messages

## Getting Help

Don't hesitate to ask:
- Open an issue for guidance
- Tag maintainers in discussions
- Join the RoboCup MSL community

Thank you for contributing to FalconsDeck!
