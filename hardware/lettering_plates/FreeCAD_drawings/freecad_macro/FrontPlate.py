import Draft
import FreeCAD as App
import FreeCAD

# Get the path the macro is in.
p=FreeCAD.ParamGet("User parameter:BaseApp/Preferences/Macro")
macro_path = p.GetString("MacroPath")
sys.path.append("macro_path")
from SnijlabSetting import *
import LetterPlateText

FONT_PATH = macro_path +"/../../fonts/"

CONFIGS = {
    'l_taurus': {   # Large plate, Taurus font
        'font':       FONT_PATH + "Taurus-Mono-Stencil-Bold-ij.ttf",
        'font_scale': 0.30   ,
		'size':       [500, 500],
        'led_offset': 1000/30
    },
    's_taurus': {   # Small plate, Taurus font
        'font':       FONT_PATH + "Taurus-Mono-Stencil-Bold-ij.ttf",
        'font_scale': 0.34,
		'size':       [230, 230],
        'led_offset': 1000/60
    },
    'l_lib': {   # Large plate, Liberation font
        'font':       FONT_PATH + "LiberationMono-Regular-stencil-50cm.ttf",
        'font_scale': 0.80   ,
		'size':       [500, 500],
        'led_offset': 1000/30
    },
    's_lib': {   # Small plate, Liberation font
        'font':       FONT_PATH + "LiberationMono-Regular-stencil-50cm.ttf",
        'font_scale': 0.84   ,
		'size':       [230, 230],
        'led_offset': 1000/60
    },
    's_muar': {   # Small plate, Muar font
        'font':       FONT_PATH + "Muar-Stencil.ttf",
        'font_scale': 0.50   ,
		'size':       [230, 230],
        'led_offset': 1000/60
    },
}

# WORDCLOCK_TEXT = ["HETGISENUZELF"]
WORDCLOCK_TEXT = LetterPlateText.WORDCLOCK_TEXT_11_11
USE_LAYERS=False    # Using layers makes it slow somehow!?
PLATE_TYPE='l_lib'


