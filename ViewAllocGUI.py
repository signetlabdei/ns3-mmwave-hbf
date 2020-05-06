#!/usr/bin/python

import sys
import gi
import cairo
import collections

gi.require_version("Gtk", "3.0")
from gi.repository import Gtk

SlotInfoType = collections.namedtuple("SlotInfoType", ('type','size','UE'))

class MyDrawingAreaFrame(Gtk.Frame):
    def __init__(self, css=None, border_width=0):
        super(MyDrawingAreaFrame,self).__init__()
        self.set_border_width(border_width)
        self.set_size_request(800, 200)
        self.vexpand = True
        self.hexpand = True
        self.surface = None

        self.area = Gtk.DrawingArea()
        self.add(self.area)

        self.area.connect("draw", self.on_draw)
        self.area.connect('configure-event', self.on_configure)
        self.frameInfo=[[]]

    def init_surface(self, area):
        # Destroy previous buffer
        if self.surface is not None:
            self.surface.finish()
            self.surface = None

        # Create a new buffer
        self.surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, area.get_allocated_width(), area.get_allocated_height())

    def redraw(self):
        self.init_surface(self.area)
        context = cairo.Context(self.surface)
        context.scale(self.surface.get_width(), self.surface.get_height())
        self.do_drawing(context)
        self.surface.flush()

    def on_configure(self, area, event, data=None):
        self.redraw()
        return False

    def on_draw(self, area, context):
        if self.surface is not None:
            context.set_source_surface(self.surface, 0.0, 0.0)
            context.paint()
        else:
            print('Invalid surface')
        return False

    def draw_rect_frame(self, ctx, frameInfo):
        if frameInfo == None:
            frame=[[SlotInfoType('blank',1,0)]]
        else:
            frame=frameInfo

        maxNSym= max([sum([s.size for s in x]) for x in frame])
        nLayers= len(frame)
        yOffset=0
        for layer in frame:
            xOffset=0
            for slot in layer:
                ctx.set_line_width(.003)
                ctx.set_source_rgb(.1,.1,.1)
                ctx.rectangle(xOffset, yOffset, slot.size*1.0/maxNSym, 1.0/nLayers)
                ctx.stroke()
                slotText = ""
                if slot.type=="PADDING":
                    ctx.set_source_rgb(.7,.7,.7)
                elif slot.type=="DLCTRL":
                    ctx.set_source_rgb(.7,.2,.2)
                    slotText="DC"
                elif slot.type=="ULCTRL":
                    ctx.set_source_rgb(.7,.6,.2)
                    slotText="UC"
                elif slot.type=="DLDATA":
                    ctx.set_source_rgb(.2,.2,.7)
                    slotText="D%d"%(slot.UE)
                elif slot.type=="ULDATA":
                    ctx.set_source_rgb(.2,.7,.2)
                    slotText="U%d"%(slot.UE)
                else:
                    ctx.set_source_rgb(.9,.9,.9)
                ctx.rectangle(xOffset, yOffset, slot.size*1.0/maxNSym,1.0/nLayers)
                ctx.fill()
                if len(slotText)>0:
                    ctx.set_source_rgb(0,0,0)
                    ctx.select_font_face("Georgia",cairo.FONT_SLANT_NORMAL, cairo.FONT_WEIGHT_BOLD)
                    ctx.set_font_size(.7/maxNSym)
                    x_bearing, y_bearing, width, height = ctx.text_extents(slotText)[:4]
                    ctx.move_to(xOffset+slot.size*1.0/maxNSym/2-width/2-x_bearing,yOffset+1.0/nLayers/2-height/2 -y_bearing)
                    ctx.show_text(slotText)
                xOffset = xOffset + slot.size*1.0/maxNSym
            yOffset = yOffset + 1.0/nLayers


    def do_drawing(self, ctx):
        self.draw_rect_frame(ctx,self.frameInfo)


