import Draft
import FreeCAD as App
import FreeCAD

from SnijlabSetting import *

class light_divider:
    def __init__(self, part, depth, length, materialThickness, nrOfLeds, ledOffset):
        self.part = part
        self.depth = depth
        self.length = length
        self.materialThickness = materialThickness
        self.nrOfLeds = nrOfLeds
        self.ledOffset = ledOffset
    
    def create(self):
        base = self.create_base()
        light_divider = self.make_cutouts(base)
        return light_divider
    
    def create_base(self):
        BottemLeft = FreeCAD.Vector(0,0,0)
        Place = FreeCAD.Placement(BottemLeft, FreeCAD.Rotation(0,0,0))
        BaseRectangle = Draft.makeRectangle(self.depth, self.length, Place)
        print (self.depth)
        print (self.length)
        self.part.addObject(BaseRectangle)
        return BaseRectangle
    
    def make_cutouts(self, BaseRectangle):
        PRV_CUT = None
        firstCut=App.activeDocument().addObject("Part::Cut", "Cut")

        FirstLedCenter = (self.length / 2) - (((self.nrOfLeds - 1) / 2) * self.ledOffset)
        print (FirstLedCenter)
        StartScots =  FirstLedCenter - (self.ledOffset/2.0) - (self.materialThickness/2)
        print (StartScots)
        for index in range(self.nrOfLeds+1):
            cutBottemLeft = FreeCAD.Vector(0, StartScots + (LASER_CUTTING_WIDTH/2.0) + (index * self.ledOffset), 0)
            Place = FreeCAD.Placement(cutBottemLeft, FreeCAD.Rotation(0,0,0))
            cutRectangle = Draft.makeRectangle(self.depth/2.0, self.materialThickness-LASER_CUTTING_WIDTH, Place)
            self.part.addObject(cutRectangle)
            if PRV_CUT == None:
                firstCut.Base = BaseRectangle	
                firstCut.Tool = cutRectangle
                PRV_CUT = firstCut
            else:
                newCut=App.activeDocument().addObject("Part::Cut", "Cut")
                newCut.Base = PRV_CUT
                newCut.Tool = cutRectangle
                PRV_CUT = newCut
            App.ActiveDocument.recompute()
        complete_scot = PRV_CUT
        self.part.addObject(complete_scot)
        return complete_scot
    
if __name__ == "__main__":
    part = App.activeDocument().addObject('App::Part','Part')
    lightDividerLength = 13*(1000.0/30.0) + 3.0 - LASER_CUTTING_WIDTH
    light_divider = light_divider(part, 15, lightDividerLength, 3.0, 13, 1000.0/30.0)
    light_divider.create()
    