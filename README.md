# FastECU-m32r-flasher
Tool to flash bricked Subaru Hitachi ECUs

### Supported devices
- WA12212920WWW / UJ20
  - Subaru Impreza MY99-00 Turbo
  - Subaru Legacy B4 Twin Turbo rev. A/B/C
  - Subaru Forester Turbo MY99-00

- WA12212930WWW / UJ30
  - Subaru Legacy 2.0 MY99 N/A

### Future supported devices
- WA12212970WWW / UJ70 (K-Line)
- WA12212970WWW / UJ70 (CAN)

--------

## Using FastECU-m32r-flasher

### WA12212920WWW / UJ20
- Connect +12v and GND to ECU power pins (IGN SW + CONTROL POWER and GND) ready to apply power
- Connect FTDI / CH340-based OBD adapter via K-Line to ECU K-Line pin, +12v and GND pins to same power as ECU
- Connect wire to VPP (TP13 on bottom of PCB) to be ready to connect +12v
- Connect wire to MOD1 (TP92 on bottom of PCB or HL06 on top) to be ready to connect +12v

Run software and apply power to ECU. Connect VPP wire to +12v and after that MOD1 wire to +12v. Select file to upload with "Select File"-button and press "Write Flash". When messagebox appears, remove MOD1 wire from +12v and press ok. Take a zip from coffee and watch your ECU to come alive again :)

### WA12212930WWW / UJ30
- Connect +12v and GND to ECU power pins (IGN SW + CONTROL POWER and GND) ready to apply power
- Connect FTDI / CH340-based OBD adapter via K-Line to ECU K-Line pin, +12v and GND pins to same power as ECU
- Connect wire to VPP TP16 on bottom of PCB or some via on top/bottom of PCB to be ready to connect +12v
- Connect wire to MOD1 (HL14 on top/bottom of PCB) to be ready to connect +12v

Run software and apply power to ECU. Connect VPP wire to +12v and after that MOD1 wire to +12v. Select file to upload with "Select File"-button and press "Write Flash". When messagebox appears, remove MOD1 wire from +12v and press ok. Take a zip from coffee and watch your ECU to come alive again :)
