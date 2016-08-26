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

#include <vector>

#include "TROOT.h"
#include "TChain.h"
#include "TFile.h"
#include "TSelector.h"
#include "TBranch.h"
#include "TString.h"
#include "TH1D.h"

using namespace std;

const Int_t kNsamples = 2520;


class DimaBaselineRemoval : public TSelector {
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

  Long64_t nentries;
  Long64_t AcqNum, AcqNumInSpill;
  ULong64_t fTicksSinceADCon;

  //Baseline-subtraction algorithm outputs.
  unsigned short Idx[kNsamples];


  std::vector<UShort_t> BLOKidx;
  
  //Pulse parameters
  std::vector<double> ColDouble;
  std::vector<double> QDCpulse;
  std::vector<double> QDCLED;
  std::vector<double> PulseHeight;
  std::vector<double> LEDPulseHeight;
  std::vector<double> PulseMinTime;
  std::vector<double> LEDPulseMinTime;
  std::vector<double> PreTrigAvg;
  std::vector<double> PostTrigAvg;
  std::vector<double> PreLEDAvg;
  std::vector<double> PostLEDAvg;

  // List of branches
  TBranch     *Ch0Branch;
  TBranch     *Ch1Branch;
  TBranch     *Ch2Branch;
  TBranch* Ch3Branch;
  TBranch* IdxBranch;
  TBranch* AcqNumInSpillBranch;
  TBranch* AcqNumBranch;
  TTree* SourceTree;
  TTree* ResultTree;
  TFile* ChnFile; 
  TBranch     *CumSumCh0Branch;
  TBranch     *CumSumCh1Branch;
  TBranch     *CumSumCh2Branch;
  TBranch     *CumSumCh3Branch;
  TBranch     *TimeBranch;


   DimaBaselineRemoval(TTree * = 0):
     nentries(0),AcqNum(0), AcqNumInSpill(0), MaxCh3(0)
  {

     printf("Calling DimaBaselineRemoval Constructor...\n");

     //Allocate vector sizes:  all 4xkNsamples.
     vecColumn_UShort.resize(kNsamples,0);
     vecChanVals.resize(4,vecColumn_UShort);

     for(int i = 0; i<4; i++){
       QDCpulse.push_back(0.);
       QDCLED.push_back(0.);
       PulseHeight.push_back(0.);
       LEDPulseHeight.push_back(0.);
       PulseMinTime.push_back(0.);
       LEDPulseMinTime.push_back(0.);
       PreTrigAvg.push_back(0.);
       PostTrigAvg.push_back(0.);
       PreLEDAvg.push_back(0.);
       PostLEDAvg.push_back(0.);
     }
     
     for(int i = 0; i<kNsamples; i++){
       fCh0[i] = 0;
       fCh1[i] = 0;
       fCh2[i] = 0;
       fCh3[i] = 0;
       fIdx[i] = 0;
     }
  }

  virtual ~DimaBaselineRemoval() {
    printf("Calling DimaBaselineRemoval destructor... \n");
  }
  virtual void    Init(TTree *tree);
  virtual void    SlaveBegin(TTree *tree);
  virtual Bool_t  Process(Long64_t entry);
  virtual void    Terminate();
  virtual Int_t   Version() const { return 2; }
 
  ClassDef(DimaBaselineRemoval,0);
};

void DimaBaselineRemoval::Init(TTree *tree)
{
  // The Init() function is called when the selector needs to initialize
  // a new tree or chain. Typically here the branch addresses and branch
  // pointers of the tree will be set.
  printf("Initialising DimaBaselineRemoval!\n");
  nentries = tree->GetEntries();
  printf("# of entries = %i\n", nentries);
  
  tree->SetMakeClass(1);
  tree->SetBranchAddress("Chn0", fCh0, &Ch0Branch);
  tree->SetBranchAddress("Chn1", fCh1, &Ch1Branch);
  tree->SetBranchAddress("Chn2", fCh2, &Ch2Branch);
  tree->SetBranchAddress("Chn3", fCh3, &Ch3Branch);
  tree->SetBranchAddress("Idx", fIdx, &IdxBranch);
  tree->SetBranchAddress("TicksSinceADCon", &fTicksSinceADCon,
			 &TimeBranch);
  tree->SetBranchAddress("AcqNumInSpill", &AcqNumInSpill, &AcqNumInSpillBranch);
  tree->SetBranchAddress("AcqNum", &AcqNum, &AcqNumBranch);

  SourceTree = tree;
  
}

void DimaBaselineRemoval::SlaveBegin(TTree *tree)
{
   // SlaveBegin() is a good place to create histograms. 
   // For PROOF, this is called for each worker.
   // The TTree* is there for backward compatibility; e.g. PROOF passes 0.
   printf("Calling DimaBaselineRemoval::SlaveBegin\n");

   TString option = GetOption();

   //Define the ResultTree
   ChnFile = new TFile("DimaBLsub.root", "RECREATE");
   ResultTree = new TTree("Results", "The algorithm processing results");
   
   ResultTree->Branch("Idx", &Idx, "Idx[2520]/s");
   ResultTree->Branch("AcqNum", &AcqNum, "AcqNum/L");
   ResultTree->Branch("AcqNumInSpill", &AcqNumInSpill, "AcqNumInSpill/L");
   ResultTree->Branch("PreTrigAvg", "vector<double>", &PreTrigAvg);
   ResultTree->Branch("PostTrigAvg", "vector<double>", &PostTrigAvg);
   ResultTree->Branch("PreLEDAvg", "vector<double>", &PreLEDAvg);
   ResultTree->Branch("PostLEDAvg", "vector<double>", &PostLEDAvg);
   ResultTree->Branch("QDCpulse", "vector<double>", &QDCpulse);
   ResultTree->Branch("QDCLED", "vector<double>", &QDCLED);
   ResultTree->Branch("PulseHeight", "vector<double>", &PulseHeight);
   ResultTree->Branch("LEDPulseHeight", "vector<double>", &LEDPulseHeight);
   ResultTree->Branch("PulseMinTime", "vector<double>", &PulseMinTime);
   ResultTree->Branch("LEDPulseMinTime", "vector<double>", &LEDPulseMinTime);
   ResultTree->Branch("MaxCh3", &MaxCh3, "MaxCh3/s");

}


