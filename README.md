# FestoolBlutooth

Theory of operation:
The Festool Remote does some more fancy pairing with the original festool receiver, but importantly, as long as the remote is not paired with a current receiver, everytime you press the button it sends out a couple of BLE advertisement packets. This software takes advantage of that, and fires a relay to turn on the vacuum cleaner.

What do you need:

1. An M5 Atom Socket Kit: https://shop.m5stack.com/collections/atom-series/products/atom-socket-kit-hlw8023-jp-us
2. A working arduino studio installation
3. A USB-c cable to hook to your computer and the M5 device (most M5 devices come with one, but this one doesn't)
4. Arduino configured to work with the M5 Atom
5. The "" library installed within Arduino


How to load the code onto the device:

1. Install Arduino according to these instructions: https://docs.m5stack.com/en/arduino/arduino_development
2. Make sure to install the M5 Atom library too (according to the linked instructions)
3. When you get to the part about board selection, select the Atom.
4. Download the ino file from this repository, and open it up in Arduino
5. Plug in the M5 Socket Kit to your computer.
6. Make sure you've selected the correct port under tools/port.
7. Click sketch, then upload.

How do you use the device:

1. First, factory reset your Festool remote (hold the "man" and "pairing" buttons together until a solid blue light appears. 
2. Plug in the M5 Atom socket, and while doing that hold down the top button. You should see a flashing blue light.
3. Hold down the "pairing" button on remote. If done correctly, the light on the M5 Atom socket should turn off. 
4. Press the "man" button to exit pairing. 

That's it. 

To factory reset the M5 Atom Socket:

1. Plug in the socket, and hold down the button for 6 seconds until you see a flashing white light. 

