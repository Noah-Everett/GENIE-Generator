//____________________________________________________________________________
/*!

\program gxscomp

\brief   A simple utility that plots the pre-calculated cross-sections used as
         input for event generation. Can also compare against a reference set of
         such pre-computed cross-sections.

         Syntax:
           gxscomp
                -f xsec_file[,label] 
               [-r reference_xsec_file[,label]] 
               [-o output]

         Options:
           [] Denotes an optional argument.
           -f Specifies a ROOT file with GENIE cross section graphs.
	   -r Specifies a reference file with GENIE cross section graphs.
           -o Specifies the output filename [default: xsec.ps]

         Notes:
           The input ROOT files are the ones generated by GENIE's gspl2root
           utility. See the GENIE User Manual for more details.
           	      
         Example:
           To compare cross sections in xsec-v2_4.root and xsec-v2_2.root
           type:
           shell$ gxscomp -f xsec-v2_4.root -r xsec-v2_2.root 
		      
\author  Costas Andreopoulos <c.andreopoulos \at cern.ch>
 University of Liverpool

\created June 06, 2008 

\cpright Copyright (c) 2003-2024, The GENIE Collaboration
         For the full text of the license visit http://copyright.genie-mc.org
         
*/
//____________________________________________________________________________

#include <cassert>
#include <sstream>
#include <string>
#include <vector>

#include <TSystem.h>
#include <TFile.h>
#include <TDirectory.h>
#include <TGraph.h>
#include <TPostScript.h>
#include <TH1D.h>
#include <TMath.h>
#include <TCanvas.h>
#include <TPavesText.h>
#include <TText.h>
#include <TStyle.h>
#include <TLegend.h>
#include <TObjString.h>

#include "Framework/Conventions/GBuild.h"
#include "Framework/Messenger/Messenger.h"
#include "Framework/Numerical/RandomGen.h"
#include "Framework/ParticleData/PDGUtils.h"
#include "Framework/ParticleData/PDGCodes.h"
#include "Framework/Utils/CmdLnArgParser.h"
#include "Framework/Utils/StringUtils.h"
#include "Framework/Utils/Style.h"

using std::ostringstream;
using std::string;
using std::vector;

using namespace genie;

// function prototypes
void    Init               (void);
void    End                (void);
void    OpenDir            (void);
void    DirNameToProbe     (void);
void    DirNameToTarget    (void);
void    MakePlots          (void);
void    MakePlotsCurrDir   (void);
TH1F *  DrawFrame          (TGraph * gr0, TGraph * gr1, TPad * p, const char * xt, const char * yt, double yminsc, double ymaxsc);
TH1F *  DrawFrame          (double xmin, double xmax, double ymin, double ymax, TPad * p, const char * xt, const char * yt);
void    Draw               (const char * plot, const char * title);
void    Draw               (TGraph* gr, const char * opt);
TGraph* TrimGraph          (TGraph * gr, int max_np_per_decade);
TGraph* DrawRatio          (TGraph * gr0, TGraph * gr1);
void    GetCommandLineArgs (int argc, char ** argv);
void    PrintSyntax        (void);
bool    CheckRootFilename  (string filename);
string  OutputFileName     (string input_file_name);

// command-line arguments
string gOptXSecFilename_curr = "";  // (-f) input ROOT cross section file
string gOptXSecFilename_ref0 = "";  // (-r) input ROOT cross section file (ref.)
string gOptOutputFilename    = "";  // (-o) output PS file
bool   gOptHaveRef;

// globals
TFile *       gXSecFile_curr        = 0;
TFile *       gXSecFile_ref0        = 0;
TDirectory *  gDirCurr              = 0;
TDirectory *  gDirRef0              = 0;
string        gLabelCurr            = "";
string        gLabelRef0            = "";
string        gDirName              = "";
TPostScript * gPS                   = 0;
TCanvas *     gC                    = 0;
TPad *        gPadTitle             = 0;
TPad *        gPadXSecs             = 0;
TPad *        gPadRatio             = 0;
TLegend *     gLS                   = 0;
string        gCurrProbeLbl         = "";
int           gCurrProbePdg         = 0;
bool          gCurrProbeIsNu        = false;
bool          gCurrProbeIsNuBar     = false;
string        gCurrTargetLbl        = "";
bool          gCurrTargetHasP       = false;
bool          gCurrTargetHasN       = false;
bool          gCurrTargetIsFreeNuc  = false;