Bool_t DimaBaselineRemoval::Process(Long64_t entry)
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
   AcqNumInSpillBranch->GetEntry(entry);
   AcqNumBranch->GetEntry(entry);

   MaxCh3 = 0;
   
   //Fill the vectors for easier access
   for(int i=0;i<4;i++){
     PreTrigAvg.at(i) = 0.;
     PostTrigAvg.at(i) = 0.;
     PreLEDAvg.at(i) = 0.;
     PostLEDAvg.at(i) = 0.;
     QDCpulse.at(i) = 0;
     QDCLED.at(i) = 0;
     PulseHeight.at(i) = 16384;
     LEDPulseHeight.at(i) = 16384;
     PulseMinTime.at(i) = 0;
     LEDPulseMinTime.at(i) = 0;
   }


   //printf("Processing file, event # = %i...  ", entry);
   for(int j=0;j<kNsamples;j++){

     vecChanVals.at(0).at(j) = fCh0[j];
     vecChanVals.at(1).at(j) = fCh1[j];
     vecChanVals.at(2).at(j) = fCh2[j];
     vecChanVals.at(3).at(j) = fCh3[j];
     Idx[j] = j+40;//1st 40 values weren't used
     
     //Get the starting estimate for Ch3's baseline (see notes).
     if(fCh3[j]>MaxCh3){MaxCh3 = fCh3[j];}
     
     //Now I can get the QDC, Pulse Height, trigger time for each channel.
     for(int i = 0; i<4; i++){
	 if((299<j)&&(j<350)){
	   //Pre-trig avg
	   PreTrigAvg.at(i) += vecChanVals.at(i).at(j);
	 }
	 else if((349<j)&&(j<550)){
	   //Grab the integral
	   QDCpulse.at(i) += vecChanVals.at(i).at(j);
	   if(vecChanVals.at(i).at(j)<PulseHeight.at(i)){
	     PulseHeight.at(i) = vecChanVals.at(i).at(j);
	     PulseMinTime.at(i) = j;
	   }
	 }
	 else if((549<j)&&(j<600)){
	   //Post-trig avg
	   PostTrigAvg.at(i) += vecChanVals.at(i).at(j);
	 }
	 else if((949<j)&&(j<1000)){
	   //Pre-LED avg
	   PreLEDAvg.at(i) += vecChanVals.at(i).at(j);
	 }
	 else if((999<j)&&(j<1200)){
	   //Grab the integral
	   QDCLED.at(i) += vecChanVals.at(i).at(j);
	   if(vecChanVals.at(i).at(j)<LEDPulseHeight.at(i)){
	     LEDPulseHeight.at(i) = vecChanVals.at(i).at(j);
	     LEDPulseMinTime.at(i) = j;
	   }
	 }
	 else if((1199<j)&&(j<1250)){
	   //Post-LED avg
	   PostLEDAvg.at(i) += vecChanVals.at(i).at(j);
	 }
       }
   }

   //Work out the correct averages
   for(int i = 0; i<4; i++){
     PreTrigAvg.at(i) = PreTrigAvg.at(i)/50;
     PostTrigAvg.at(i) = PostTrigAvg.at(i)/50;
     PreLEDAvg.at(i) = PreLEDAvg.at(i)/50;
     PostLEDAvg.at(i) = PostLEDAvg.at(i)/50;
     QDCpulse.at(i) = -((QDCpulse.at(i)) -
			((PreTrigAvg.at(i)+PostTrigAvg.at(i))/2)*200);
     QDCLED.at(i) = -((QDCLED.at(i)) -
		      ((PreLEDAvg.at(i)+PostLEDAvg.at(i))/2)*200);
     PulseHeight.at(i) = -(PulseHeight.at(i) -
			   (PreTrigAvg.at(i)+PostTrigAvg.at(i))/2);
     LEDPulseHeight.at(i) = -(LEDPulseHeight.at(i) -
			      (PreLEDAvg.at(i)+PostLEDAvg.at(i))/2);
   }
   

   ResultTree->Fill();
   
   
   return kTRUE;
}

void DimaBaselineRemoval::Terminate()
{
   // The Terminate() function is the last function to be called during the
   // analysis of a tree with a selector. It always runs on the client, it can
   // be used to present the results graphically or save the results to file.

  ResultTree->Write();
     //Free up memory
     //delete fCh0, fCh1, fCh2, fCh3;
     //delete gCh0, gCh1, gCh2, gCh3;
     //delete fIdx;
     //delete &vecChanVals, &vecBaseline, &vecSignal, &vecCumSum;
     //delete BaselineCh0, BaselineCh1, BaselineCh2, BaselineCh3;
     //delete CumSumCh0, CumSumCh1, CumSumCh2, CumSumCh3;
     //delete QDChisto, TimeAfterTrigHisto;

}
