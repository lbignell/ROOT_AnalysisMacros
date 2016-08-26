// For more information on the TSelector framework see 
// $ROOTSYS/README/README.SELECTOR or the ROOT User Manual.

// The file for this selector can be found at
// http://lcg-heppkg.web.cern.ch/lcg-heppkg/ROOT/eventdata.root
// i.e run
//   root [0] f = TFile::Open("http://lcg-heppkg.web.cern.ch/lcg-heppkg/ROOT/eventdata.root");
//   root [1] EventTree->Process("EventSelector.C+")

// The following methods are defined in this file:
//    SlaveBegin():   called after Begin(), when on PROOF called only on the
//                    slave servers, a convenient place to create your histograms.
//    Process():      called for each event, in this function you decide what
//                    to read and fill your histograms.
//    Terminate():    called at the end of the loop on the tree,
//                    a convenient place to draw/fit your histograms.
//
// To use this file, try the following session on your Tree T:
//
// Root > T->Process("EventSelector.C")
// Root > T->Process("EventSelector.C","some options")
// Root > T->Process("EventSelector.C+")
//

//This file can be edited (it's currently not used for anything and should make
//a nice template).

//This sillyness is a workaround for cint's sillyness.
//Once I get this paper written I think I'll switch to pyroot...
#include <vector>
//typedef vector<int> vector_int;
//#include <vector>

#include "TROOT.h"
#include "TChain.h"
#include "TFile.h"
#include "TSelector.h"
#include "TBranch.h"
#include "TString.h"
#include "TH1D.h"

using namespace std;

const Int_t kNsamples = 2560;
//Assign parameters for algorithm in following format:
//{ch0, ch1, ch2, ch3}
static const double arr_avg_baseline[4] = {8133, 8166, 8165, 8137};
static const double arr_max_amplitude[4] = {13, 17, 50, 36};
static const int arr_avg_pre_samples[4] = {4, 4, 4, 4};
static const int arr_avg_post_samples[4] = {4, 4, 4, 4};
static const int arr_trig_pre_samples[4] = {5, 5, 5, 5};
static const int arr_trig_post_samples[4] = {5, 5, 5, 5};

class SimpleSumIntegralv2 : public TSelector {
public :

  // Declaration of leaf types
  unsigned short fCh0[kNsamples];   //Initialise to 0
  unsigned short fCh1[kNsamples];   //Initialise to 0
  unsigned short fCh2[kNsamples];
  unsigned short fCh3[kNsamples];
  unsigned short fSampleNo[kNsamples-40];
  unsigned short gCh0[kNsamples-40];
  unsigned short gCh1[kNsamples-40];
  unsigned short gCh2[kNsamples-40];
  unsigned short gCh3[kNsamples-40];
  std::vector<std::vector<unsigned short> > vecChanVals;//replaces gChx arrays
  std::vector<unsigned short> vecColumn_UShort;
  Long64_t nentries;
  Long64_t AcqNum, AcqNumInSpill;
  ULong64_t fTicksSinceADCon;
  unsigned short MinValue0, MinValue1, MinValue2, MinValue3;
  unsigned short MinValIdx0, MinValIdx1, MinValIdx2, MinValIdx3;

  //Baseline-subtraction algorithm outputs.
  //I use vectors to make working with them easier and the array for the actual
  //output to the TTree.
  //std::vector<std::vector<double> > vecBaseline;
  //std::vector<double> vecColumn_double;
  double BaselineCh0[kNsamples-40];
  double BaselineCh1[kNsamples-40];
  double BaselineCh2[kNsamples-40];
  double BaselineCh3[kNsamples-40];

  //std::vector<std::vector<double> > vecSignal;
  double SignalCh0[kNsamples-40];
  double SignalCh1[kNsamples-40];
  double SignalCh2[kNsamples-40];
  double SignalCh3[kNsamples-40];

  //std::vector<std::vector<unsigned long> > vecCumSum;
  //std::vector<unsigned long> vecColumn_ULong;
  ULong64_t CumSumCh0[kNsamples-40];
  ULong64_t CumSumCh1[kNsamples-40];
  ULong64_t CumSumCh2[kNsamples-40];
  ULong64_t CumSumCh3[kNsamples-40];
  
  bool IsSignal[kNsamples-40];
  //Output histograms from baseline-fitting algorithm
  TH1D* QDChisto;
  TH1D* TimeAfterTrigHisto;
  TH1D* PulseWidth;
   
  // List of branches
  TBranch     *Ch0Branch;
  TBranch     *Ch1Branch;
  TBranch     *Ch2Branch;
  TBranch* Ch3Branch;
  TBranch* TimeBranch;
  TTree* SourceTree;
  TTree* ChnValTree;
  TFile* ChnFile; 

  TTree* HeaderTree;
  TTree* FooterTree;
  TChain* HeaderChain;
  TChain* FooterChain;

