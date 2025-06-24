# ESP32-S3-GPIO-Control-System

> A professional web-based GPIO control system for ESP32-S3 with automatic device discovery and real-time pin manipulation.

## 🚀 Features

### 🔍 Smart Device Discovery
- **Wildcard IP scanning** - Search patterns like `192.168.1.*` or `10.0.*.100`
- **Automatic detection** - Finds all ESP32-S3 devices on your network
- **Batch scanning** - Fast parallel discovery with progress tracking
- **Device information** - Shows name, version, uptime, and memory status

### ⚡ Real-time GPIO Control
- **Individual pin control** - Toggle any GPIO pin HIGH/LOW instantly
- **Pin mode configuration** - Set INPUT, INPUT_PULLUP, or OUTPUT modes
- **PWM control** - Adjust PWM values with real-time sliders (0-255)
- **Bulk operations** - Control multiple pins simultaneously
- **Protected pins** - Safety mechanisms for critical system pins

### 🌐 Modern Web Interface
- **Responsive design** - Works on desktop, tablet, and mobile
- **No installation required** - Runs in any modern web browser
- **Real-time feedback** - Instant visual updates for pin states
- **Toast notifications** - Clear success/error messages
- **Manual refresh control** - Update data when you need it

### 🛡️ Safety & Reliability
- **Protected pin detection** - Prevents control of USB, UART, and Flash pins
- **Input validation** - Comprehensive error checking
- **CORS support** - Proper cross-origin resource sharing
- **Connection status** - Clear indicators for device connectivity

## 📋 Table of Contents