class front_plate:
    def __init__(self, part, text, font, fontScale, ledOffset, length, height, ldrDiameter, scots_thickness):
        self.drawText = True
        self.drawSquares = False
        self.drawSquaresHoles = False
        self.drawCircles = True
        self.drawLdr = True
        self.drawOutline = True
        self.part = part
        if USE_LAYERS:
            self.pocket_layer = Draft.makeVisGroup(name="pockets")
            self.part.addObject(self.pocket_layer)
            self.text_layer = Draft.makeVisGroup(name="text")
            self.part.addObject(self.text_layer)
            self.outline_layer = Draft.makeVisGroup(name="outline")
            self.part.addObject(self.outline_layer)
        else:
            self.pocket_layer = self.part
            self.text_layer = self.part
            self.outline_layer = self.part

        self.text = text
        self.font = font
        self.fontScale = fontScale
        self.ledOffset = ledOffset
        self.length = length
        self.height = height
        self.ldrDiameter = ldrDiameter
        
        self.ledHoleWith = ledOffset - scots_thickness
        self.nrOfLeds = len(self.text[0])
        #Caculate the needed correction for a nice Symmetric letters,
        self.letterCorrection = self.caculat_center_off_mass('H')
        self.frontPlateCenter = FreeCAD.Vector(length/2.0, height/2.0, 0)
        self.posFirstLetterBottemLeft = FreeCAD.Vector(self.frontPlateCenter.x - ((self.nrOfLeds /2.0) * self.ledOffset), self.frontPlateCenter.y + (((self.nrOfLeds/2.0) -1) * self.ledOffset), 0)
        
        print (self.nrOfLeds)
        print (self.letterCorrection)
        print (self.frontPlateCenter)
        print (self.posFirstLetterBottemLeft)

    def change_draw_settings(self, drawText, drawSquares, drawSquaresHoles, drawLdr, drawOutline):
        self.drawText = drawText
        self.drawSquares = drawSquares
        self.drawSquaresHoles = drawSquaresHoles
        self.drawLdr = drawLdr
        self.drawOutline = drawOutline
    
    def create(self):
        if self.drawLdr:
            self.make_hole_for_ldr()
        self.front_plate_iterator()
        if self.drawOutline:
            self.make_outline()
    
    def make_outline(self):
        position = FreeCAD.Placement(FreeCAD.Vector(0, 0, 0), FreeCAD.Rotation(0,0,0))
        FrontPlateRectangle = Draft.makeRectangle(self.length, self.height, position)
        self.outline_layer.addObject(FrontPlateRectangle)
    
    def make_hole_for_ldr(self):
        TopOfFirstLine = self.posFirstLetterBottemLeft.y + self.ledOffset
        SpaceBeteenTopEnFirstLine = self.length - TopOfFirstLine
        positionLdr = FreeCAD.Vector(self.frontPlateCenter.x, self.height - ((SpaceBeteenTopEnFirstLine/3.0)*2.0), 0)
        print("LDR offcet: " + str(((SpaceBeteenTopEnFirstLine/3.0)*2.0)))
        PlaceLdr = FreeCAD.Placement(positionLdr, FreeCAD.Rotation(0,0,0))
        ldrCircle = Draft.makeCircle(self.ldrDiameter/2.0, PlaceLdr)
        self.text_layer.addObject(ldrCircle)
    
    def caculat_center_off_mass(self, letter):
        TestLetter = Draft.makeShapeString('H', self.font, self.ledHoleWith * self.fontScale)
        LetterCenter = App.activeDocument().addObject('PartDesign::Point','DatumPoint')
        LetterCenter.Support = TestLetter
        LetterCenter.MapMode = 'CenterOfMass'
        center = {}
        center['x']= LetterCenter.Placement.Base.x
        center['y'] = LetterCenter.Placement.Base.y
        
        TestLetter.Document.removeObject(TestLetter.Name)
        LetterCenter.Document.removeObject(LetterCenter.Name)
        return center

    def front_plate_iterator(self):
        for line_nr, line in enumerate(self.text):
            for letter_nr, letter in enumerate(line):
                PosBottemLeft = FreeCAD.Vector(self.posFirstLetterBottemLeft.x + (self.ledOffset * letter_nr), self.posFirstLetterBottemLeft.y - (self.ledOffset * line_nr), 0)
                CenterSquare = FreeCAD.Vector(PosBottemLeft.x + (self.ledOffset/2), PosBottemLeft.y + (self.ledOffset/2))
                if self.drawText:
                    #The font is not alwayse 1:1 so scale it.
                    text = Draft.makeShapeString(letter, self.font, self.ledHoleWith * self.fontScale)
                    AdjCenter = FreeCAD.Vector(CenterSquare.x - self.letterCorrection['x'], CenterSquare.y - self.letterCorrection['y'])
                    text.Placement.Base = AdjCenter
                    self.text_layer.addObject(text)
                if self.drawSquares:
                    RecBottemLeft = FreeCAD.Vector(CenterSquare.x - (self.ledOffset/2), CenterSquare.y - (self.ledOffset/2))
                    Place = FreeCAD.Placement(RecBottemLeft,FreeCAD.Rotation(0,0,0))
                    Rectangle = Draft.makeRectangle(self.ledOffset, self.ledOffset, Place)
                    self.part.addObject(Rectangle)
                if self.drawSquaresHoles:
                    HoleBottemLeft = FreeCAD.Vector(CenterSquare.x - (self.ledHoleWith/2), CenterSquare.y - (self.ledHoleWith/2))
                    Place = FreeCAD.Placement(HoleBottemLeft,FreeCAD.Rotation(0,0,0))
                    HoleRectangle = Draft.makeRectangle(self.ledHoleWith, self.ledHoleWith, Place)
                    self.part.addObject(HoleRectangle)
                if self.drawSquares and self.drawSquaresHoles:
                    myCut=App.activeDocument().addObject("Part::Cut","Cut")
                    myCut.Base = Rectangle
                    myCut.Tool = HoleRectangle
                    self.part.addObject(myCut)
                if self.drawCircles:
                    Place = FreeCAD.Placement(CenterSquare,FreeCAD.Rotation(0,0,0))
                    self.pocket_layer.addObject(Draft.makeCircle(self.ledOffset*0.9/2, placement=Place))

    
if __name__ == "__main__":
    part = App.activeDocument().addObject('App::Part','Part')
    text = WORDCLOCK_TEXT
    config = CONFIGS[PLATE_TYPE]
    # font = "C:/proj/esp8266_wordclock/hardware/lettering_plates/fonts/LiberationMono-Regular-stencil-23cm.ttf"
    # fontScale = 0.80
    font = config['font']
    fontScale = config['font_scale']
    ledOffset = config['led_offset']
    length = config['size'][0]
    height = config['size'][1]
    ldrDiameter = 5
    scots_thickness = 3
    front_plate = front_plate( part, text, font, fontScale, ledOffset, length, height, ldrDiameter, scots_thickness)
    front_plate.create()
    