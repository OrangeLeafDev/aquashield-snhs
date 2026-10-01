# AQUASHIELD Microcontroller Source Code
## Instructions
Plug in the appropriate sensors to their assigned pinouts. You can customize them in the code directly.

For UART, connect UNO's Pin 3 to ESP32's Pin 16, and UNO's Pin 2 to ESP32's Pin 17.

⚠️IMPORTANT⚠️: Step down the voltage of UNO's Pin 3 so that the 5v output becomes 3.3v
## Uno Code default parameters
UART
- Timeout (MS) = 1000 ms
- RX Pin Number = 2
- TX Pin Number = 3

Ultrasonic
- Trigger Pin Number = 9
- Echo Pin Number = 10

Water
- Analog Out Pin Number = A0

L298N Motor Driver
- IN1 Pin Number = 5
- IN2 Pin Number = 6
- EN1 Pin Number = 9
- Motor Reverse = False

## ESP32 Code default parameters
UART
- RX Pin Number = 16
- TX Pin Number = 17
- Ping Interval (MS) = 2000
- Ping Timeout (MS) = 500

Networking
- SSID = "[redacted]" (name of the wifi)
- Password = "[redacted]" (password of the wifi)
- Port = 80 (server port)
- Local IP = 192.168.100.245
- Gateway = 192.168.100.1
- Subnet = 255.255.255.0

## Web Server API
Data can be fetched from `http://[ESP32-IP-ADDRESS]/state` via an HTTP request

The following parameters are expected:
```
distance - Float (in centimeters)
water    - Float (in raw analog readings)
isonline - Boolean (true & false value)
motor    - Integer (0 or 1)
status   - String
