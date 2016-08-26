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

const Int_t kNsamples = 2520;
//Assign parameters for algorithm in following format:
//{ch0, ch1, ch2, ch3}
static const double avg_baseline[4] = {8133, 8166, 8165, 8000};
static const double max_amplitude[4] = {13, 17, 50, 100};
static const int avg_pre_samples[4] = {4, 4, 4, 4};
static const int avg_post_samples[4] = {4, 4, 4, 4};
static const int trig_pre_samples[4] = {2, 2, 2, 10};
static const int trig_post_samples[4] = {10, 10, 15, 50};

class BaselineRemoval : public TSelector {
public :

  // Declaration of leaf types
  unsigned short fIdx[kNsamples];
  unsigned short fCh0[kNsamples];
  unsigned short fCh1[kNsamples];
  unsigned short fCh2[kNsamples];
  unsigned short fCh3[kNsamples];
  std::vector<std::vector<UShort_t> > vecChanVals;//replaces gChx arrays
  std::vector<UShort_t> vecColumn_UShort;

  unsigned short MaxCh3;

  std::vector<std::vector<ULong64_t> > vecCumSum;
  std::vector<ULong64_t> vecColumn_ULong;
  ULong64_t fCumSumCh0[kNsamples];
  ULong64_t fCumSumCh1[kNsamples];
  ULong64_t fCumSumCh2[kNsamples];
  ULong64_t fCumSumCh3[kNsamples];

  Long64_t nentries;
  Long64_t AcqNum, AcqNumInSpill;
  ULong64_t fTicksSinceADCon;

  //Baseline-subtraction algorithm outputs.
  //I use vectors to make working with them easier and the array for the actual
  //output to the TTree.
  std::vector<std::vector<double> > vecBaseline;
  std::vector<double> vecColumn_double;
  double BaselineCh0[kNsamples];
  double BaselineCh1[kNsamples];
  double BaselineCh2[kNsamples];
  double BaselineCh3[kNsamples];
  unsigned short Idx[kNsamples];

  double SignalCh0[kNsamples];
  double SignalCh1[kNsamples];
  double SignalCh2[kNsamples];
  double SignalCh3[kNsamples];

  
  bool IsSignal[kNsamples];
  //Output histograms from baseline-fitting algorithm
  //  TH1D* QDChisto;
  //TH1D* TimeAfterTrigHisto;
  //TH1D* PulseWidth;
   
  // List of branches
  TBranch     *Ch0Branch;
  TBranch     *Ch1Branch;
  TBranch     *Ch2Branch;
  TBranch* Ch3Branch;
  TBranch* IdxBranch;
  TTree* SourceTree;
  TTree* ResultTree;
  TFile* ChnFile; 
  TBranch     *CumSumCh0Branch;
  TBranch     *CumSumCh1Branch;
  TBranch     *CumSumCh2Branch;
  TBranch     *CumSumCh3Branch;
  TBranch     *TimeBranch;


   BaselineRemoval(TTree * = 0):
     nentries(0),AcqNum(0), AcqNumInSpill(0), MaxCh3(0)
  {

     printf("Calling BaselineRemoval Constructor...\n");

     //Allocate vector sizes:  all 4xkNsamples.
     vecColumn_UShort.resize(kNsamples,0);
     vecColumn_ULong.resize(kNsamples,0);
     vecColumn_double.resize(kNsamples,0.);
     vecChanVals.resize(4,vecColumn_UShort);
     vecCumSum.resize(4, vecColumn_ULong);
     vecBaseline.resize(4, vecColumn_double);
 
     for(int i = 0; i<kNsamples; i++){
       
       fCh0[i] = 0;
       fCh1[i] = 0;
       fCh2[i] = 0;
       fCh3[i] = 0;
       fIdx[i] = 0;
       fCumSumCh0[i] = 0;
       fCumSumCh1[i] = 0;
       fCumSumCh2[i] = 0;
       fCumSumCh3[i] = 0;
       //       if((i>49)&&(i<1051)){
	   
       //Idx[i] = i;
       BaselineCh0[i] = 0;
       BaselineCh1[i] = 0;
       BaselineCh2[i] = 0;
       BaselineCh3[i] = 0;
       SignalCh0[i] = 0;
       SignalCh1[i] = 0;
       SignalCh2[i] = 0;
       SignalCh3[i] = 0;
       IsSignal[i] = false;
	 //}
     }
  }