class MyWindow(Gtk.Window):
    def __init__(self):
        Gtk.Window.__init__(self, title="Ns3 mmWave HBF Frame Visualization Tool")
        self.set_default_size(800, 100)
        self.connect("destroy", Gtk.main_quit)

        self.frameFocused=0
        self.maxFrames=10
        self.frameInfoCache={}
        self.workingfilename="TCP-SBF-PAD4-HARQ.log"
        self.maxLayer=1

        grid = Gtk.Grid()
        self.add(grid)

        self.drawingArea = MyDrawingAreaFrame()
        grid.attach(self.drawingArea, 0, 0, 1, 1)

        self.actionbar = Gtk.ActionBar()
        self.actionbar.set_hexpand(True)
        grid.attach(self.actionbar, 0, 1, 1, 1)


        self.fChooser = Gtk.FileChooserButton()
        self.fChooser.connect("file-set", self.file_selected)
        self.actionbar.pack_start(self.fChooser)
        self.bButton = Gtk.Button(label="<=")
        self.bButton.connect("clicked", self.bk_button_clicked)
        self.actionbar.pack_start(self.bButton)
        self.subframeLabel = Gtk.Label("Subframe: ")
        self.actionbar.pack_start(self.subframeLabel)
        self.subframeNuminput = Gtk.SpinButton(value=0,wrap=True,numeric=True)
        self.subframeNuminput.set_range(0,self.maxFrames-1)
        self.subframeNuminput.connect("value-changed", self.sf_num_input)
        self.actionbar.pack_start(self.subframeNuminput)
        self.maxframeLabel = Gtk.Label("/%d"%(self.maxFrames))
        self.actionbar.pack_start(self.maxframeLabel)
        self.fButton = Gtk.Button(label="=>")
        self.fButton.connect("clicked", self.fw_button_clicked)
        self.actionbar.pack_start(self.fButton)

    def bk_button_clicked(self, widget):
        self.frameFocused= (self.frameFocused - 1) % self.maxFrames
        self.focus_frame()
    def fw_button_clicked(self, widget):
        self.frameFocused= (self.frameFocused + 1) % self.maxFrames
        self.focus_frame()
    def sf_num_input(self, widget):
        self.frameFocused= self.subframeNuminput.get_value()
        self.focus_frame()
    def file_selected(self, widget):
        self.workingfilename = self.fChooser.get_filename()
        self.frameInfoCache.clear()
        self.maxFrames=0
        self.maxLayer=0
        self.focus_frame()
    def focus_frame(self):
        self.subframeNuminput.set_value(self.frameFocused)
        self.subframeNuminput.set_range(0,self.maxFrames-1)
        self.maxframeLabel.set_text("/%d"%(self.maxFrames))
        self.drawingArea.frameInfo=self.getFrameInfo(self.frameFocused)
        self.drawingArea.redraw()
        self.drawingArea.queue_draw()
    def getFrameInfo(self, frameNum ):
        if self.frameInfoCache.has_key(frameNum):
            frameInfo = self.frameInfoCache[frameNum]
        else:
            frameInfo = self.obtainFrameInfoFromFile(frameNum)
            self.frameInfoCache[frameNum]=frameInfo
        return(frameInfo)
    def obtainFrameInfoFromFile(self, frameNum ):
        file=open(self.workingfilename)
        frameInfo=[[]]
        if self.workingfilename.find('.log')>=0:
            for ln in file:
                if ln.find("Fr ")==0:
                    fr = int(ln[ ln.find("Fr ")+3:ln.find(' ',ln.find('Fr ')+3) ])
                    sf = int(ln[ ln.find("Sf ")+3:ln.find(' ',ln.find('Sf ')+3) ])
                    self.maxFrames=max(self.maxFrames,fr*10+sf)
                    if ln.find("layerIdx ")>=0:
                        layerStr = ln[ ln.find("layerIdx ")+9: ]
                        self.maxLayer = max(int(layerStr[ layerStr.find('of ')+3:]),self.maxLayer)
                    else:
                        self.maxLayer = 1
                        layerStr= "0 of 1"
                    if frameNum == ( fr*10+sf ):
                        rangeStr = ln[ ln.find("sym range ")+10:ln.find('of',ln.find("sym range ")+10) ]
                        startSymb = int(rangeStr[ 0:rangeStr.find(' ') ])
                        endSymb = int(rangeStr[ rangeStr.find('to ')+3:-1 ])
                        layer = int(layerStr[ 0:ln.find(' ') ])
                        while len(frameInfo)<self.maxLayer:
                            frameInfo.append([])
                        if len(frameInfo[layer])==0:
                            nextSymb=0
                        else:
                            nextSymb=sum([s.size for s in frameInfo[layer]])
                        if nextSymb<startSymb:
                            slot=SlotInfoType('PADDING',startSymb-nextSymb,0)
                            frameInfo[layer].append(slot)
                        type="unknown"
                        if ln.find("DL CTRL")>=0:
                            type="DLCTRL"
                            ue=0
                        elif ln.find("UL CTRL")>=0:
                            type="ULCTRL"
                            ue=0
                        elif ln.find("to UE ")>=0:
                            ue=int(ln[ ln.find("to UE ")+6:ln.find(' ',ln.find("to UE ")+6) ])
                            if ln.find("DL")>=0:
                                type="DLDATA"
                            elif ln.find("UL")>=0:
                                type="ULDATA"
                        slot=SlotInfoType(type,endSymb-startSymb+1,ue)
                        frameInfo[layer].append(slot)
        else:
            bFirstLine=True
            frameInfo[0].append( SlotInfoType("DLCTRL",1,0) )
            for ln in file:
                if bFirstLine:
                    bFirstLine=False
                    continue
                vals=ln.split('\t')
                bDL=(vals[0].find('DL')>=0)
                fr=int(vals[2])
                sf=int(vals[3])
                self.maxFrames=max(self.maxFrames,fr*10+sf)
                if frameNum == ( fr*10+sf ):
                    startSymb=int(vals[4])
                    numSymb=int(vals[5])
                    rnti=int(vals[7])
                    size=int(vals[9])
                    mcsact=int(vals[10])
                    sinrdB=float(vals[12])
                    corrupted=int(vals[13])
                    tbler=float(vals[14])
                    layer = int(vals[15] if bDL else vals[16][:-1])
                    self.maxLayer = max(layer+1,self.maxLayer)
                    while len(frameInfo)<self.maxLayer:
                        frameInfo.append([])
                    if len(frameInfo[layer])==0:
                        nextSymb=0
                    else:
                        nextSymb=sum([s.size for s in frameInfo[layer]])
                    if nextSymb<startSymb:
                        slot=SlotInfoType('PADDING',startSymb-nextSymb,0)
                        frameInfo[layer].append(slot)
                    type="DLDATA" if bDL else "ULDATA"
                    slot=SlotInfoType(type,numSymb,rnti)
                    frameInfo[layer].append(slot)
            if len(frameInfo[0])==0:
                nextSymb=0
            else:
                nextSymb=sum([s.size for s in frameInfo[0]])
            if nextSymb<13:
                frameInfo[0].append( SlotInfoType('PADDING',13-nextSymb,0) )
            frameInfo[0].append( SlotInfoType("ULCTRL",1,0) )
        if len(frameInfo)==0:
            frameInfo=None
        file.close()
        return(frameInfo)

win = MyWindow()
win.show_all()
Gtk.main()
