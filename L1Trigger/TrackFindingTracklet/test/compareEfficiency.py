import ROOT as r
import os
import os.path

from optparse import OptionParser
parser = OptionParser()

# Use this for user specific label at the end of the filename
parser.add_option('--label', metavar='F', type='string', action='store',
                  default='',
                  dest='label',
                  help='')
(options, args) = parser.parse_args()
argv = []

userLabel = options.label

# Labels for input files
PUtypes = ["0","140","200"]
ptRangeTypes = {
0:"",
'L' : "Pt2to8",
'H' : "Pt8to100"
}
pdgIdTypes = { 0 : "",
               1 : "_injet",
               2 : "_injet_highpt",
               13 : "_pdgid13"#,
#               11 : "pdgid11",
#               211 : "pdgid211"
}

def SetPlotStyle():
  # from ATLAS plot style macro
  # use plain black on white colors
  r.gStyle.SetFrameBorderMode(0)
  r.gStyle.SetFrameFillColor(0)
  r.gStyle.SetCanvasBorderMode(0)
  r.gStyle.SetCanvasColor(0)
  r.gStyle.SetPadBorderMode(0)
  r.gStyle.SetPadColor(0)
  r.gStyle.SetStatColor(0)
  r.gStyle.SetHistLineColor(1)

  r.gStyle.SetPalette(1)

  # set the paper & margin sizes
  r.gStyle.SetPaperSize(20,26)
  r.gStyle.SetPadTopMargin(0.05)
  r.gStyle.SetPadRightMargin(0.05)
  r.gStyle.SetPadBottomMargin(0.16)
  r.gStyle.SetPadLeftMargin(0.16)

  # set title offsets (for axis label)
  r.gStyle.SetTitleXOffset(1.4)
  r.gStyle.SetTitleYOffset(1.4)

  # use large fonts
  r.gStyle.SetTextFont(42)
  r.gStyle.SetTextSize(0.05)
  r.gStyle.SetLabelFont(42,"x")
  r.gStyle.SetTitleFont(42,"x")
  r.gStyle.SetLabelFont(42,"y")
  r.gStyle.SetTitleFont(42,"y")
  r.gStyle.SetLabelFont(42,"z")
  r.gStyle.SetTitleFont(42,"z")
  r.gStyle.SetLabelSize(0.05,"x")
  r.gStyle.SetTitleSize(0.05,"x")
  r.gStyle.SetLabelSize(0.05,"y")
  r.gStyle.SetTitleSize(0.05,"y")
  r.gStyle.SetLabelSize(0.05,"z")
  r.gStyle.SetTitleSize(0.05,"z")

  # use bold lines and markers
  r.gStyle.SetMarkerStyle(20)
  r.gStyle.SetMarkerSize(1.2)
  r.gStyle.SetHistLineWidth(2)
  r.gStyle.SetLineStyleString(2,"[12 12]")

  # get rid of error bar caps
  r.gStyle.SetEndErrorSize(0.)

  # do not display any of the standard histogram decorations
  r.gStyle.SetOptTitle(0)
  r.gStyle.SetOptStat(0)
  r.gStyle.SetOptFit(0)

  # put tick marks on top and RHS of plots
  r.gStyle.SetPadTickX(1)
  r.gStyle.SetPadTickY(1)

def mySmallText(x, y, color, text):
  tsize=0.044;
  l = r.TLatex();
  l.SetTextSize(tsize); 
  l.SetNDC();
  l.SetTextColor(color);
  l.DrawLatex(x,y,text);