  virtual ~BaselineRemoval() {
    //Free up memory
    //delete fCh0, fCh1, fCh2, fCh3;
    //delete gCh0, gCh1, gCh2, gCh3;
    //delete fIdx, IsSignal;
    //     delete &vecChanVals, &vecBaseline, &vecSignal, &vecCumSum;
    //delete BaselineCh0, BaselineCh1, BaselineCh2, BaselineCh3;
    //delete CumSumCh0, CumSumCh1, CumSumCh2, CumSumCh3;
    //delete QDChisto, TimeAfterTrigHisto;
  }
  
  virtual void    Init(TTree *tree);
  virtual void    SlaveBegin(TTree *tree);
  virtual Bool_t  Process(Long64_t entry);
  virtual void    Terminate();
  virtual Int_t   Version() const { return 2; }
  void BLsub(int Chn, vector<UShort_t>& ChVals,
  	     vector<double>& BL, vector<ULong64_t>& CumSum);
  
  ClassDef(BaselineRemoval,0);
};

void BaselineRemoval::Init(TTree *tree)
{
  // The Init() function is called when the selector needs to initialize
  // a new tree or chain. Typically here the branch addresses and branch
  // pointers of the tree will be set.
  printf("Initialising BaselineRemoval!\n");
  nentries = tree->GetEntries();
  
  tree->SetMakeClass(1);
  tree->SetBranchAddress("Chn0", fCh0, &Ch0Branch);
  tree->SetBranchAddress("Chn1", fCh1, &Ch1Branch);
  tree->SetBranchAddress("Chn2", fCh2, &Ch2Branch);
  tree->SetBranchAddress("Chn3", fCh3, &Ch3Branch);
  tree->SetBranchAddress("Idx", fIdx, &IdxBranch);
  tree->SetBranchAddress("CumSumCh0", fCumSumCh0, &CumSumCh0Branch);
  tree->SetBranchAddress("CumSumCh1", fCumSumCh1, &CumSumCh1Branch);
  tree->SetBranchAddress("CumSumCh2", fCumSumCh2, &CumSumCh2Branch);
  tree->SetBranchAddress("CumSumCh3", fCumSumCh3, &CumSumCh3Branch);
  tree->SetBranchAddress("TicksSinceADCon", &fTicksSinceADCon,
			 &TimeBranch);

  SourceTree = tree;
  
}