   SimpleSumIntegralv2(TTree * = 0):
     nentries(0),AcqNum(0), AcqNumInSpill(0),
     MinValue0(0),MinValue1(0),MinValue2(0),MinValue3(0),
     MinValIdx0(0),MinValIdx1(0),MinValIdx2(0),MinValIdx3(0)
  {

     printf("Calling SimpleSumIntegralv2 Constructor...\n");
     for(int j = 0; j<4; j++){
       vecChanVals.push_back(vecColumn_UShort);
       //vecBaseline.push_back(vecColumn_double);
       //vecSignal.push_back(vecColumn_double);
       //vecCumSum.push_back(vecColumn_ULong);

       for(int i = 0; i<kNsamples; i++){
	 fCh0[i] = 0;
	 fCh1[i] = 0;
	 fCh2[i] = 0;
	 fCh3[i] = 0;
	 if(i>39){
	   vecChanVals.at(j).push_back(0);
	   fSampleNo[i-40] = i;
	   gCh0[i-40] = 0;
	   gCh1[i-40] = 0;
	   gCh2[i-40] = 0;
	   gCh3[i-40] = 0;
	   //vecBaseline.at(j).push_back(0);
	   BaselineCh0[i-40] = 0;
	   BaselineCh1[i-40] = 0;
	   BaselineCh2[i-40] = 0;
	   BaselineCh3[i-40] = 0;
	   //vecSignal.at(j).push_back(0);
	   SignalCh0[i-40] = 0;
	   SignalCh1[i-40] = 0;
	   SignalCh2[i-40] = 0;
	   SignalCh3[i-40] = 0;
	   //vecCumSum.at(j).push_back(0);
	   CumSumCh0[i-40] = 0;
	   CumSumCh1[i-40] = 0;
	   CumSumCh2[i-40] = 0;
	   CumSumCh3[i-40] = 0;
	   IsSignal[i-40] = false;
	 }
       }
     }
  }

   virtual ~SimpleSumIntegralv2() {
     //Free up memory
   }

   virtual void    Init(TTree *tree);
   virtual void    SlaveBegin(TTree *tree);
   virtual Bool_t  Process(Long64_t entry);
   virtual void    Terminate();
   virtual Int_t   Version() const { return 2; }
  
   ClassDef(SimpleSumIntegralv2,0);
};

void SimpleSumIntegralv2::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
  printf("Initialising SimpleSumIntegralv2!\n");
  nentries = tree->GetEntries();

  //tree->Print();
  tree->SetMakeClass(1);
  tree->SetBranchAddress("Channel0", fCh0, &Ch0Branch);
  tree->SetBranchAddress("Channel1", fCh1, &Ch1Branch);
  tree->SetBranchAddress("Channel2", fCh2, &Ch2Branch);
  tree->SetBranchAddress("Channel3", fCh3, &Ch3Branch);
  tree->SetBranchAddress("TriggerCountFromRunStart", &fTicksSinceADCon,
			 &TimeBranch);

  SourceTree = tree;

}

void SimpleSumIntegralv2::SlaveBegin(TTree *tree)
{
   // SlaveBegin() is a good place to create histograms. 
   // For PROOF, this is called for each worker.
   // The TTree* is there for backward compatibility; e.g. PROOF passes 0.
   printf("Calling SimpleSumIntegralv2::SlaveBegin\n");

   TString option = GetOption();

   //Define the ChnValTree
   ChnFile = new TFile("SimpleIntegral.root", "RECREATE"); 
   ChnValTree = new TTree("Results", "The algorithm processing results");
		       
   ChnValTree->Branch("Chn0", &gCh0, "Ch0[2520]/s");
   ChnValTree->Branch("Chn1", &gCh1, "Ch1[2520]/s");
   ChnValTree->Branch("Chn2", &gCh2, "Ch2[2520]/s");
   ChnValTree->Branch("Chn3", &gCh3, "Ch3[2520]/s");
   ChnValTree->Branch("Idx", &fSampleNo, "Idx[2520]/s");
   ChnValTree->Branch("CumSumCh0", &CumSumCh0, "CumSumCh0[2520]/l");
   ChnValTree->Branch("CumSumCh1", &CumSumCh1, "CumSumCh1[2520]/l");
   ChnValTree->Branch("CumSumCh2", &CumSumCh2, "CumSumCh2[2520]/l");
   ChnValTree->Branch("CumSumCh3", &CumSumCh3, "CumSumCh3[2520]/l");
   ChnValTree->Branch("AcqNum", &AcqNum, "AcqNum/L");
   ChnValTree->Branch("AcqNumInSpill", &AcqNumInSpill, "AcqNumInSpill/L");
   ChnValTree->Branch("MinVal0", &MinValue0, "MinVal0/s");
   ChnValTree->Branch("MinVal1", &MinValue1, "MinVal1/s");
   ChnValTree->Branch("MinVal2", &MinValue2, "MinVal2/s");
   ChnValTree->Branch("MinVal3", &MinValue3, "MinVal3/s");
   ChnValTree->Branch("MinValIdx0", &MinValIdx0, "MinValIdx0/s");
   ChnValTree->Branch("MinValIdx1", &MinValIdx1, "MinValIdx1/s");
   ChnValTree->Branch("MinValIdx2", &MinValIdx2, "MinValIdx2/s");
   ChnValTree->Branch("MinValIdx3", &MinValIdx3, "MinValIdx3/s");
   ChnValTree->Branch("BaselineCh0", &BaselineCh0, "NoBaselineCh0[2520]/D");
   ChnValTree->Branch("BaselineCh1", &BaselineCh1, "NoBaselineCh1[2520]/D");
   ChnValTree->Branch("BaselineCh2", &BaselineCh2, "NoBaselineCh2[2520]/D");
   ChnValTree->Branch("BaselineCh3", &BaselineCh3, "NoBaselineCh3[2520]/D");
   ChnValTree->Branch("TicksSinceADCon",&fTicksSinceADCon, "TicksSinceADCon/l");
}

