#include "TROOT.h"
#include "TH1.h"
#include "TMath.h"
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include <iostream>

using namespace std;

Double_t median1(TH1 *h1) { 
   //compute the median for 1-d histogram h1 
   Int_t nbins = h1->GetXaxis()->GetNbins(); 
   Double_t *x = new Double_t[nbins]; 
   Double_t *y = new Double_t[nbins]; 
   for (Int_t i=0;i<nbins;i++) {
      x[i] = h1->GetXaxis()->GetBinCenter(i+1); 
      y[i] = h1->GetBinContent(i+1); 
   } 
   Double_t median = TMath::Median(nbins,x,y); 
   delete [] x; 
   delete [] y; 
   return median; 
} 

void MedianSimple(const char* filename) { 

  //Open the file
  TFile* f = new TFile(filename);
  //  char* thefilename = filename;
 
  TTree* ResultsTree = (TTree*)f->Get("Results");

  TH1D* hnewLEDA;
  TCanvas* c2 = new TCanvas("LED ON, PMT A", filename, 2);
  //Get LED ON events for PMT A
  Long64_t nentriesLEDA = ResultsTree->Draw("QDCsimple[0]>>hnewLEDA(100000, 0., 0.)", "(130000<QDCsimple[2])&&(QDCsimple[2]<160000)&&(1000<TimeSimple[0])&&(TimeSimple[0]<1080)");
  hnewLEDA = (TH1D*)gDirectory->Get("hnewLEDA");


  Double_t medianLEDA = median1(hnewLEDA); 
  Double_t meanLEDA = hnewLEDA->GetMean();
  delete hnewLEDA;
  c2->Clear();

  TH1D* hnewLEDB;
  Long64_t nentriesLEDB = ResultsTree->Draw("QDCsimple[1]>>hnewLEDB(100000, 0., 0.)", "(130000<QDCsimple[2])&&(QDCsimple[2]<160000)&&(1000<TimeSimple[1])&&(TimeSimple[1]<1080)");
  hnewLEDB = (TH1D*)gDirectory->Get("hnewLEDB");

  Double_t medianLEDB = median1(hnewLEDB);
  Double_t meanLEDB = hnewLEDB->GetMean(); 
  delete hnewLEDB;
  c2->Clear();

  TH1D* hnewBeamA;
  //Get BeamON events for PMT A
  //Long64_t nentriesBeamA = ResultsTree->Draw("QDCsimple[0]>>hnewBeamA", "(0<(TimeSimple[2]-TimeSimple[3]))&&((TimeSimple[2]-TimeSimple[3])<15)&&(-70<(TimeSimple[0]-TimeSimple[3]))&&((TimeSimple[0]-TimeSimple[3])<-45)&&(-75<(TimeSimple[0]-TimeSimple[2]))&&((TimeSimple[0]-TimeSimple[2])<-50)&&(325<TimeSimple[0])&&(TimeSimple[0]<385)");
  Long64_t nentriesBeamA = ResultsTree->Draw("QDCsimple[0]>>hnewBeamA(100000, 0., 0.)", "(325<TimeSimple[0])&&(TimeSimple[0]<375)&&(390<TimeSimple[2])&&(TimeSimple[2]<450)&&(390<TimeSimple[3])&&(TimeSimple[3]<450)");
  hnewBeamA = (TH1D*)gDirectory->Get("hnewBeamA");

  Double_t medianBeamA = median1(hnewBeamA); 
  Double_t meanBeamA = hnewBeamA->GetMean();
  delete hnewBeamA;
  c2->Clear();

  TH1D* hnewBeamB;
  //Get BeamON events for PMT B
  Long64_t nentriesBeamB = ResultsTree->Draw("QDCsimple[1]>>hnewBeamB(100000, 0., 0.)", "(325<TimeSimple[1])&&(TimeSimple[1]<375)&&(390<TimeSimple[2])&&(TimeSimple[2]<450)&&(390<TimeSimple[3])&&(TimeSimple[3]<450)");

  hnewBeamB = (TH1D*)gDirectory->Get("hnewBeamB");

  Double_t medianBeamB = median1(hnewBeamB); 
  Double_t meanBeamB = hnewBeamB->GetMean();
  delete hnewBeamB;
  c2->Clear();
   
  cout<<"---------------------------------------------------------------"<<endl;
  cout << "Median Data for file: \t" << filename << endl;
  cout <<"\tPMTA, LED \tPMTB, LED \tPMTA, Beam  \tPMTB, Beam"
       << endl;
  cout << "Median \t" <<   medianLEDA << "\t" << medianLEDB << "\t"
       << medianBeamA << "\t" << medianBeamB << endl;
  cout << "Mean \t" <<   meanLEDA << "\t" << meanLEDB << "\t"
       << meanBeamA << "\t" << meanBeamB << endl;
  cout << "# events \t" << nentriesLEDA << "\t" << nentriesLEDB
       << "\t" << nentriesBeamA << "\t" << nentriesBeamB << endl;
  cout<<"---------------------------------------------------------------"<<endl;

   f->Close();
  
} 