void BaselineRemoval::SlaveBegin(TTree *tree)
{
   // SlaveBegin() is a good place to create histograms. 
   // For PROOF, this is called for each worker.
   // The TTree* is there for backward compatibility; e.g. PROOF passes 0.
   printf("Calling BaselineRemoval::SlaveBegin\n");

   TString option = GetOption();

   //Define the ResultTree
   ChnFile = new TFile("BLsubtracted.root", "RECREATE"); 
   ResultTree = new TTree("Results", "The algorithm processing results");
   //QDChisto = new TH1D("QDChisto",
   //       "The histogram of integrated pulses;Pulse Height",
   //       32768,0,16384);//# bins, lower edge of 1st bin, upper
   //                                 //edge of last bin.
   //TimeAfterTrigHisto = new TH1D("TimeAfterTrigHisto",
   //		 "Pulses time relative to wfm start",
   //		 (2520*4), 0, 2520);
   //PulseWidth = new TH1D("Pulse Width",
   //		 "Histogram of pulse widths",
   //		 2520,0,2520);
   //Long64_t FileNum = tree->GetChainOffset();

   ResultTree->Branch("Idx", &Idx, "Idx[2520]/s");
   ResultTree->Branch("AcqNum", &AcqNum, "AcqNum/L");
   ResultTree->Branch("AcqNumInSpill", &AcqNumInSpill, "AcqNumInSpill/L");
   ResultTree->Branch("BaselineCh0", &BaselineCh0, "NoBaselineCh0[2520]/D");
   ResultTree->Branch("BaselineCh1", &BaselineCh1, "NoBaselineCh1[2520]/D");
   ResultTree->Branch("BaselineCh2", &BaselineCh2, "NoBaselineCh2[2520]/D");
   ResultTree->Branch("BaselineCh3", &BaselineCh3, "NoBaselineCh3[2520]/D");
   ResultTree->Branch("SignalCh0", &SignalCh0, "SignalCh0[2520]/D");
   ResultTree->Branch("SignalCh1", &SignalCh1, "SignalCh1[2520]/D");
   ResultTree->Branch("SignalCh2", &SignalCh2, "SignalCh2[2520]/D");
   ResultTree->Branch("SignalCh3", &SignalCh3, "SignalCh3[2520]/D");
   //ResultTree->Branch("QDC", "TH1D", &QDChisto);
   //ResultTree->Branch("TimeHisto", "TH1D", &TimeAfterTrigHisto); 
   ResultTree->Branch("TicksSinceADCon",&fTicksSinceADCon,"TicksSinceADCon/l");
   //ResultTree->Branch("PulseWidth", "TH1D", &PulseWidth);
}