def getAllHistogramsFromFile( what, sample, ptRange, pdgid, rebin, normToOne=False ):

  # Make list of input trees
  inputFileNames = [];
  inputFileNameTemplate = ""
  if 'TTbar' in sample:
    inputFileNameTemplate = "output_{sample}_PU{PU}_{trunc}Truncation{pdg}{userLabel}.root"
  else :
    inputFileNameTemplate = "output_{sample}{ptRange}_PU{PU}_{trunc}Truncation{pdg}{userLabel}.root"

  inputFileNames.append( inputFileNameTemplate.format(sample = sample, PU = PUtypes[0], ptRange=ptRangeTypes[ptRange], pdg=pdgIdTypes[pdgid], trunc = 'With', userLabel=userLabel ) )
  inputFileNames.append( inputFileNameTemplate.format(sample = sample, PU = PUtypes[1], ptRange=ptRangeTypes[ptRange], pdg=pdgIdTypes[pdgid], trunc = 'With', userLabel=userLabel ) )
  inputFileNames.append( inputFileNameTemplate.format(sample = sample, PU = PUtypes[2], ptRange=ptRangeTypes[ptRange], pdg=pdgIdTypes[pdgid], trunc = 'With', userLabel=userLabel ) )
  inputFileNames.append( inputFileNameTemplate.format(sample = sample, PU = PUtypes[0], ptRange=ptRangeTypes[ptRange], pdg=pdgIdTypes[pdgid], trunc = 'Without', userLabel=userLabel ) )
  inputFileNames.append( inputFileNameTemplate.format(sample = sample, PU = PUtypes[1], ptRange=ptRangeTypes[ptRange], pdg=pdgIdTypes[pdgid], trunc = 'Without', userLabel=userLabel ) )
  inputFileNames.append( inputFileNameTemplate.format(sample = sample, PU = PUtypes[2], ptRange=ptRangeTypes[ptRange], pdg=pdgIdTypes[pdgid], trunc = 'Without', userLabel=userLabel ) )

  # Get trees from files
  inputFiles=[];
  for i in range(0,len(inputFileNames)):
    if os.path.isfile( inputFileNames[i] ):
      inputFiles.append(r.TFile(inputFileNames[i]))
    else:
      inputFiles.append(None)

  histograms = {
    'PU0_wt' : getHistogramFromFile(inputFiles[0], what),
    'PU140_wt' : getHistogramFromFile(inputFiles[1], what),
    'PU200_wt' : getHistogramFromFile(inputFiles[2], what),
    'PU0_wot' : getHistogramFromFile(inputFiles[3], what),
    'PU140_wot' : getHistogramFromFile(inputFiles[4], what),
    'PU200_wot' : getHistogramFromFile(inputFiles[5], what),
  }
      

  if rebin > 1:
    for n, h in histograms.iteritems():
      if h != None:
        h.Rebin(rebin)

  if normToOne:
    for n, h in histograms.iteritems():
      if h != None:
        h.Scale( 1 / h.Integral() )

  return histograms

def getHistogramFromFile(file, histogramName):
  if file != None and file.GetListOfKeys().Contains(histogramName):
    h = file.Get(histogramName)
    h.SetDirectory(0)
    return h
  else: return None

def setMarkerAndLineAttributes(h, colour, style, lineStyle=1 ):
  h.SetLineColor( colour )
  h.SetMarkerColor( colour )
  h.SetMarkerStyle( style )
  h.SetLineStyle( lineStyle )

def drawHistogramWithOption(h,drawOption):
  h.Draw(drawOption)
  if not 'same' in drawOption:
    drawOption +=', same'
  return drawOption

def setupLegend(sample, histograms, PULabels, legPosition=""):
  legx = 0.6;
  legy = 0.2;
  if legPosition == 'topright':
    legx = 0.6
    legy = 0.65
  r.gPad.cd()
  l = r.TLegend(legx,legy,legx+0.3,legy+0.18)
  l.SetFillColor(0)
  l.SetFillStyle(0)
  l.SetLineColor(0)
  l.SetTextSize(0.04)
  l.AddEntry(histograms['PU0_wt'], "With truncation", "p")
  l.AddEntry(histograms['PU0_wot'], "Without truncation", "l")
  l.AddEntry(None,"","")

  if histograms['PU0_wt'] != None or histograms['PU0_wot'] != None :
    h = histograms['PU0_wt']
    if h == None: h = histograms['PU0_wot']
    l.AddEntry(h,PULabels[0],"lp")
  if histograms['PU140_wt'] != None or histograms['PU140_wot'] != None :
    h = histograms['PU140_wt']
    if h == None: h = histograms['PU140_wot']
    l.AddEntry(h,PULabels[1],"lp")
  if histograms['PU200_wt'] != None or histograms['PU200_wot'] != None :
    h = histograms['PU200_wt']
    if h == None: h = histograms['PU200_wot']
    l.AddEntry(h,PULabels[2],"lp")
  l.SetTextFont(42)

  return l

