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
  //TBranch* QDCBranch = (TBranch*)ResultsTree->GetBranch("QDCsimple");
  //TBranch* SignalBranch = (TBranch*)ResultsTree->GetBranch("Signal");
  //TBranch* TimeBranch = (TBranch*)ResultsTree->GetBranch("TimeSimple");
  //TBranch* BLOKidxBranch = (TBranch*)ResultsTree->GetBranch("BLOKidx");


  TH1D* hnewLEDA;
  //char* name1 = strcat("LED ON, PMT A, ", filename);
  TCanvas* c2 = new TCanvas("LED ON, PMT A", filename, 2);
  //Get LED ON events for PMT A
  Long64_t nentriesLEDA = ResultsTree->Draw("QDCsimple[0]>>hnewLEDA", "(130000<QDCsimple[2])&&(QDCsimple[2]<160000)&&(BLOKidx<900)&&(1000<TimeSimple[0])&&(TimeSimple[0]<1080)");
  hnewLEDA = (TH1D*)gDirectory->Get("hnewLEDA");
  Double_t medianLEDA = median1(hnewLEDA); 

  TH1D* hnewLEDB;
  //char* name2 = strcat("LED ON, PMT B, ", filename);
  TCanvas* c3 = new TCanvas("LED ON, PMT B", filename, 3);
  //Get LED ON events for PMT B
  Long64_t nentriesLEDB = ResultsTree->Draw("QDCsimple[1]>>hnewLEDB", "(130000<QDCsimple[2])&&(QDCsimple[2]<160000)&&(BLOKidx<900)&&(1000<TimeSimple[1])&&(TimeSimple[1]<1080)");
  hnewLEDB = (TH1D*)gDirectory->Get("hnewLEDB");
  Double_t medianLEDB = median1(hnewLEDB); 

  TH1D* hnewBeamA;
  //char *name3 = strcat("LED ON, Beam A, ", filename);
  TCanvas* c4 = new TCanvas("LED ON, Beam A", filename, 4);
  //Get BeamON events for PMT A
  Long64_t nentriesBeamA = ResultsTree->Draw("QDCsimple[0]>>hnewBeamA", "(QDCsimple[0]>-100)&&(QDCsimple[0]<100000)&&(390<TimeSimple[2])&&(TimeSimple[2]<450)&&(390<TimeSimple[3])&&(TimeSimple[3]<450)&&(BLOKidx[3]<300)&&(325<TimeSimple[0])&&(TimeSimple[0]<375)");
  hnewBeamA = (TH1D*)gDirectory->Get("hnewBeamA");
  Double_t medianBeamA = median1(hnewBeamA); 

  TH1D* hnewBeamB;
  //char* name4 = strcat("LED ON, Beam B, ", filename);
  TCanvas* c5 = new TCanvas("LED ON, Beam B", filename, 5);
  //Get BeamON events for PMT B
  Long64_t nentriesBeamB = ResultsTree->Draw("QDCsimple[1]>>hnewBeamB", "(QDCsimple[1]>-100)&&(QDCsimple[1]<100000)&&(390<TimeSimple[2])&&(TimeSimple[2]<450)&&(390<TimeSimple[3])&&(TimeSimple[3]<450)&&(BLOKidx[3]<300)&&(325<TimeSimple[1])&&(TimeSimple[1]<375)");
  hnewBeamB = (TH1D*)gDirectory->Get("hnewBeamB");
  Double_t medianBeamB = median1(hnewBeamB); 

    
  //printf("Median of Slice %d, Median=%g, Mean = %g\n",i,median,mean); 
  cout<<"---------------------------------------------------------------"<<endl;
  cout << "Median Data for file: \t" << filename << endl;
  cout <<"\tPMTA, LED \tPMTB, LED \tPMTA, Beam  \tPMTB, Beam"
       << endl;
  cout << "Median \t" <<   medianLEDA << "\t" << medianLEDB << "\t"
       << medianBeamA << "\t" << medianBeamB << endl;
  cout << "# events \t" << nentriesLEDA << "\t" << nentriesLEDB
       << "\t" << nentriesBeamA << "\t" << nentriesBeamB << endl;
  cout<<"---------------------------------------------------------------"<<endl;

   f->Close();
  
} 