//_________________________________________________________________________________
int main(int argc, char ** argv)
{
  GetCommandLineArgs (argc,argv);
  utils::style::SetDefaultStyle();
  Init();
  MakePlots();
  End();
  
  LOG("gxscomp", pINFO)  << "Done!";

  return 0;
}
//_________________________________________________________________________________
void Init(void)
{
  gC = new TCanvas("c","",20,20,500,650);
  gC->SetBorderMode(0);
  gC->SetFillColor(0);

  // create output file
  gPS = new TPostScript(gOptOutputFilename.c_str(), 111);

  // front page
  gPS->NewPage();
  gC->Range(0,0,100,100);
  TPavesText hdr(10,40,90,70,3,"tr");
  hdr.AddText("GENIE cross sections");
  hdr.AddText(" ");
  hdr.AddText(" ");
  hdr.AddText("Plotting data from: ");
  hdr.AddText(gOptXSecFilename_curr.c_str());
  if(gOptHaveRef) {
     hdr.AddText(" ");
     hdr.AddText("Comparing with reference data (red circles) from: ");
     hdr.AddText(gOptXSecFilename_ref0.c_str());
  } else {
     hdr.AddText(" ");
  }
  hdr.AddText(" ");
  hdr.AddText(" ");
  hdr.Draw();
  gC->Update();

  gPS->NewPage();

  //
  if(gOptHaveRef) {
    gPadTitle = new TPad("gPadTitle","",0.05,0.90,0.95,0.97);
    gPadXSecs = new TPad("gPadXSecs","",0.05,0.35,0.95,0.88);
    gPadRatio = new TPad("gPadRatio","",0.05,0.03,0.95,0.32);
  } else {
    gPadTitle = new TPad("gPadTitle","",0.05,0.90,0.95,0.97);
    gPadXSecs = new TPad("gPadXSecs","",0.05,0.05,0.95,0.88);
    gPadRatio = new TPad("gPadRatio","",0.05,0.03,0.95,0.04);
  }

  gPadTitle->Range(0,0,100,100);

  gPadTitle->SetBorderMode(0);
  gPadTitle->SetFillColor(0);
  gPadXSecs->SetBorderMode(0);
  gPadXSecs->SetFillColor(0);
  gPadRatio->SetBorderMode(0);
  gPadRatio->SetFillColor(0);

  gPadXSecs->SetGridx();
  gPadXSecs->SetGridy();
  gPadXSecs->SetLogx();
  gPadXSecs->SetLogy();
  gPadRatio->SetGridx();
  gPadRatio->SetGridy();
  gPadRatio->SetLogx();

  gPadTitle->Draw();
  gPadXSecs->Draw();
  gPadRatio->Draw();

  gPadXSecs->cd();

  gLS = new TLegend(0.80,0.25,0.90,0.45);
  gLS -> SetFillColor(0);
//gLS -> SetFillStyle(0);
  gLS -> SetBorderSize(0);
}
//_________________________________________________________________________________
void End(void)
{
  gPS->Close();

  delete gC;
  delete gPS;
  delete gLS;
}
//_________________________________________________________________________________
void OpenDir(void)
{
  gDirCurr  = (TDirectory *) gXSecFile_curr->Get(gDirName.c_str());
  gDirRef0  = 0;
  if(gXSecFile_ref0) {
    gDirRef0 = (TDirectory *) gXSecFile_ref0->Get(gDirName.c_str());
  }

  if(!gDirRef0) {
    LOG("gxscomp", pINFO)  << "No reference plots will be shown.";
  }
}
//_________________________________________________________________________________
void DirNameToProbe(void)
{
// Figure out the probe type from the input directory name
//

  string dirname = gDirName;

  int    pdg     = 0;
  string label   = "";

  if( dirname.find ("nu_e_bar") != string::npos ) 
  { 
    label = "#bar{#nu_{e}}";    
    pdg   = kPdgAntiNuE;   
  }
  else 
  if ( dirname.find ("nu_e") != string::npos ) 
  { 
    label = "#nu_{e}";          
    pdg   = kPdgNuE;       
  }
  else 
  if ( dirname.find ("nu_mu_bar") != string::npos ) 
  { 
    label = "#bar{#nu_{#mu}}";  
    pdg   = kPdgAntiNuMu;  
  }
  else 
  if ( dirname.find ("nu_mu") != string::npos ) 
  { 
    label ="#nu_{#mu}";       
    pdg   = kPdgNuMu;      
  }
  else 
  if ( dirname.find ("nu_tau_bar") != string::npos ) 
  { 
    label = "#bar{#nu_{#tau}}"; 
    pdg   = kPdgAntiNuTau; 
  }
  else 
  if ( dirname.find ("nu_tau") != string::npos ) 
  { 
    label = "#nu_{#tau}";      
    pdg   = kPdgNuTau;     
  }

  gCurrProbeLbl     = label;
  gCurrProbePdg     = pdg;
  gCurrProbeIsNu    = pdg::IsNeutrino     (pdg);
  gCurrProbeIsNuBar = pdg::IsAntiNeutrino (pdg);
}
//_________________________________________________________________________________
void DirNameToTarget(void)
{
// Figure out the target type from the input directory name
//

  string label;
  string dirname = gDirName;

  if ( gCurrProbePdg == kPdgAntiNuE )  
  { 
    label = dirname.substr ( dirname.find("nu_e_bar")+9, dirname.length() ); 
  }
  else 
  if ( gCurrProbePdg == kPdgNuE )  
  { 
    label = dirname.substr ( dirname.find("nu_e")+5, dirname.length() ); 
  }
  else 
  if ( gCurrProbePdg == kPdgAntiNuMu )  
  { 
    label = dirname.substr ( dirname.find("nu_mu_bar")+10, dirname.length() ); 
  }
  else 
  if ( gCurrProbePdg == kPdgNuMu )  
  { 
    label = dirname.substr ( dirname.find("nu_mu")+6, dirname.length() ); 
  }
  else 
  if ( gCurrProbePdg == kPdgAntiNuTau )  
  { 
    label = dirname.substr ( dirname.find("nu_tau_bar")+11, dirname.length() ); 
  }
  else 
  if ( gCurrProbePdg == kPdgNuTau )  
  { 
    label = dirname.substr ( dirname.find("nu_tau")+7, dirname.length() ); 
  }

  bool free_n   = (label.size()==1 && label.find("n") !=string::npos);
  bool free_p   = (label.size()==2 && label.find("H1")!=string::npos);
  bool free_nuc = free_p || free_n;

  gCurrTargetHasP = (free_n) ? false : true;
  gCurrTargetHasN = (free_p) ? false : true;

  gCurrTargetLbl  = (free_nuc) ? "" : ("(" + label + ")");

  gCurrTargetIsFreeNuc  = free_nuc;
}
//_________________________________________________________________________________
void MakePlots(void)
{
  // Open files
  gXSecFile_curr = new TFile(gOptXSecFilename_curr.c_str(), "read");
  gXSecFile_ref0 = 0;
  if (gOptHaveRef) {
    gXSecFile_ref0 = new TFile(gOptXSecFilename_ref0.c_str(), "read");
  }

  // Get list of directories in the input file
  TList * directories = gXSecFile_curr->GetListOfKeys();

  // Loop over directories & generate plots for each one
  int ndir = directories->GetEntries();
  for(int idir=0; idir<ndir; idir++) {
    TObjString * dir = (TObjString*) directories->At(idir);
    gDirName = dir->GetString().Data();
    MakePlotsCurrDir();
  }
}
//_________________________________________________________________________________
void MakePlotsCurrDir(void)
{
  LOG("gxscomp", pINFO)  << "Plotting graphs from directory: " << gDirName;

  OpenDir         ();
  DirNameToProbe  ();
  DirNameToTarget ();

  const char * probestr  = gCurrProbeLbl.c_str();
  const char * targetstr = gCurrTargetLbl.c_str();

  LOG("gxscomp", pINFO)  << "Probe  : " << probestr;
  LOG("gxscomp", pINFO)  << "Target : " << targetstr;

  //
  // Start plotting...
  //

  if(!gCurrTargetIsFreeNuc) {
    Draw("tot_cc", Form("%s + %s, TOT CC",probestr,targetstr));
    Draw("tot_nc", Form("%s + %s, TOT NC",probestr,targetstr));
  }

  if(gCurrTargetHasN) {
      Draw("tot_cc_n",               Form("%s + n %s, TOT CC"                                , probestr, targetstr));
      Draw("tot_nc_n",               Form("%s + n %s, TOT NC"                                , probestr, targetstr));
    if(gCurrProbeIsNu) {
      Draw("qel_cc_n",               Form("%s + n %s, QEL CC"                                , probestr, targetstr));
    }
      Draw("qel_nc_n",               Form("%s + n %s, NCEL"                                  , probestr, targetstr));
      Draw("res_cc_n",               Form("%s + n %s, RES CC"                                , probestr, targetstr));
      Draw("res_nc_n",               Form("%s + n %s, RES NC"                                , probestr, targetstr));
      Draw("res_cc_n_1232P33",       Form("%s + n %s, RES CC, P33(1232)"                     , probestr, targetstr));
      Draw("res_cc_n_1535S11",       Form("%s + n %s, RES CC, S11(1535)"                     , probestr, targetstr));
      Draw("res_cc_n_1520D13",       Form("%s + n %s, RES CC, D13(1520)"                     , probestr, targetstr));
      Draw("res_cc_n_1650S11",       Form("%s + n %s, RES CC, S11(1650)"                     , probestr, targetstr));
      Draw("res_cc_n_1700D13",       Form("%s + n %s, RES CC, D13(1700)"                     , probestr, targetstr));
      Draw("res_cc_n_1675D15",       Form("%s + n %s, RES CC, D15(1675)"                     , probestr, targetstr));
      Draw("res_cc_n_1620S31",       Form("%s + n %s, RES CC, S31(1620)"                     , probestr, targetstr));
      Draw("res_cc_n_1700D33",       Form("%s + n %s, RES CC, D33(1700)"                     , probestr, targetstr));
      Draw("res_cc_n_1440P11",       Form("%s + n %s, RES CC, P11(1440)"                     , probestr, targetstr));
      Draw("res_cc_n_1720P13",       Form("%s + n %s, RES CC, P13(1720)"                     , probestr, targetstr));
      Draw("res_cc_n_1680F15",       Form("%s + n %s, RES CC, F15(1680)"                     , probestr, targetstr));
      Draw("res_cc_n_1910P31",       Form("%s + n %s, RES CC, P31(1910)"                     , probestr, targetstr));
      Draw("res_cc_n_1920P33",       Form("%s + n %s, RES CC, P33(1920)"                     , probestr, targetstr));
      Draw("res_cc_n_1905F35",       Form("%s + n %s, RES CC, F35(1905)"                     , probestr, targetstr));
      Draw("res_cc_n_1950F37",       Form("%s + n %s, RES CC, F37(1950)"                     , probestr, targetstr));
      Draw("res_cc_n_1710P11",       Form("%s + n %s, RES CC, P11(1710)"                     , probestr, targetstr));
      Draw("res_nc_n_1232P33",       Form("%s + n %s, RES NC, P33(1232)"                     , probestr, targetstr));
      Draw("res_nc_n_1535S11",       Form("%s + n %s, RES NC, S11(1535)"                     , probestr, targetstr));
      Draw("res_nc_n_1520D13",       Form("%s + n %s, RES NC, D13(1520)"                     , probestr, targetstr));
      Draw("res_nc_n_1650S11",       Form("%s + n %s, RES NC, S11(1650)"                     , probestr, targetstr));
      Draw("res_nc_n_1700D13",       Form("%s + n %s, RES NC, D13(1700)"                     , probestr, targetstr));
      Draw("res_nc_n_1675D15",       Form("%s + n %s, RES NC, D15(1675)"                     , probestr, targetstr));
      Draw("res_nc_n_1620S31",       Form("%s + n %s, RES NC, S31(1620)"                     , probestr, targetstr));
      Draw("res_nc_n_1700D33",       Form("%s + n %s, RES NC, D33(1700)"                     , probestr, targetstr));
      Draw("res_nc_n_1440P11",       Form("%s + n %s, RES NC, P11(1440)"                     , probestr, targetstr));
      Draw("res_nc_n_1720P13",       Form("%s + n %s, RES NC, P13(1720)"                     , probestr, targetstr));
      Draw("res_nc_n_1680F15",       Form("%s + n %s, RES NC, F15(1680)"                     , probestr, targetstr));
      Draw("res_nc_n_1910P31",       Form("%s + n %s, RES NC, P31(1910)"                     , probestr, targetstr));
      Draw("res_nc_n_1920P33",       Form("%s + n %s, RES NC, P33(1920)"                     , probestr, targetstr));
      Draw("res_nc_n_1905F35",       Form("%s + n %s, RES NC, F35(1905)"                     , probestr, targetstr));
      Draw("res_nc_n_1950F37",       Form("%s + n %s, RES NC, F37(1950)"                     , probestr, targetstr));
      Draw("res_nc_n_1710P11",       Form("%s + n %s, RES NC, P11(1710)"                     , probestr, targetstr));
      Draw("dis_cc_n",               Form("%s + n %s, DIS CC"                                , probestr, targetstr));
      Draw("dis_nc_n",               Form("%s + n %s, DIS NC"                                , probestr, targetstr));
    if(gCurrProbeIsNu) {    
      Draw("dis_cc_n_ubarsea",       Form("%s + n %s, DIS CC (#bar{u}_{sea})"                , probestr, targetstr));
      Draw("dis_cc_n_dval",          Form("%s + n %s, DIS CC (d_{val})"                      , probestr, targetstr));
      Draw("dis_cc_n_dsea",          Form("%s + n %s, DIS CC (d_{sea})"                      , probestr, targetstr));
      Draw("dis_cc_n_ssea",          Form("%s + n %s, DIS CC (s_{sea})"                      , probestr, targetstr));
    }
    if(gCurrProbeIsNuBar) {    
      Draw("dis_cc_n_sbarsea",       Form("%s + n %s, DIS CC (#bar{s}_{sea})"                , probestr, targetstr));
      Draw("dis_cc_n_dbarsea",       Form("%s + n %s, DIS CC (#bar{d}_{val})"                , probestr, targetstr));
      Draw("dis_cc_n_uval",          Form("%s + n %s, DIS CC (u_{val})"                      , probestr, targetstr));
      Draw("dis_cc_n_usea",          Form("%s + n %s, DIS CC (u_{sea})"                      , probestr, targetstr));
    }
      Draw("dis_nc_n_sbarsea",       Form("%s + n %s, DIS NC (#bar{s}_{sea})"                , probestr, targetstr));
      Draw("dis_nc_n_ubarsea",       Form("%s + n %s, DIS NC (#bar{u}_{sea})"                , probestr, targetstr));
      Draw("dis_nc_n_dbarsea",       Form("%s + n %s, DIS NC (#bar{d}_{sea})"                , probestr, targetstr));
      Draw("dis_nc_n_dval",          Form("%s + n %s, DIS NC (d_{val})"                      , probestr, targetstr));
      Draw("dis_nc_n_dsea",          Form("%s + n %s, DIS NC (d_{sea})"                      , probestr, targetstr));
      Draw("dis_nc_n_uval",          Form("%s + n %s, DIS NC (u_{val})"                      , probestr, targetstr));
      Draw("dis_nc_n_usea",          Form("%s + n %s, DIS NC (u_{sea})"                      , probestr, targetstr));
      Draw("dis_nc_n_ssea",          Form("%s + n %s, DIS NC (s_{sea})"                      , probestr, targetstr));
    if(gCurrProbeIsNu) {    
      Draw("dis_cc_n_dval_charm",    Form("%s + n %s, DIS CC (d_{val} -> c)"                 , probestr, targetstr));
      Draw("dis_cc_n_dsea_charm",    Form("%s + n %s, DIS CC (d_{sea} -> c)"                 , probestr, targetstr));
      Draw("dis_cc_n_ssea_charm",    Form("%s + n %s, DIS CC (s_{sea} -> c)"                 , probestr, targetstr));
    }
    if(gCurrProbeIsNuBar) {    
      Draw("dis_cc_n_dbarsea_charm", Form("%s + n %s, DIS CC (#bar{d}_{sea} -> #bar{c})"     , probestr, targetstr));
      Draw("dis_cc_n_sbarsea_charm", Form("%s + n %s, DIS CC (#bar{s}_{sea} -> #bar{c})"     , probestr, targetstr));
    }
  }//N>0?


  if(gCurrTargetHasP) {
      Draw("tot_cc_p",               Form("%s + p %s, TOT CC"                                , probestr, targetstr));
      Draw("tot_nc_p",               Form("%s + p %s, TOT NC"                                , probestr, targetstr));
    if(gCurrProbeIsNuBar) {
      Draw("qel_cc_p",               Form("%s + p %s, QEL CC"                                , probestr, targetstr));
    }
      Draw("qel_nc_p",               Form("%s + p %s, NCEL"                                  , probestr, targetstr));
      Draw("res_cc_p",               Form("%s + p %s, RES CC"                                , probestr, targetstr));
      Draw("res_nc_p",               Form("%s + p %s, RES NC"                                , probestr, targetstr));
      Draw("res_cc_p_1232P33",       Form("%s + p %s, RES CC, P33(1232)"                     , probestr, targetstr));
      Draw("res_cc_p_1535S11",       Form("%s + p %s, RES CC, S11(1535)"                     , probestr, targetstr));
      Draw("res_cc_p_1520D13",       Form("%s + p %s, RES CC, D13(1520)"                     , probestr, targetstr));
      Draw("res_cc_p_1650S11",       Form("%s + p %s, RES CC, S11(1650)"                     , probestr, targetstr));
      Draw("res_cc_p_1700D13",       Form("%s + p %s, RES CC, D13(1700)"                     , probestr, targetstr));
      Draw("res_cc_p_1675D15",       Form("%s + p %s, RES CC, D15(1675)"                     , probestr, targetstr));
      Draw("res_cc_p_1620S31",       Form("%s + p %s, RES CC, S31(1620)"                     , probestr, targetstr));
      Draw("res_cc_p_1700D33",       Form("%s + p %s, RES CC, D33(1700)"                     , probestr, targetstr));
      Draw("res_cc_p_1440P11",       Form("%s + p %s, RES CC, P11(1440)"                     , probestr, targetstr));
      Draw("res_cc_p_1720P13",       Form("%s + p %s, RES CC, P13(1720)"                     , probestr, targetstr));
      Draw("res_cc_p_1680F15",       Form("%s + p %s, RES CC, F15(1680)"                     , probestr, targetstr));
      Draw("res_cc_p_1910P31",       Form("%s + p %s, RES CC, P31(1910)"                     , probestr, targetstr));
      Draw("res_cc_p_1920P33",       Form("%s + p %s, RES CC, P33(1920)"                     , probestr, targetstr));
      Draw("res_cc_p_1905F35",       Form("%s + p %s, RES CC, F35(1905)"                     , probestr, targetstr));
      Draw("res_cc_p_1950F37",       Form("%s + p %s, RES CC, F37(1950)"                     , probestr, targetstr));
      Draw("res_cc_p_1710P11",       Form("%s + p %s, RES CC, P11(1710)"                     , probestr, targetstr));
      Draw("res_nc_p_1232P33",       Form("%s + p %s, RES NC, P33(1232)"                     , probestr, targetstr));
      Draw("res_nc_p_1535S11",       Form("%s + p %s, RES NC, S11(1535)"                     , probestr, targetstr));
      Draw("res_nc_p_1520D13",       Form("%s + p %s, RES NC, D13(1520)"                     , probestr, targetstr));
      Draw("res_nc_p_1650S11",       Form("%s + p %s, RES NC, S11(1650)"                     , probestr, targetstr));
      Draw("res_nc_p_1700D13",       Form("%s + p %s, RES NC, D13(1700)"                     , probestr, targetstr));
      Draw("res_nc_p_1675D15",       Form("%s + p %s, RES NC, D15(1675)"                     , probestr, targetstr));
      Draw("res_nc_p_1620S31",       Form("%s + p %s, RES NC, S31(1620)"                     , probestr, targetstr));
      Draw("res_nc_p_1700D33",       Form("%s + p %s, RES NC, D33(1700)"                     , probestr, targetstr));
      Draw("res_nc_p_1440P11",       Form("%s + p %s, RES NC, P11(1440)"                     , probestr, targetstr));
      Draw("res_nc_p_1720P13",       Form("%s + p %s, RES NC, P13(1720)"                     , probestr, targetstr));
      Draw("res_nc_p_1680F15",       Form("%s + p %s, RES NC, F15(1680)"                     , probestr, targetstr));
      Draw("res_nc_p_1910P31",       Form("%s + p %s, RES NC, P31(1910)"                     , probestr, targetstr));
      Draw("res_nc_p_1920P33",       Form("%s + p %s, RES NC, P33(1920)"                     , probestr, targetstr));
      Draw("res_nc_p_1905F35",       Form("%s + p %s, RES NC, F35(1905)"                     , probestr, targetstr));
      Draw("res_nc_p_1950F37",       Form("%s + p %s, RES NC, F37(1950)"                     , probestr, targetstr));
      Draw("res_nc_p_1710P11",       Form("%s + p %s, RES NC, P11(1710)"                     , probestr, targetstr));
      Draw("dis_cc_p",               Form("%s + p %s, DIS CC"                                , probestr, targetstr));
      Draw("dis_nc_p",               Form("%s + p %s, DIS NC"                                , probestr, targetstr));
    if(gCurrProbeIsNu) {    
      Draw("dis_cc_p_ubarsea",       Form("%s + p %s, DIS CC (#bar{u}_{sea})"                , probestr, targetstr));
      Draw("dis_cc_p_dval",          Form("%s + p %s, DIS CC (d_{val})"                      , probestr, targetstr));
      Draw("dis_cc_p_dsea",          Form("%s + p %s, DIS CC (d_{sea})"                      , probestr, targetstr));
      Draw("dis_cc_p_ssea",          Form("%s + p %s, DIS CC (s_{sea})"                      , probestr, targetstr));
    }
    if(gCurrProbeIsNuBar) {    
      Draw("dis_cc_n_sbarsea",       Form("%s + n %s, DIS CC (#bar{s}_{sea})"                , probestr, targetstr));
      Draw("dis_cc_n_dbarsea",       Form("%s + n %s, DIS CC (#bar{d}_{val})"                , probestr, targetstr));
      Draw("dis_cc_n_uval",          Form("%s + n %s, DIS CC (u_{val})"                      , probestr, targetstr));
      Draw("dis_cc_n_usea",          Form("%s + n %s, DIS CC (u_{sea})"                      , probestr, targetstr));
    }
      Draw("dis_nc_p_sbarsea",       Form("%s + p %s, DIS NC (#bar{s}_{sea})"                , probestr, targetstr));
      Draw("dis_nc_p_ubarsea",       Form("%s + p %s, DIS NC (#bar{u}_{sea})"                , probestr, targetstr));
      Draw("dis_nc_p_dbarsea",       Form("%s + p %s, DIS NC (#bar{d}_{sea})"                , probestr, targetstr));
      Draw("dis_nc_p_dval",          Form("%s + p %s, DIS NC (d_{val})"                      , probestr, targetstr));
      Draw("dis_nc_p_dsea",          Form("%s + p %s, DIS NC (d_{sea})"                      , probestr, targetstr));
      Draw("dis_nc_p_uval",          Form("%s + p %s, DIS NC (u_{val})"                      , probestr, targetstr));
      Draw("dis_nc_p_usea",          Form("%s + p %s, DIS NC (u_{sea})"                      , probestr, targetstr));
      Draw("dis_nc_p_ssea",          Form("%s + p %s, DIS NC (s_{sea})"                      , probestr, targetstr));
    if(gCurrProbeIsNu) {    
      Draw("dis_cc_p_dval_charm",    Form("%s + p %s, DIS CC (d_{val} -> charm)"             , probestr, targetstr));
      Draw("dis_cc_p_dsea_charm",    Form("%s + p %s, DIS CC (d_{sea} -> charm)"             , probestr, targetstr));
      Draw("dis_cc_p_ssea_charm",    Form("%s + p %s, DIS CC (s_{sea} -> charm)"             , probestr, targetstr));
    }
    if(gCurrProbeIsNuBar) {    
      Draw("dis_cc_p_dbarsea_charm", Form("%s + p %s, DIS CC (#bar{d}_{sea} -> #bar{charm})" , probestr, targetstr));
      Draw("dis_cc_p_sbarsea_charm", Form("%s + p %s, DIS CC (#bar{s}_{sea} -> #bar{charm})" , probestr, targetstr));
    }
  }//Z>0

}
//_________________________________________________________________________________
TH1F* DrawFrame(TGraph * gr0, TGraph * gr1, TPad * p, const char * xt, const char * yt, double yminsc, double ymaxsc)
{
  double xmin = 1E-5;
  double xmax = 1;
  double ymin = 1E-5;
  double ymax = 1;

  if(gr0) {  
    TAxis * x0 = gr0 -> GetXaxis();
    TAxis * y0 = gr0 -> GetYaxis();
    xmin = x0 -> GetXmin();
    xmax = x0 -> GetXmax();
    ymin = y0 -> GetXmin();
    ymax = y0 -> GetXmax();
  }
  if(gr1) {
     TAxis * x1 = gr1 -> GetXaxis();
     TAxis * y1 = gr1 -> GetYaxis();
     xmin = TMath::Min(xmin, x1 -> GetXmin());
     xmax = TMath::Max(xmax, x1 -> GetXmax());
     ymin = TMath::Min(ymin, y1 -> GetXmin());
     ymax = TMath::Max(ymax, y1 -> GetXmax());
  }
  xmin *= 0.5;
  xmax *= 1.5;
  ymin *= yminsc;
  ymax *= ymaxsc;
  xmin = TMath::Max(0.1, xmin);
  
  return DrawFrame(xmin, xmax, ymin, ymax, p, xt, yt);
}
//_________________________________________________________________________________
TH1F* DrawFrame(double xmin, double xmax, double ymin, double ymax, TPad * p, const char * xt, const char * yt)
{
  if(!p) return 0;
  TH1F * hf = (TH1F*) p->DrawFrame(xmin, ymin, xmax, ymax);
  hf->GetXaxis()->SetTitle(xt);
  hf->GetYaxis()->SetTitle(yt);
  hf->GetYaxis()->SetTitleSize(0.03);
  hf->GetYaxis()->SetTitleOffset(1.5);
  hf->GetXaxis()->SetLabelSize(0.03);
  hf->GetYaxis()->SetLabelSize(0.03);
  return hf;
}
//_________________________________________________________________________________
void Draw(TGraph* gr, const char * opt)
{
  if(!gr) return;
  gr->Draw(opt);
}
//_________________________________________________________________________________
void Draw(const char * plot, const char * title)
{
  gPadTitle->cd();
  TPavesText hdr(10,10,90,90,1,"tr");
  hdr.AddText(title);
  hdr.SetFillColor(kWhite);
  hdr.Draw();

  gPadXSecs->cd();
  TGraph * gr_curr = (TGraph*) gDirCurr->Get(plot);
  TGraph * gr_ref0 = 0;
  if(gDirRef0) {
      gr_ref0 = (TGraph*) gDirRef0->Get(plot);
  }

  if(gr_curr == 0 && gr_ref0 ==0) return;

  if(!title) return;

  // trim points in reference plot (shown with markers) so that the markers
  // don't hide the current prediction (shown with a line).
  // Keep, at most, 20 points per decade.
  TGraph * gr_ref0_trim = TrimGraph(gr_ref0, 20);

  utils::style::Format(gr_curr,      1,1,1,1,1,1.0);
  utils::style::Format(gr_ref0_trim, 1,1,1,2,4,0.7);

  DrawFrame (gr_curr, gr_ref0, gPadXSecs, "E_{#nu} (GeV)", "#sigma (10^{-38} cm^{2})", 0.5, 1.5);
  Draw (gr_curr,      "L");
  Draw (gr_ref0_trim, "P");

  gLS->Clear();
  gLS->AddEntry(gr_curr, gLabelCurr.c_str(), "L");
  if(gOptHaveRef) {
    gLS->AddEntry(gr_ref0_trim, gLabelRef0.c_str(), "P");
  }
  gLS->Draw();

  // plot ratio of current and reference models
  if(gOptHaveRef) 
  {
    gPadRatio->cd();
    TGraph * gr_ratio = DrawRatio(gr_curr, gr_ref0);
    DrawFrame (gr_ratio, 0, gPadRatio, "E_{#nu} (GeV)", Form("%s / %s", gLabelCurr.c_str(), gLabelRef0.c_str()), 0.9, 1.1);
    Draw      (gr_ratio, "L");
  }

  gC ->Update();

  if(gr_ref0_trim) {
    delete gr_ref0_trim;
  }

  gPS->NewPage();
}
//_________________________________________________________________________________
TGraph * TrimGraph(TGraph * gr, int max_np_per_decade)
{
  if(!gr) return 0;

  const int np = gr->GetN();
  vector<bool> remv(np);

  int    fp   = 0; 
  int    lp   = 0; 

  while(1) {
    double xmin = gr->GetX()[fp];
    double xmax = 10 * xmin;
    int ndec = 0;
    for(int i=fp; i<np; i++) {
       remv[i] = false;
       lp = i;
       double x = gr->GetX()[i];
       if(x > xmin && x <= xmax) ndec++;
       if(x > xmax) break;
    }
    if(ndec > max_np_per_decade) {
       double keep_prob = (double)max_np_per_decade/ (double) ndec;
       int keep_rate = TMath::FloorNint(1./keep_prob);
       int ithrow = 0;
       for(int i=fp; i<=lp; i++) {
         if(ithrow % keep_rate) {
           remv[i] = true;
         }
         ithrow++;
       } 
    }
    if(lp == np-1) break;
    fp = lp+1;
  }
 
  int np2 = 0;
  for(int i=0; i<np; i++) {
   if(!remv[i]) {
     np2++;
   }
  }

  double * x = new double[np2];
  double * y = new double[np2];

  int j=0;
  for(int i=0; i<np; i++) {
   if(!remv[i]) {
     x[j] = gr->GetX()[i];
     y[j] = gr->GetY()[i];
     j++;
   }
  }
  TGraph * gr2 = new TGraph(np2,x,y);

  delete [] x;
  delete [] y;

  return gr2;
}
//_________________________________________________________________________________
TGraph * DrawRatio(TGraph * gr0, TGraph * gr1)
{
// gr0 / gr1

  if(!gr0) return 0;
  if(!gr1) return 0;

  LOG("gxscomp", pDEBUG) << "Drawing ratio...";

  const int np = gr0->GetN();

  double * x = new double[np];
  double * y = new double[np];

  for(int i=0; i<np; i++) {
     x[i] = gr0->GetX()[i];
     y[i] = 0;

     if(gr0->Eval(x[i]) != 0. && gr1->Eval(x[i]) != 0.) 
     {
       y[i] = gr0->Eval(x[i]) / gr1->Eval(x[i]);
     }
     else {
       if(gr0->Eval(x[i]) != 0.) 
       {
         y[i] = -1;
       }
       else 
       {
         y[i] =  1;
       }
     }
  }

  TGraph * ratio = new TGraph(np,x,y);

  delete [] x;
  delete [] y;

  return ratio;
}
//_________________________________________________________________________________
void GetCommandLineArgs(int argc, char ** argv)
{
  LOG("gxscomp", pINFO) << "*** Parsing command line arguments";

  CmdLnArgParser parser(argc,argv);

  // get input GENIE cross section file
  if( parser.OptionExists('f') ) {
    string inp = parser.ArgAsString('f');
    if(inp.find(",") != string::npos) {
       vector<string> inpv = utils::str::Split(inp,",");
       assert(inpv.size()==2);
       gOptXSecFilename_curr = inpv[0];
       gLabelCurr = inpv[1];
    } else {
       gOptXSecFilename_curr = inp;
       gLabelCurr = "current";
    }
    bool ok = CheckRootFilename(gOptXSecFilename_curr.c_str());
    if(!ok) {
      PrintSyntax();
      exit(1);
    }
  } else {
    PrintSyntax();
    exit(1);
  }

  // get [reference] input GENIE cross section file
  if( parser.OptionExists('r') ) {
    string inp = parser.ArgAsString('r');
    if(inp.find(",") != string::npos) {
       vector<string> inpv = utils::str::Split(inp,",");
       assert(inpv.size()==2);
       gOptXSecFilename_ref0 = inpv[0];
       gLabelRef0 = inpv[1];
    } else {
       gOptXSecFilename_ref0 = inp;
       gLabelRef0 = "reference";
    }
    bool ok = CheckRootFilename(gOptXSecFilename_ref0.c_str());
    if(!ok) {
      PrintSyntax();
      exit(1);
    }
    gOptHaveRef = true;
  } else {
    LOG("gxscomp", pNOTICE) << "No reference cross section file";
    gOptHaveRef = false;
  }

  // get output filename
  if( parser.OptionExists('o') ) {
    gOptOutputFilename = parser.ArgAsString('o');
  } else {
    gOptOutputFilename = "xsec.ps";
  }

}
//_________________________________________________________________________________
void PrintSyntax(void)
{
  LOG("gxscomp", pNOTICE)
    << "\n\n" << "Syntax:" << "\n"
    << " gxscomp  -f xsec_file [-r reference_xsec_file] [-o output]\n";
}
//_________________________________________________________________________________
bool CheckRootFilename(string filename)
{
  if(filename.size() == 0) return false;

  bool is_accessible = ! (gSystem->AccessPathName(filename.c_str()));
  if (!is_accessible) {
   LOG("gxscomp", pERROR)
       << "The input ROOT file [" << filename << "] is not accessible";
   return false;
  }
  return true;
}
//_________________________________________________________________________________
string OutputFileName(string inpname)
{
// Builds the output filename based on the name of the input filename
// Perfors the following conversion: name.root -> name.nuxsec_test.ps

  unsigned int L = inpname.length();

  // if the last 4 characters are "root" (ROOT file extension) then
  // remove them
  if(inpname.substr(L-4, L).find("root") != string::npos) {
    inpname.erase(L-4, L);
  }

  ostringstream name;
  name << inpname << "nuxsec_test.ps";

  return gSystem->BaseName(name.str().c_str());
}
//_________________________________________________________________________________