void BaselineRemoval::BLsub(int chn,
		    std::vector<UShort_t>& ChVals,
		    std::vector<double>& BL,
		    std::vector<ULong64_t>& CumSum){
  //Print some values for testing:
  //printf("InBLsub\n");
  //printf("avg_baseline[%i] = %f \n", chn, avg_baseline[chn]);
  //printf("max_amplitude[%i] = %f \n", chn, max_amplitude[chn]);
  //printf("avg_pre_samples[%i] = %i \n", chn, avg_pre_samples[chn]);
  //printf("Channel # = %i\n", chn);
  //printf("CumSum element 100 = %lu\n", CumSum.at(100));
  //printf("ChVals element 100 = %i\n", ChVals.at(100));
  bool hazTriggered = false;
  int k = 0;
  unsigned short ThisTrigStart, ThisTrigStop;
  int StartIdx = 0;
  int StopIdx = 0;

  std::vector<unsigned short> TrigStop;
  std::vector<unsigned short> TrigStart;

  //Initialise the baseline estimate parameter.
  ThisBL = avg_baseline[chn];
  if((chn==3)&&(MaxCh3<(avg_baseline[3]+200))){
    ThisBL = MaxCh3;
  }

  for(int j=(kNsamples-1); j>-1; j--){
    //printf("In Loop, j = %i. ", j);
    //Step backwards through array and get every trigger
    if((ChVals.at(j)<(avg_baseline[chn]-max_amplitude[chn]))&&
       (!hazTriggered)){
      //We have a trigger!
      TrigStop.push_back(j);
      hazTriggered = true;
    }
    //Now check if we're already in a trigger and value is sub-threshold
    if((ChVals.at(j)>(avg_baseline[chn]-max_amplitude[chn]))&&
       (hazTriggered)){
      TrigStart.push_back(j);
      hazTriggered = false;
    }    

    //printf("Size of ChVals = %i\n", ChVals.size());
    //printf("Size of CumSum = %i\n", CumSum.size());

    //Also calculate the moving average BL (without ROIs).
    k = kNsamples-1-j;
    if((k>(avg_pre_samples[chn]-1))&&
       (k<(kNsamples-1-avg_post_samples[chn]))){
      //process normally.
      BL.at(k) =
	(CumSum.at(k+avg_post_samples[chn])-
	 CumSum.at(k-avg_pre_samples[chn]))/
	((k+avg_post_samples[chn])-(k-avg_pre_samples[chn]));
    }
    else if(k<(kNsamples-1-avg_post_samples[chn])){
      //at beginning of wfm, calculate avg between
      //[0, avg_post_samples+k]
      BL.at(k) =
	(CumSum.at(k+avg_post_samples[chn])-CumSum.at(0))/
	(avg_post_samples[chn]+k);
    }
    else{
      //at end of wfm, calc avg btw [(k)-avg_post_samples, kNsamples-1]
      BL.at(k) =
	(CumSum.at(kNsamples-1)-
	 CumSum.at(k-avg_pre_samples[chn]))/
	((kNsamples-1)-(k-avg_pre_samples[chn]));
    }
    if(BL.at(k)>10000){
      //printf("Large BL! k = %i, CumSum.at(k+avg_post_samples[chn]) = %lu, CumSum.at(k-avg_pre_samples[chn]) = %lu, BL.at(k) = %f\n", k, CumSum.at(k+avg_post_samples[chn]), CumSum.at(k-avg_pre_samples[chn]), BL.at(k)); 
      printf("Large BL! k = %i, CumSum.at(k+avg_post_samples[chn]) = %lu BL.at(k) = %f\n", k, CumSum.at(k+avg_post_samples[chn]), BL.at(k)); 

    }
  }

  //Check if the loop left the trigger on, and add a trigger start at idx 0.
  if(hazTriggered){TrigStart.push_back(0);}

  //printf("Passed the big loop...\n");

  //Now I can just iterate over the triggered sections
  //std::vector<unsigned short>::reverse_iterator rit = TrigStart.rbegin();
  //if(!(TrigStart.empty())){
  if(TrigStart.size()!=TrigStop.size()){
    printf("ERROR: TrigStart is not same size as TrigStop!\n");
    printf("TrigStart.size() = %i, TrigStop.size() = %i \n",
	   TrigStart.size(), TrigStop.size());
    printf("TrigStop.at(0) = %i\n", TrigStop.at(0));
    printf("TrigStop.back() = %i\n", TrigStop.back());
  }
  else{//process normally
    //printf("TrigStart.size() = TrigStop.size() \n");
    while(!(TrigStart.empty())){
      ThisTrigStart = TrigStart.back();
      ThisTrigStop = TrigStop.back();
      TrigStart.pop_back();
      TrigStop.pop_back();
      
      StartIdx = ThisTrigStart-trig_pre_samples[chn];
      StopIdx = ThisTrigStop + trig_post_samples[chn];
      
      if((StartIdx-1) < 0){
	//Need to ensure the array isn't indexed out of bounds
	StartIdx = 1;
      }
      
      if((StopIdx+1) > (kNsamples-1)){
	//Need to ensure the array isn't indexed out of bounds
	StopIdx = (kNsamples-2);
      }
      
      for(int q=StartIdx;
	  q<StopIdx;
	  q++){
	//Adjust BL to be interpolated between ROI
	BL.at(q) = BL.at(StartIdx-1) + 
	  (BL.at(StopIdx+1) - BL.at(StartIdx-1))*
	  (q - (StartIdx - 1))/(StopIdx+1 - (StartIdx - 1));
      }
    }
  }
}


