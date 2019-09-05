import Draft
import FreeCAD as App
import FreeCAD

try:
    from freecad_macro_libs import SnijlabSetting
except:
    import SnijlabSetting

INSERT_TEXT = True
INSERT_SQUARES = True
INSERT_HOLE_SQUARES = True

class front_plate:
    def __init__(self, part, text, font, fontScale, ledOffset, length, height, ldrDiameter, scots_thickness):
        self.part = part
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
    
    def create(self):
        self.make_hole_for_ldr()
        self.front_plate_iterator()
        self.make_outline()
    
    def make_outline(self):
        position = FreeCAD.Placement(FreeCAD.Vector(0, 0, 0), FreeCAD.Rotation(0,0,0))
        FrontPlateRectangle = Draft.makeRectangle(self.length, self.height, position)
        self.part.addObject(FrontPlateRectangle)
    
    def make_hole_for_ldr(self):
        TopOfFirstLine = self.posFirstLetterBottemLeft.y + self.ledOffset
        SpaceBeteenTopEnFirstLine = self.length - TopOfFirstLine
        positionLdr = FreeCAD.Vector(self.frontPlateCenter.x, self.height - (SpaceBeteenTopEnFirstLine/2.0), 0)
        PlaceLdr = FreeCAD.Placement(positionLdr, FreeCAD.Rotation(0,0,0))
        ldrCircle = Draft.makeCircle(self.ldrDiameter/2.0, PlaceLdr)
        self.part.addObject(ldrCircle)
    
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
                if INSERT_TEXT:
                    #The font is not alwayse 1:1 so scale it.
                    text = Draft.makeShapeString(letter, self.font, self.ledHoleWith * self.fontScale)
                    AdjCenter = FreeCAD.Vector(CenterSquare.x - self.letterCorrection['x'], CenterSquare.y - self.letterCorrection['y'])
                    text.Placement.Base = AdjCenter
                    self.part.addObject(text)
                if INSERT_SQUARES:
                    RecBottemLeft = FreeCAD.Vector(CenterSquare.x - (self.ledOffset/2), CenterSquare.y - (self.ledOffset/2))
                    Place = FreeCAD.Placement(RecBottemLeft,FreeCAD.Rotation(0,0,0))
                    Rectangle = Draft.makeRectangle(self.ledOffset, self.ledOffset, Place)
                    self.part.addObject(Rectangle)
                if INSERT_HOLE_SQUARES:
                    HoleBottemLeft = FreeCAD.Vector(CenterSquare.x - (self.ledHoleWith/2), CenterSquare.y - (self.ledHoleWith/2))
                    Place = FreeCAD.Placement(HoleBottemLeft,FreeCAD.Rotation(0,0,0))
                    HoleRectangle = Draft.makeRectangle(self.ledHoleWith, self.ledHoleWith, Place)
                    self.part.addObject(HoleRectangle)
                if INSERT_SQUARES and INSERT_HOLE_SQUARES:
                    myCut=App.activeDocument().addObject("Part::Cut","Cut")
                    myCut.Base = Rectangle
                    myCut.Tool = HoleRectangle
                    self.part.addObject(myCut)

    
if __name__ == "__main__":
    WORDCLOCK_TEXT = ["HETGISENUZELF",
    "BIJNAWDERTIEN",
    "DRIENEGENTIEN",
    "ZEVENTIENVIJF",
    "TWEENVIERTIEN",
    "ACHTTIENKWART",
    "ZESTIENTWAALF",
    "OVERVOOROHALF",
    "VIJFWACHTDRIE",
    "TWEENZESZEVEN",
    "TIENTWAALFELF",
    "BVIERNEGENRWA",
    "VUURAGEWEESTU"]

    part = App.activeDocument().addObject('App::Part','Part')
    text = WORDCLOCK_TEXT
    font = "C:/Users/rutger.huijgen/Dropbox/Hobby/wordclock_git/lettering_plates/fonts/Muar-Stencil.ttf"
    fontScale = 0.55
    ledOffset = 1000.0/30.0
    length = 500
    height = 500
    ldrDiameter = 5
    scots_thickness = 3
    front_plate = front_plate( part, text, font, fontScale, ledOffset, length, height, ldrDiameter, scots_thickness)
    front_plate.create()
    