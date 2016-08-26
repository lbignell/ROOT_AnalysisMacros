//This is a macro to test out the convolution function, and set it with
//different parameters (to try and get some decent starting params).
//Based on TimeFitv2.C
#ifndef __TimeFit__
#define __Timefit__

#include "TROOT.h"
#include "TFile.h"
#include "TH1.h"
#include "TMath.h"
#include "TF1.h"
#include "TStyle.h"
#include "TBranch.h"
#include "TTree.h"
#include "TGraph.h"
#include "TLeaf.h"
#include "TCanvas.h"
#include <vector>
#include "TFitResultPtr.h"
#include "TGraphErrors.h"

using namespace std;

//Declare IRF as global
vector<double> IRF;

bool HazRun = true;//set false for some debug output

Double_t lowbound = 0;
Double_t upbound = 0;

class TimeFit{
public:
  //Member functions to be called
  void* GetPointerToValue(TBranch*, int, const char*);
  static Double_t ConvExp(Double_t*, Double_t*); //needs to be static so it
  //looks like proper function (as opposed to a member function) to ROOT. As
  //I've changed the program design to make IRF a global variable, having it in
  //a class is no longer necessary.
  //Constructor
  //TimeFit(){};
  //~TimeFit(){};

  //The code below didn't work, I still don't fully understand why; the compiler
  //wouldn't link it. Me or a ALCiC 'feature'?
  //void SetIRF(vector<double>&);
  //vector<double> IRF;
  //vector<double>& GetIRF();

};
  
//void TimeFit::SetIRF(vector<double>& theIRF){
  //TimeFit::IRF.clear();
  //IRF = theIRF;
//}

//vector<double>& TimeFit::GetIRF(){ return IRF; }

void* TimeFit::GetPointerToValue(TBranch* theBranch, int entry,
				const char* name){
  theBranch->GetEntry(entry);
  TLeaf* theLeaf = theBranch->GetLeaf(name);
  return theLeaf->GetValuePointer();
}

Double_t TimeFit::ConvExp(Double_t *x, Double_t *par){
  //Fit parameters
  //par[0] = Time offset for exponential decay signals
  //par[1] = Amplitude of short decay time component
  //par[2] = Short decay time constant
  //par[3] = Amplitude for long decay time component
  //par[4] = Long decay time constant

  //Access the avg cerenkov waveform using IRF.

  /////////////////////////////////////////////////////
  //This implementation is currently INCORRECT!
  //I am not integrating the overlap between the Wfm
  //and the exponential function! Rather, I am
  //Multiplying each point on the wfm by the summed
  //exponential fn. FIX THIS!
  /////////////////////////////////////////////////////

  //I want conv(x[0]) = sum_j {IRF(j)*DblExpFn(x[0]-j)}
  //Q: What should be the limits of +/- j?
  //A: ~300-500 should do it.

  // Control constants
  //Double_t Xlow = x[0]-500;
  //Double_t Xup = x[0]-par[0]; 
  Double_t conv = 0;
  Double_t ExpFn = 0;
  Double_t IRFval = 0;
  vector<double> theIRF = IRF;
  //theIRF.resize(2520, 0);// = TimeFit::IRF;
  //
  if(HazRun==false){
    cout << "First call of ConvExp" << endl;
    cout << "x[0] = " << x[0] << endl;
    //cout << "Xlow = " << Xlow << endl;
    //cout << "Xup = " << Xup << endl;
    cout << "theIRF.size() = " << theIRF.size() << endl;
    cout << "Running Loop... " << endl;
  }

  for(Double_t i = 300; i<500; i++){
    //Do the convolution
    IRFval = theIRF.at(i);
    //cout << "x[0] = " << x[0] << ", i = " << i << ", x[0]-i = " << (x[0]-i)
    //	 << endl;
    if((x[0]-i)<0){ ExpFn = 0; }
    else{
      ExpFn = par[0]*TMath::Exp(-(x[0]-i)/par[1]) + 
	par[2]*TMath::Exp(-(x[0]-i)/par[3]);
    }
    conv+=IRFval*ExpFn;
  }

  //for(Double_t i = Xlow; i<Xup; i++){
    //Do the convolution
    //if(i<1){
  //ExpFn = 0;
  //}
  //else{
  //  ExpFn = par[1]*TMath::Exp(-(i - par[0])/par[2]) + 
  //	par[3]*TMath::Exp(-(i - par[0])/par[4]);
  //}
    //convolved(x[0]) = ExpFn(x[0]-j)*theIRF(j)
    //conv += theIRF.at(x[0])*ExpFn;
    //if(HazRun==false){
  //cout << "i = " << i << ", ExpFn = " << ExpFn << ", conv = " << conv << endl;
  //}
  //} 

  HazRun = true;
  //Normalise?
  return conv;
}