# ----------------------------------------------------------------------------------------------------------------
# Main script
def compareEfficiency(what, sample, ptRange=0, pdgid=0,rebin=0, normToOne=False, legPosition=""):
  
  SetPlotStyle()
  # Labels for the plots
  PULabels = ["<PU>=0", "<PU>=140", "<PU>=200"]
  ptRangeLabels = ["2 < P_{T} < 8 GeV","P_{T} > 8 GeV"]

  # Get histograms
  histograms = getAllHistogramsFromFile( what, sample, ptRange, pdgid, rebin, normToOne )

  canvas = r.TCanvas()

  # Draw histogram with truncation, as points
  drawOption='p'
  if histograms['PU0_wt'] != None:
    setMarkerAndLineAttributes( histograms['PU0_wt'], 1, 20, 1)
    drawOption = drawHistogramWithOption( histograms['PU0_wt'], drawOption )
  if histograms['PU140_wt'] != None :
    setMarkerAndLineAttributes( histograms['PU140_wt'], 2, 22, 1)
    drawOption = drawHistogramWithOption( histograms['PU140_wt'], drawOption )
  if histograms['PU200_wt'] != None:
    setMarkerAndLineAttributes( histograms['PU200_wt'], 9, 21, 1)
    drawOption = drawHistogramWithOption (histograms['PU200_wt'], drawOption)

  if 'same' in drawOption:
    drawOption = 'hist,l,same'
  else:
    drawOption = 'hist,l'

  # Draw histograms without truncation, as lines
  if histograms['PU0_wot'] != None:
    setMarkerAndLineAttributes( histograms['PU0_wot'], 1, 20, 2)
    drawOption = drawHistogramWithOption (histograms['PU0_wot'], drawOption )
  if histograms['PU140_wot'] != None:
    setMarkerAndLineAttributes( histograms['PU140_wot'], 2, 4, 2)
    drawOption = drawHistogramWithOption (histograms['PU140_wot'], drawOption )
  if histograms['PU200_wot'] != None:
    setMarkerAndLineAttributes( histograms['PU200_wot'], 9, 33, 2)
    drawOption = drawHistogramWithOption (histograms['PU200_wot'], drawOption )
  if 'eff' in what:
    r.gPad.SetGridy();

  # Make the legend
  l = setupLegend(sample,histograms,PULabels, legPosition)
  l.Draw()

  # Save canvas
  outputDir = 'OverlayPlots'+userLabel
  if not os.path.isdir(outputDir):
    os.mkdir(outputDir)
  outputFileName = "{outputDir}/{sample}_{what}.pdf".format( outputDir=outputDir, sample = sample, what=what )

  if 'TTbar' in sample:
    if pdgid == 13:
      outputFileName = "{outputDir}/{sample}_muons_{what}.pdf".format( outputDir=outputDir, sample = sample, what=what )
    elif pdgid == 1:
      outputFileName = "{outputDir}/{sample}_injet_{what}.pdf".format( outputDir=outputDir, sample = sample, what=what )
    elif pdgid == 2:
      outputFileName = "{outputDir}/{sample}_injet_highpt_{what}.pdf".format( outputDir=outputDir, sample = sample, what=what )

  if 'ntrk_pt' in what and sample != 'TTbar':
    outputFileName = "{outputDir}/{sample}_{ptRange}_{what}.pdf".format( outputDir=outputDir, ptRange=ptRange, sample = sample, what=what )

  canvas.Print(outputFileName);

if __name__ == '__main__':
  r.gROOT.SetBatch()

  for sample in ['TTbar']:
    for pdg in [13]:
      compareEfficiency("eff_pt_L",sample,0,pdg)
      compareEfficiency("eff_pt_H",sample,0,pdg)
      compareEfficiency("eff_eta_L",sample,0,pdg)
      compareEfficiency("eff_eta_H",sample,0,pdg)
    for pdg in [1,2]:
      compareEfficiency("eff_pt",sample,0,pdg)
      compareEfficiency("eff_eta",sample,0,pdg)

    #compareEfficiency("eff_d0",sample,0,0,rebin=0, normToOne=False, legPosition="topright")
    compareEfficiency("eff_d0",sample,0,0)
    
    compareEfficiency("ntrk_pt2",sample,0,0,rebin=4, normToOne=True, legPosition="topright")
    compareEfficiency("ntrk_pt3",sample,0,0,rebin=4, normToOne=True, legPosition="topright")
    compareEfficiency("ntrk_pt10",sample,0,0,rebin=4, normToOne=True, legPosition="topright")



