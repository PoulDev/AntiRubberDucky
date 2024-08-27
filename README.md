# AntiRubberDucky
AntiRubberDucky is a Linux tool designed to detect and mitigate Rubber Ducky attacks.

## Installation
```bash
git clone https://github.com/PoulDev/AntiRubberDucky
cd AntiRubberDucky
chmod u+x ./install.sh
sudo ./install.sh
```

## How it works
AntiRubberDucky uses the udev linux API to continuously monitor usb devices.
If a new [HID (Human Interface Device)](https://en.wikipedia.org/wiki/Human_interface_device) has been connected, it listens for keypress events, if the device is typing at a speed that isn't humanly possible, it instantly disconnects it, mitigating any rubber ducky attack.

## Can't a Rubber Ducky just type slower?
Yes, by making a script that types at human speed the program will not detect it as a rubber ducky, but by doing so, you are taking away half of the features that make it such an effective hacking device.