- [Quick Start](#-quick-start)
- [Hardware Requirements](#-hardware-requirements)
- [Installation](#-installation)
- [API Documentation](#-api-documentation)
- [Usage Examples](#-usage-examples)
- [Configuration](#-configuration)
- [Troubleshooting](#-troubleshooting)
- [Contributing](#-contributing)
- [License](#-license)

## 🚀 Quick Start

### 1. Flash the ESP32-S3
```cpp
// Update WiFi credentials in the Arduino code
const char* WIFI_SSID = "Your_WiFi_Name";
const char* WIFI_PASSWORD = "Your_WiFi_Password";
```

### 2. Upload the Code
- Open `esp32_gpio_server.ino` in Arduino IDE
- Select ESP32-S3 board
- Upload to your device

### 3. Open the Web Interface
- Open `gpio_control_interface.html` in your browser
- Enter search pattern (e.g., `192.168.1.*`)
- Click "🔍 Search" to find your device
- Select your ESP32-S3 and click "Connect & Load GPIO"

### 4. Start Controlling!
- Toggle pins HIGH/LOW
- Adjust PWM values with sliders
- Use bulk operations for multiple pins
- Monitor real-time pin states

## 🔧 Hardware Requirements

### Minimum Requirements
- **ESP32-S3** development board
- **WiFi connection** (2.4GHz)
- **Power supply** (USB or external)

### Recommended Specifications
- ESP32-S3-WROOM-1 or ESP32-S3-DevKitC-1
- At least 4MB Flash memory
- Stable 5V power supply for reliable operation

### Compatible Boards
- ESP32-S3-DevKitC-1
- ESP32-S3-WROOM-1-N4
- ESP32-S3-WROOM-1-N8
- Most ESP32-S3 development boards

## 📦 Installation

### Prerequisites
```bash
# Arduino IDE with ESP32 board support
# Required libraries:
- WiFi (built-in)
- WebServer (built-in) 
- ArduinoJson (install via Library Manager)
- SPIFFS (built-in)
```

### Step-by-Step Setup

1. **Clone the Repository**
   ```bash
   git clone https://github.com/alihayajneh/ESP32-S3-GPIO-Control-System.git
   cd ESP32-S3-GPIO-Control-System
   ```

2. **Install Arduino Libraries**
   - Open Arduino IDE
   - Go to Tools → Manage Libraries
   - Search and install "ArduinoJson" by Benoit Blanchon

3. **Configure WiFi**
   ```cpp
   // In esp32_gpio_server.ino, update these lines:
   const char* WIFI_SSID = "Your_WiFi_Network";
   const char* WIFI_PASSWORD = "Your_WiFi_Password";
   ```

4. **Upload to ESP32-S3**
   - Connect ESP32-S3 via USB
   - Select Board: "ESP32S3 Dev Module"
   - Select Port: Your ESP32-S3 port
   - Click Upload

5. **Open Web Interface**
   - Open `gpio_control_interface.html` in any modern browser
   - No server setup required - runs locally!

## 📡 API Documentation

### Base URL
```
http://[ESP32_IP]/api/
```

### Authentication
No authentication required - uses IP-based access control.

### Endpoints

#### Device Information
```http
GET /api/status
```
Returns system status, uptime, memory usage, and device information.

**Response:**
```json
{
  "device": "ESP32-S3-API",
  "version": "2.0.0", 
  "status": "running",
  "uptime_seconds": 3600,
  "free_heap": 245760,
  "wifi_connected": true
}
```

#### GPIO Control
```http
GET /api/gpio
```
Returns status of all GPIO pins.

```http
POST /api/gpio
Content-Type: application/json

{
  "pin": 2,
  "state": true
}
```
Controls individual GPIO pin.

#### PWM Control
```http
POST /api/pwm
Content-Type: application/json

{
  "pin": 2,
  "value": 128,
  "frequency": 1000
}
```
Sets PWM value on specified pin.

#### Bulk Operations
```http
POST /api/bulk
Content-Type: application/json

{
  "pins": [
    {"pin": 2, "state": true},
    {"pin": 4, "state": false},
    {"pin": 5, "mode": "INPUT"}
  ]
}
```
Controls multiple pins in a single request.

### Error Responses
```json
{
  "error": "Invalid pin number",
  "success": false,
  "timestamp": 1634567890
}
```

## 💡 Usage Examples

### Device Discovery
```javascript
// Search for devices on local network
searchPattern: "192.168.1.*"     // Scans 192.168.1.1-255
searchPattern: "192.168.*.*"     // Scans entire 192.168.x.x
searchPattern: "10.0.0.100-110"  // Scans range 10.0.0.100-110
```

### GPIO Control Examples
```cpp
// Set pin 2 HIGH
POST /api/gpio {"pin": 2, "state": true}

// Set pin mode to INPUT_PULLUP  
POST /api/gpio {"pin": 3, "mode": "INPUT_PULLUP"}

// Set PWM on pin 4 to 50% duty cycle
POST /api/pwm {"pin": 4, "value": 128, "frequency": 1000}

// Bulk operation - set multiple pins
POST /api/bulk {
  "pins": [
    {"pin": 2, "state": true},
    {"pin": 4, "state": false}
  ]
}
```

### JavaScript Integration
```javascript
// Connect to device
const response = await fetch('http://192.168.1.100/api/gpio', {
  method: 'POST',
  headers: {'Content-Type': 'application/json'},
  body: JSON.stringify({pin: 2, state: true})
});

const result = await response.json();
console.log(result.message); // "GPIO 2 set to HIGH"
```

## ⚙️ Configuration

### WiFi Settings
```cpp
// Static IP (optional)
IPAddress local_IP(192, 168, 1, 100);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// Configure static IP
WiFi.config(local_IP, gateway, subnet);
```

### GPIO Configuration
```cpp
// Protected pins (cannot be controlled)
const int PROTECTED_PINS[] = {19, 20, 43, 44}; // USB and UART

// Available GPIO pins
const int USABLE_GPIOS[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 
                           12, 13, 14, 15, 16, 17, 18, 21, 35, 36, 
                           37, 38, 39, 40, 41, 42, 45, 46, 47, 48};
```

### Server Configuration
```cpp
// Change server port (default: 80)
WebServer server(8080);

// CORS settings
server.sendHeader("Access-Control-Allow-Origin", "*");
```

## 🐛 Troubleshooting

### Common Issues

#### "Device not found during scan"
- **Check WiFi connection** - Ensure ESP32-S3 is connected to same network
- **Verify IP range** - Use correct subnet for your network
- **Check firewall** - Disable firewall temporarily to test
- **Power cycle** - Reset ESP32-S3 and try again

#### "CORS errors in browser"
- **Check CORS headers** - Ensure server sends proper headers
- **Use updated code** - Latest version includes comprehensive CORS support
- **Try different browser** - Some browsers cache CORS failures

#### "GPIO control not working"
- **Check pin numbers** - Ensure using valid GPIO pins (0-48)
- **Avoid protected pins** - Pins 19, 20, 43, 44 are protected
- **Verify pin mode** - Set to OUTPUT before controlling
- **Check connections** - Ensure proper wiring if using external components

#### "Web interface not loading"
- **Check IP address** - Verify ESP32-S3 IP in serial monitor
- **Network connectivity** - Ensure devices on same network
- **Browser compatibility** - Use modern browser (Chrome, Firefox, Safari, Edge)

### Serial Monitor Debugging
```
ESP32-S3 API Server v2.0.0
✅ WiFi connected successfully!
📍 IP Address: 192.168.1.100
🚀 API Server started on port 80
🌐 CORS fully enabled
✅ Ready for connections!
```

### Network Testing
```bash
# Test device connectivity
ping 192.168.1.100

# Test API endpoint
curl http://192.168.1.100/api/status

# Test from browser console
fetch('http://192.168.1.100/api/status').then(r => r.json()).then(console.log)
```

## 🤝 Contributing

We welcome contributions! Here's how to get started:

### Development Setup
1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Test thoroughly on actual hardware
5. Commit your changes (`git commit -m 'Add amazing feature'`)
6. Push to the branch (`git push origin feature/amazing-feature`)
7. Open a Pull Request

### Contribution Guidelines
- **Test on hardware** - All changes must be tested on real ESP32-S3
- **Follow code style** - Use consistent formatting and naming
- **Update documentation** - Include README updates for new features
- **Add examples** - Provide usage examples for new functionality

### Areas for Contribution
- 📱 Mobile app development
- 🔐 Authentication system
- 📊 Data logging and visualization
- 🏠 Home Assistant integration
- 🐍 Python library for API access
- 📚 Additional documentation and tutorials

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

```
MIT License

Copyright (c) 2025 Dr Ali Hayajneh

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
```

## 🙏 Acknowledgments

- **Espressif Systems** - For the amazing ESP32-S3 platform
- **Arduino Community** - For the comprehensive development environment
- **ArduinoJson Library** - For elegant JSON handling in C++
- **Contributors** - Everyone who has contributed to this project

## 📈 Project Stats

![GitHub stars](https://img.shields.io/github/stars/alihayajneh/ESP32-S3-GPIO-Control-System)
![GitHub forks](https://img.shields.io/github/forks/alihayajneh/ESP32-S3-GPIO-Control-System)
![GitHub issues](https://img.shields.io/github/issues/alihayajneh/ESP32-S3-GPIO-Control-System)
![GitHub last commit](https://img.shields.io/github/last-commit/alihayajneh/ESP32-S3-GPIO-Control-System)

## 🚀 What's Next?

### Planned Features
- [ ] 📱 Mobile app (React Native)
- [ ] 🔐 User authentication system
- [ ] 📊 Real-time data visualization
- [ ] 🏠 Home Assistant integration
- [ ] 🐍 Python SDK
- [ ] 📚 Video tutorials
- [ ] 🔧 GPIO pin templates/presets
- [ ] 📈 Performance monitoring

### Version Roadmap
- **v2.1.0** - Mobile app release
- **v2.2.0** - Authentication system
- **v3.0.0** - Advanced visualization dashboard

---

<div align="center">

**⭐ Star this repo if you find it useful!**

[🐛 Report Bug](https://github.com/alihayajneh/ESP32-S3-GPIO-Control-System/issues) • [✨ Request Feature](https://github.com/alihayajneh/ESP32-S3-GPIO-Control-System/issues)
Made with ❤️ for the ESP32 community

</div>