Bool_t BaselineRemoval::Process(Long64_t entry)
{
   // The Process() function is called for each entry in the tree to be 
   // processed. The entry argument specifies which entry in the currently
   // loaded tree is to be processed.

   // *** 1. *** Tell the tree to load the data for this entry:
   Ch0Branch->GetEntry(entry);
   Ch1Branch->GetEntry(entry);
   Ch2Branch->GetEntry(entry);
   Ch3Branch->GetEntry(entry);
   IdxBranch->GetEntry(entry);
   CumSumCh0Branch->GetEntry(entry);
   CumSumCh1Branch->GetEntry(entry);
   CumSumCh2Branch->GetEntry(entry);
   CumSumCh3Branch->GetEntry(entry);

   MaxCh3 = 0;
   
   //Fill the vectors for easier access
   for(int j=0;j<kNsamples;j++){
     vecChanVals.at(0).at(j) = fCh0[j];
     //printf("vecChanVals.at(0).at(j) = %f"
     vecChanVals.at(1).at(j) = fCh1[j];
     vecChanVals.at(2).at(j) = fCh2[j];
     vecChanVals.at(3).at(j) = fCh3[j];
     vecCumSum.at(0).at(j) = fCumSumCh0[j]-fCumSumCh0[50];
     vecCumSum.at(1).at(j) = fCumSumCh1[j]-fCumSumCh1[50];
     vecCumSum.at(2).at(j) = fCumSumCh2[j]-fCumSumCh2[50];
     vecCumSum.at(3).at(j) = fCumSumCh3[j]-fCumSumCh3[50];
     Idx[j] = j+40;

     //Get the starting estimate for Ch3's baseline (see notes).
     if(fCh3[j]>MaxCh3){MaxCh3 = fCh3[j];}

   }

   
   //AcqNum is now a unique identifier for each trace.
   AcqNum = entry + SourceTree->GetChainOffset();
   AcqNumInSpill = entry;


   // *** 2. *** Do the actual analysis
   //printf("Entry # = %lu\n", entry);
   for(int i=0; i<4; i++){
     //printf("Calling BLsub, channel %i... ", i);
     //Call the pulse processing algorithm for each channel
     //printf("Prior to call, BL.at(10) = %f\n", vecBaseline.at(i).at(10));
     BLsub(i, vecChanVals.at(i), vecBaseline.at(i), vecCumSum.at(i));
     //printf("After call, BL.at(10) = %f\n", vecBaseline.at(i).at(10));
     //printf("Success!\n");
   }


   ///////////////////////////////////////////////////////////////////
   //Need to add a section to fill output arrays with vector values.//
   ///////////////////////////////////////////////////////////////////
   for(int i = 0; i<(kNsamples-1); i++){
     //Fill output arrays.
     BaselineCh0[i] = vecBaseline.at(0).at(i);
     BaselineCh1[i] = vecBaseline.at(1).at(i);
     BaselineCh2[i] = vecBaseline.at(2).at(i);
     BaselineCh3[i] = vecBaseline.at(3).at(i);
     SignalCh0[i] = -(vecChanVals.at(0).at(i) - vecBaseline.at(0).at(i));
     SignalCh1[i] = -(vecChanVals.at(1).at(i) - vecBaseline.at(1).at(i));
     SignalCh2[i] = -(vecChanVals.at(2).at(i) - vecBaseline.at(2).at(i));
     SignalCh3[i] = -(vecChanVals.at(3).at(i) - vecBaseline.at(3).at(i));
   }
   //printf("Output arrays filled\n");

   ResultTree->Fill();
   
   return kTRUE;
}

void BaselineRemoval::Terminate()
{
   // The Terminate() function is the last function to be called during the
   // analysis of a tree with a selector. It always runs on the client, it can
   // be used to present the results graphically or save the results to file.

  ResultTree->Write();
     //Free up memory
     //delete fCh0, fCh1, fCh2, fCh3;
     //delete gCh0, gCh1, gCh2, gCh3;
     //delete fIdx, IsSignal;
     //delete &vecChanVals, &vecBaseline, &vecSignal, &vecCumSum;
     //delete BaselineCh0, BaselineCh1, BaselineCh2, BaselineCh3;
     //delete CumSumCh0, CumSumCh1, CumSumCh2, CumSumCh3;
     //delete QDChisto, TimeAfterTrigHisto;

}