#endif


void TestFunc(const char* fnameIRF, const char* fnameWbLS, const char* fnameMed,
	      int PMT_IRF, int PMT_WbLS, Double_t LB, Double_t UB,
	      Double_t par1, Double_t par2, Double_t par3, Double_t par4){
  
  //Instanciate a TimeFit class object.
  TimeFit* tf = new TimeFit();
  
  //Get IRF wfm
  TFile* f = new TFile(fnameIRF);
  TTree* IRFTree = (TTree*)f->Get("AvgSignal");
  TBranch* WfmBranch = (TBranch*)IRFTree->GetBranch("AvgShiftSigwCuts");
  
  vector< vector<double> > vecAllIRF = *(vector< vector<double> >*)
    (tf->GetPointerToValue(WfmBranch, 0, WfmBranch->GetName()));

  vector<double> theWfmIRF = vecAllIRF.at(PMT_IRF);

  //make the index
  vector<double> Idx;
  for(int i = 0; i<theWfmIRF.size(); i++){
    Idx.push_back(i);
  }

  TGraph* IRFGraph = new TGraph(static_cast<int>(theWfmIRF.size()),
			     &(Idx[0]), &(theWfmIRF[0]));
  //tf->SetIRF(theWfmIRF);
  //for(int i = 300; i<500; i++){
  //IRF.push_back(theWfmIRF.at(i));
  //}
  IRF = theWfmIRF;

  f->Close();

  //Get WbLS wfm
  f = new TFile(fnameWbLS);
  TTree* WbLSTree = (TTree*)f->Get("AvgSignal");

  TBranch* WbLSBranch = (TBranch*)WbLSTree->GetBranch("AvgShiftSigwCuts");

  vector< vector<double> > vecAllWbLS = *(vector< vector<double> >*)
     (tf->GetPointerToValue(WbLSBranch, 0, WbLSBranch->GetName()));

  vector<double> theWfmWbLS = vecAllWbLS.at(PMT_WbLS);

  //Create a TGraph with which to fit the convolved function.
  TGraph* WfmGraph = new TGraph(static_cast<int>(theWfmWbLS.size()),
				&(Idx[0]), &(theWfmWbLS[0]));

  f->Close();

  //Get num entries from median file.
  f = new TFile(fnameMed);
  TTree* MedTree = (TTree*)f->Get("MedianResults");
  ULong64_t nEntriesWbLS;
  string BrName = "NentriesBeam";//A or B, depending on PMT
  if(PMT_WbLS==0){
    BrName += "A";
    TBranch* MedBranch = (TBranch*)MedTree->GetBranch(BrName.c_str());
    nEntriesWbLS = *(ULong64_t*)
      (tf->GetPointerToValue(MedBranch, 0, MedBranch->GetName()));
  }
  else if(PMT_WbLS==1){
    BrName += "B";
    TBranch* MedBranch = (TBranch*)MedTree->GetBranch(BrName.c_str());
    nEntriesWbLS = *(ULong64_t*)
      (tf->GetPointerToValue(MedBranch, 0, MedBranch->GetName()));
  }
  else{
    cout << "ERROR: PMT for WbLS dataset must be either 0 or 1" << endl;
    return;
  }

  f->Close();

  vector<double> Difference;
  double sumIRF = 0;
  double sumWbLS = 0;
  vector<double> Idx2;
  vector<double> WbLSErrors;
  vector<double> IdxErrors;

  //Code to get the difference btw the datasets
  if(theWfmIRF.size()!=theWfmWbLS.size()){
    printf("ERROR: The IRF and WbLS waveforms are of different lengths");
  }
  else{
    //Get total number of counts in vectors
    for(int i = 300; i<500; i++){
      sumIRF += theWfmIRF.at(i);
      sumWbLS += theWfmWbLS.at(i);
    }
    //Now subtract
    for(int i = 300; i<500; i++){
      Difference.push_back((theWfmWbLS.at(i)/sumWbLS)-(theWfmIRF.at(i)/sumIRF));
      Idx2.push_back(i);
      //IRF.at(i) = IRF.at(i)/sumIRF;
    }
  }

  for(int i = 0; i<theWfmWbLS.size(); i++){
    //Fill errors vector for x and y
    //I'm giving the summed waveform the same relative error as an averaged
    //waveform (this should be correct)
    WbLSErrors.push_back(sqrt(abs(theWfmWbLS.at(i))));
    IdxErrors.push_back(0);
  }
  
  TGraph* DiffGraph = new TGraph(static_cast<int>(Difference.size()),
				 &(Idx2[0]), &(Difference[0]));

  TGraphErrors* WfmGraphErrs = 
    new TGraphErrors(static_cast<int>(theWfmWbLS.size()),
		     &(Idx[0]), &(theWfmWbLS[0]),
		     &(IdxErrors[0]), &(WbLSErrors[0]));
  
  //Plot this for testing.
  //WfmGraph->Draw("AP");
  
  WfmGraph->SetName(fnameWbLS);
  
  //DiffGraph->Draw("AP");

  //Now I want to fit this.

  //Some Parameters:
  Double_t FitParams[4];
  //Don't actually need this; it's constrained by the IRF.
  //FitParams[0] = LB;//336;//Exp function offset.
  FitParams[0] = par1;//0.75;//Short decay time amplitude
  FitParams[1] = par2;//5;//Short decay time const
  FitParams[2] = par3;//0.1;//Long decay time amplitude
  FitParams[3] = par4;//80;//Long decay time const
  lowbound = LB;
  upbound = UB;
  Double_t LowLimit[4];
  Double_t UpLimit[4];
  //LowLimit[0] = 335;
  //UpLimit[0] = 338;
  LowLimit[0] = 0;
  UpLimit[0] = 10;
  LowLimit[1] = 1;
  UpLimit[1] = 50;
  LowLimit[2] = 0;
  UpLimit[2] = 10;
  LowLimit[3] = 10;
  UpLimit[3] = 500;


  Char_t FunName[100];
  
  sprintf(FunName,"Fit_%s",WfmGraph->GetName());
  
  TF1 *ffitold = (TF1*)gROOT->GetListOfFunctions()->FindObject(FunName);
  if (ffitold) delete ffitold;

  TF1 *ffit = new TF1(FunName,TimeFit::ConvExp,lowbound,upbound,4);
  ffit->SetParameters(FitParams);
  ffit->SetParNames("ShortAmpl","ShortConst","LongAmpl", "LongConst");

  for (int i=0; i<5; i++) {
    ffit->SetParLimits(i, LowLimit[i], UpLimit[i]);
  }

  TFitResultPtr r = WfmGraph->Fit(FunName,"RBVS");   // fit within specified range, use ParLimits, do not plot

  //TCanvas c2;

  WfmGraph->SetMarkerSize(1.5);
  WfmGraph->SetMarkerColor(2);
  WfmGraph->Draw("AP");

  //IRFGraph->SetMarkerSize(1.5);
  //IRFGraph->SetMarkerColor(3);
  //IRFGraph->Draw("AP");

  ffit->SetLineColor(4);
  ffit->Draw("PSAME*");
  //ffit->Draw("C");

  //WfmGraph->SetMarkerSize(1.5);
  //WfmGraph->SetMarkerColor(2);
  //WfmGraph->Draw("P");

}
