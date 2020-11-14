# Needed Electronics
- NodeMcu Lolin V3 ESP32 [Link](https://www.ebay.com/itm/NodeMCU-V3-ESP8266-ESP-12-E-Lua-CH340-WiFI-WLan-IoT-Lolin-Mini-N3A2-V9T3/312753940110)
- Led strip: WS2812B [Link](https://www.ebay.com/itm/1-5m-WS2812B-RGB-Led-Strip-Light-30-60-144Leds-m-Individual-Addressable-IC-DC5V/401755047203?var=671534865138)
- LDR: 5MM GL5537 [Link](https://www.ebay.com/itm/20pcs-Photoresistor-5MM-GL5537-LDR-Photo-Resistors-Light-Dependent-Resistor/381374819080?epid=1381908731&hash=item58cbb5eb08:g:INIAAOxyYSdTAJq4)
- 100kOhm Resistor [link](https://www.ebay.com/itm/100PCS-1-4W-0-25W-Metal-Film-Resistor-1-Full-Range-of-Values-0-to-10M/252838503547?hash=item3ade59647b:m:m9AAPzkedx9P_upvXOpgF9Q) 

# Wiring
- ESP(Rx) -> LedStrip (Data)
- ESP(GND) -> LedStrip (GND)
- ESP(VU) -> LedStrip (5v)
- LDR(1) -> ESP(GND)
- LDR(2) -> ESP(A0)
- ESP(A0) -> RESISTOR(100kOhm) -> ESP(3v) 

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

#### Hardware Drawings
##### letterplate
- I Have created a FreeCAD pyhton macro for generating a Letterplate, ..\hardware\lettering_plates\FreeCAD_drawings\freecad_macro
- The letterplate is paramatic so can be used for 11*11 of 13*13
- After the generation is done, export the part as .STEP (the DXF generator from freecad is not smooth)
- FreeCAD export DXF WORKS (0.18.4):
    - Edit -> Preference -> Inport-Export -> DXF
    - Max Spline Segment to 0,1 or 0,01 mm (NOTE use , instead of .)
    - Select all elements
    - File -> Export -> Autodesk DXF 2d
- Import the .STEP file into Fusion for creating the DXF file of gcode
- Gcode
    - Create A body by setting the hight
    - Switch to manufacture
    - start creating the Gcode
- DXF:
    - Create a sketch on the Top view
    - Menu -> Create -> Project / include -> include 3D Geometry
    - Select the Body from the letterplate
    - Save Sketch -> right klik the Sketch and save as DXF
##### ledschotten
- The LDR for the 11*11 clock is 214.444 mm from the bottem (This information is printed by FreeCAD)
- The LDR for the 13*13 clock is 477.778 mm from the bottem
- The LEdschotten is completly drawn in Fusion360, and is partly paramatic.

#### Assemble
- Create the led-panel by cutting out a square from hardboard
- Place the led-panel and the light divider on eatch other
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
- The MDF plate can also be replaced by "Foamed pvc"


# Note:
The NodeMcu Lolin V3 is needed because the Lolin has a 3v reference voltage for its adc that is used for the LDR all other NodeMC have 1v reference voltage

## Cost:
- Lolin NodeMCU: €3
- LDR: €0.1
- Resistor 10Kohm: €0.1
- LED strip: €10.0/€12.0
- Ikea RIBBA: €5.99/€9.99 
- MDF 12mm: €10.0
- letterplate
    - lasercutting: €24.0/€45.0
