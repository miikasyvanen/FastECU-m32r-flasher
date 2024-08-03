# FastECU-m32r-flasher
Tool to flash bricked Subaru Hitachi ECUs

### Supported devices
- WA12212920WWW / UJ20 (M32150)

### Future supported devices
- WA12212930WWW / UJ30 (M321xx)
- WA12212970WWW / UJ70 (M3217x)

### UJ20 usage
- Connect +12v and GND to ECU power pins (IGN SW + CONTROL POWER and GND) ready to apply power
- Connect FTDI / CH340-based OBD adapter via K-Line to ECU K-Line pin, +12v and GND pins to same power as ECU
- Connect wire to VPP (TP13 on bottom of PCB) to be ready to connect +12v
- Connect wire to MOD1 (TP92 on bottom of PCB or HL06 on top) to be ready to connect +12v

Run software and apply power to ECU. Connect VPP wire to +12v and after that MOD1 wire to +12v. Select file to upload with "three dot"-button next to "Write Flash"-button. Press "Write Flash". When message box appears, remove MOD1 wire from +12v and press ok. Take a zip from coffee and watch your ECU to come alive again :)
