# Needed Electronics
- NodeMcu Lolin V3 ESP32 [Link](https://www.ebay.com/itm/NodeMCU-V3-ESP8266-ESP-12-E-Lua-CH340-WiFI-WLan-IoT-Lolin-Mini-N3A2-V9T3/312753940110)
- Led strip: WS2812B [Link](https://www.ebay.com/itm/1-5m-WS2812B-RGB-Led-Strip-Light-30-60-144Leds-m-Individual-Addressable-IC-DC5V/401755047203?var=671534865138)
- LDR: 5MM GL5537 [Link](https://www.ebay.com/itm/20pcs-Photoresistor-5MM-GL5537-LDR-Photo-Resistors-Light-Dependent-Resistor/381374819080?epid=1381908731&hash=item58cbb5eb08:g:INIAAOxyYSdTAJq4)

# Wirring
- ESP(Rx) -> LedStrip (Data)
- ESP(GND) -> LedStrip (GND)
- ESP(VU) -> LedStrip (5v)
- LDR(1) -> ESP(3v)
- LDR(2) -> ESP(A0)

# Hardware
### RIBBA 23x23
- 23x23=60 Leds per meter
    - 3 meter for 11x11 leds (2.01m)
- MDF plate 23*23 
- Ikea RIBBA frame 23cm [link](https://www.ikea.com/nl/nl/p/ribba-fotolijst-zwart-40378401/)

### RIBBA 50x50
- 30 Leds per meter
    - 6 meter for 13x13 leds (5.7m)
    - 5 meter for 11x11 leds (4.04m)
- MDF plate 50x50
- Ikea RIBBA frame 50cm [link](https://www.ikea.com/nl/nl/p/ribba-fotolijst-zwart-00378436/)

### Frameless
- 30 Leds per meter 6 meter for 13x13 leds (5.7m)
- MDF plate 50x50
- letterplate (with connection holes)
- light divider (with hanging hole)
- led panel on hardboard

#### Assemble
- Create the led-panel by cutting out a square from a hardboard pannel
- Place the led-panel en light divider on eatch other
- Trace the holes from the light divider on the led panel hardboard
- Place the led-strips centerd on the taces on the hardboard by following the this pattern:
    - Start upper left with Din end place it to the upper Right side end with Dout
    - The second row start with Dout on the left side and ends with Din
    - continue this zigzag pattern until all rows are done
- Use a Flatcable to connect the ledstrips from Dout to Din.
    - Cut flatcable, strip and tin both ends, cut the ends to they are separeted from eatch other
    - pre-solder both ends of the ledstrip
    - Start upper right with Dout solder all 3 pins(GND, Dout, 5V) Fold the flacable 90 degrees down, fold it 90 degrees towarts Din Solder again

- Drill (2mm) hole through both panels in between 4 led holes, from the front to the back. These hols can later be used to connect both pannels
- Connect the ESP and test the led pannel
- Connect the letterplate with the light divider
- Connect the light divider with the led panel with screws (2,5mm)

# Tips & tricks:
- The MDF plate can also be replaced by "Geschuimd pvc"

- 

# Note:
The NodeMcu Lolin V3 is needed because the Lolin has a 3v reference voltage for its adc that is used for the LDR all other NodeMC have 1v reference voltage

## Kosten:
- ESP: €3
- LDR: €0.1
- LED strip: €10.0/€12.0
- Ikea RIBBA: €5.99/€9.99 
- MDF: €10.0
- letterplaat
    - lazersnijden: €24.0/€45.0
