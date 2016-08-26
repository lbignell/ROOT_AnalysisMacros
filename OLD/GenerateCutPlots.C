//This is a macro intended to generate a series of plots illustrating the cuts
//used for the data as they are successively applied (to look for bias).
#include "TROOT.h"
#include "TH1.h"
#include "TMath.h"
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include <iostream>

using namespace std;


void GenerateCutPlots(const char* filename){
  //open the file
  TFile* f = TFile::Open(filename);
  TTree* ResultsTree = (TTree*)f->Get("Results");
  TCanvas* c1 = new TCanvas("Beam ON QDC", filename, 1);
  //Plot LED events with no cuts, PMTA.
  TH1D* hLEDAnone;
  Long64_t nentriesBeamNoCutsA =
    ResultsTree->Draw("QDCsimple[0]>>hLEDAnone(500, -100, 900)",
		      "",
		      "");
  //Plot LED events with 1st cut; Ch2 pulse on, PMTA.
  hLEDAnone->GetXaxis()->SetTitle("QDC (A.U.)");
  hLEDAnone->SetLineColor(1);//1 = black, 2 = red, 4 = blue, 6 = magenta
  TH1D* hLEDA1;
  Long64_t nentriesBeamCut1A = 
    ResultsTree->Draw("QDCsimple[0]>>hLEDA1(500, -100, 900)",
		      "(130000<QDCsimple[2])&&(QDCsimple[2]<160000)",
		      "SAME");
  hLEDA1->SetLineColor(2);
  //Plot LED events with 2nd cut, PMT on time, PMTA.
  TH1D* hLEDA2;
  Long64_t nentriesBeamCut2A = 
    ResultsTree->Draw("QDCsimple[0]>>hLEDA2(500, -100, 900)",
		      "(130000<QDCsimple[2])&&(QDCsimple[2]<160000)&&(1000<TimeSimple[0])&&(TimeSimple[0]<1080)",
		      "SAME");
  hLEDA2->SetLineColor(4);

  c1->SetLogy(1);

  //Now save the plot to the folder...


}