Bool_t SimpleSumIntegralv2::Process(Long64_t entry)
{
   // The Process() function is called for each entry in the tree to be 
   // processed. The entry argument specifies which entry in the currently
   // loaded tree is to be processed.
   // It can be passed to either SimpleSumIntegralv2::GetEntry() or
   // TBranch::GetEntry() to read either all or the required parts of the TTree.
   //
   // This function should contain the "body" of the analysis: select relevant
   // tree entries, run algorithms on the tree entry and typically fill
   // histograms.


   // *** 1. *** Tell the tree to load the data for this entry:
   Ch0Branch->GetEntry(entry);
   Ch1Branch->GetEntry(entry);
   Ch2Branch->GetEntry(entry);
   Ch3Branch->GetEntry(entry);
   TimeBranch->GetEntry(entry);

   MinValue0 = 16383;//14 bit resolution
   MinValue1 = 16383;
   MinValue2 = 16383;
   MinValue3 = 16383;

   bool HazTriggered = false;
   double PulseIntegral = 0;
   int StartChannel = 0;

   for(int j=40;j<kNsamples;j++){
     gCh0[j-40]=fCh0[j];
     gCh1[j-40]=fCh1[j];
     gCh2[j-40]=fCh2[j];
     gCh3[j-40]=fCh3[j];
     vecChanVals.at(0)[j-40] = fCh0[j];
     vecChanVals.at(1)[j-40] = fCh1[j];
     vecChanVals.at(2)[j-40] = fCh2[j];
     vecChanVals.at(3)[j-40] = fCh3[j];
     fSampleNo[j-40] = j;

     //Subtract simple baseline.
     BaselineCh0[j-40] = -(fCh0[j] - arr_avg_baseline[0]);
     BaselineCh1[j-40] = -(fCh1[j] - arr_avg_baseline[1]);
     BaselineCh2[j-40] = -(fCh2[j] - arr_avg_baseline[2]);
     BaselineCh3[j-40] = -(fCh3[j] - arr_avg_baseline[3]);

     //Calculate the minimum value in the waveform.
     if(fCh0[j]<MinValue0) {
       MinValue0=fCh0[j];
       MinValIdx0 = j;
     }
     if(fCh1[j]<MinValue1) {
       MinValue1=fCh1[j];
       MinValIdx1 = j;
     }
     if(fCh2[j]<MinValue2) {
       MinValue2=fCh2[j];
       MinValIdx2 = j;
     }
     if(fCh3[j]<MinValue3) {
       MinValue3=fCh3[j];
       MinValIdx3 = j;
     }

     //Fill the CumSum arrays
     if(j==40){
       CumSumCh0[j-40] = fCh0[j];
       CumSumCh1[j-40] = fCh1[j];
       CumSumCh2[j-40] = fCh2[j];
       CumSumCh3[j-40] = fCh3[j];
       //vecCumSum.at(0).push_back(fCh0
     }
     else{
       CumSumCh0[j-40] = fCh0[j]+CumSumCh0[j-40-1];
       CumSumCh1[j-40] = fCh1[j]+CumSumCh1[j-40-1];
       CumSumCh2[j-40] = fCh2[j]+CumSumCh2[j-40-1];
       CumSumCh3[j-40] = fCh3[j]+CumSumCh3[j-40-1];
     }

   }
   
   //AcqNum is now a unique identifier for each trace.
   AcqNum = entry + SourceTree->GetChainOffset();
   AcqNumInSpill = entry;


   // *** 2. *** Do the actual analysis
   
   ChnValTree->Fill();
   
   return kTRUE;
}

void SimpleSumIntegralv2::Terminate()
{
   // The Terminate() function is the last function to be called during the
   // analysis of a tree with a selector. It always runs on the client, it can
   // be used to present the results graphically or save the results to file.

  //TTree* 
    
    ChnValTree->Write();

     //Free up memory
     //delete fCh0, fCh1, fCh2, fCh3;
     //delete gCh0, gCh1, gCh2, gCh3;
     //delete fSampleNo, IsSignal;
     //delete &vecChanVals, &vecBaseline, &vecSignal, &vecCumSum;
     //delete BaselineCh0, BaselineCh1, BaselineCh2, BaselineCh3;
     //delete CumSumCh0, CumSumCh1, CumSumCh2, CumSumCh3;
     //delete QDChisto, TimeAfterTrigHisto;

}

