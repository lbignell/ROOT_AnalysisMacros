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
#include <fstream>
#include <iostream>

using namespace std;

const Int_t kNsamples = 2520;


///////////////////////////////////////////////////////////////////////////////
//Assign parameters for algorithm in following format:
//{ch0, ch1, ch2, ch3}
//Note that good results require:
// trig_pre_samples > avg_post_samples
// and
// trig_post_samples > avg_pre_samples
//(otherwise the pulse could be used in the averaging).

//There is another condition for pulse pile-up that:
// UnTrigSettleLength > trig_post_samples + trig_pre_samples
//If this is not done, the next pulse may interfere in the baseline calculation
///////////////////////////////////////////////////////////////////////////////
//Original Values
//static const double avg_baseline[4] = {8133, 8166, 8165, 8000};
//static const double max_amplitude[4] = {13, 17, 50, 100};
//static const int avg_pre_samples[4] = {4, 4, 4, 4};
//static const int avg_post_samples[4] = {4, 4, 4, 4};
//static const int trig_pre_samples[4] = {10, 10, 10, 10};
//static const int trig_post_samples[4] = {10, 10, 10, 60};
//static const int UnTrigSettleLength[4] = {21, 21, 21, 71};
///////////////////////////////////////////////////////////////////////////////
//Alternate values - longer fall-time
//static const double avg_baseline[4] = {8133, 8166, 8165, 8000};
//static const double max_amplitude[4] = {20, 20, 50, 100};
//static const int avg_pre_samples[4] = {4, 4, 4, 4};
//static const int avg_post_samples[4] = {4, 4, 4, 4};
//static const int trig_pre_samples[4] = {10, 10, 10, 10};
//static const int trig_post_samples[4] = {30, 30, 30, 90};
//static const int UnTrigSettleLength[4] = {41, 41, 41, 101};
///////////////////////////////////////////////////////////////////////////////

class BaselineRemovalv5 : public TSelector {
public :

  double avg_baseline[4];// = {8133, 8166, 8165, 8000};
  double max_amplitude[4];// = {13, 17, 50, 100};
  int avg_pre_samples[4];// = {4, 4, 4, 4};
  int avg_post_samples[4];// = {4, 4, 4, 4};
  int trig_pre_samples[4];// = {10, 10, 10, 10};
  int trig_post_samples[4];// = {10, 10, 10, 60};
  int UnTrigSettleLength[4];// = {21, 21, 21, 71};
  

  // Declaration of leaf types
  unsigned short fIdx[kNsamples];
  unsigned short fCh0[kNsamples];
  unsigned short fCh1[kNsamples];
  unsigned short fCh2[kNsamples];
  unsigned short fCh3[kNsamples];
  std::vector<std::vector<UShort_t> > vecChanVals;//replaces gChx arrays
  std::vector<UShort_t> vecColumn_UShort;

  unsigned short MaxCh3;
  unsigned short BLisOK_ch0, BLisOK_ch1, BLisOK_ch2, BLisOK_ch3;

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
  std::vector<std::vector<double> > vecSignal;
  std::vector<std::vector<double> > vecBaseline;
  std::vector<std::vector<double> > vecAvgSignal;
  std::vector<std::vector<double> > vecAvgSignalwCuts;
  std::vector<std::vector<double> > vecAvgShiftSigwCuts;
  std::vector<double> vecColumn_double;
  unsigned short Idx[kNsamples];


  std::vector<UShort_t> BLOKidx;
  
  //Pulse parameters
  std::vector<std::vector<double> > QDCsimple;
  std::vector<std::vector<double> > TimeSimple;
  std::vector<std::vector<double> > PulseWidthSimple;
  std::vector<double> ColDouble;
  std::vector< std::vector<double> > InterpDiff;
  std::vector< std::vector<double> > PulseHeight;
  //TH1D* QDChisto;
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
  TTree* AvgSigTree;
  TFile* ChnFile; 
  TFile* AvgFile; 
  TBranch     *CumSumCh0Branch;
  TBranch     *CumSumCh1Branch;
  TBranch     *CumSumCh2Branch;
  TBranch     *CumSumCh3Branch;
  TBranch     *TimeBranch;


   BaselineRemovalv5(TTree * = 0):
     nentries(0),AcqNum(0), AcqNumInSpill(0), MaxCh3(0)
  {

     printf("Calling BaselineRemovalv5 Constructor...\n");

     //string thisBL;
     //string thisampl;
     //string thisavg_pre;
     //string thisavg_post;
     //string thistrig_pre;
     //string thistrig_post;
     // string thisUTrigLen;

     fstream file;
     file.open("params.txt", ios_base::in | ios_base::out | ios_base::app);

     //Fill the algorithm parameter arrays
     for(int i = 0; i<4; i++){
       //Read the next line in to the variables.       
       file >> avg_baseline[i] >> max_amplitude[i] >> avg_pre_samples[i]
	    >> avg_post_samples[i] >> trig_pre_samples[i]
	    >> trig_post_samples[i] >> UnTrigSettleLength[i];
     }
     

     //Allocate vector sizes:  all 4xkNsamples.
     vecColumn_UShort.resize(kNsamples,0);
     vecColumn_ULong.resize(kNsamples,0);
     vecColumn_double.resize(kNsamples,0.);
     vecChanVals.resize(4,vecColumn_UShort);
     vecCumSum.resize(4, vecColumn_ULong);
     vecBaseline.resize(4, vecColumn_double);
     vecSignal.resize(4, vecColumn_double);
     vecAvgSignal.resize(4, vecColumn_double);
     vecAvgSignalwCuts.resize(4, vecColumn_double);
     vecAvgShiftSigwCuts.resize(4, vecColumn_double);
     BLOKidx.resize(4, 0);
     
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
     }
  }

  virtual ~BaselineRemovalv5() {
    printf("Calling BaselineRemovalv5 destructor... \n");
  }
  virtual void    Init(TTree *tree);
  virtual void    SlaveBegin(TTree *tree);
  virtual Bool_t  Process(Long64_t entry);
  virtual void    Terminate();
  virtual Int_t   Version() const { return 2; }
  void BLsub(int Chn, vector<UShort_t>& ChVals,
  	     vector<double>& BL, vector<ULong64_t>& CumSum);
  
  ClassDef(BaselineRemovalv5,0);
};

void BaselineRemovalv5::Init(TTree *tree)
{
  // The Init() function is called when the selector needs to initialize
  // a new tree or chain. Typically here the branch addresses and branch
  // pointers of the tree will be set.
  printf("Initialising BaselineRemovalv5!\n");
  nentries = tree->GetEntries();
  printf("# of entries = %i\n", nentries);
  
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

void BaselineRemovalv5::SlaveBegin(TTree *tree)
{
   // SlaveBegin() is a good place to create histograms. 
   // For PROOF, this is called for each worker.
   // The TTree* is there for backward compatibility; e.g. PROOF passes 0.
   printf("Calling BaselineRemovalv5::SlaveBegin\n");

   TString option = GetOption();

   //Define the ResultTree
   ChnFile = new TFile("BLsubtractedv5.root", "RECREATE");
   ResultTree = new TTree("Results", "The algorithm processing results");

   ResultTree->Branch("Idx", &Idx, "Idx[2520]/s");
   ResultTree->Branch("AcqNum", &AcqNum, "AcqNum/L");
   ResultTree->Branch("AcqNumInSpill", &AcqNumInSpill, "AcqNumInSpill/L");
   ResultTree->Branch("Signal", "std::vector<std::vector<double> >",
		      &vecSignal);
   ResultTree->Branch("Baseline", "std::vector<std::vector<double> >",
		      &vecBaseline);
   ResultTree->Branch("BLOKidx", "std::vector<UShort_t>", &BLOKidx);
   ResultTree->Branch("TicksSinceADCon",&fTicksSinceADCon,"TicksSinceADCon/l");
   ResultTree->Branch("QDCsimple", "std::vector<std::vector<double> >",
		      &QDCsimple);
   ResultTree->Branch("TimeSimple", "std::vector<std::vector<double> >",
		      &TimeSimple);
   ResultTree->Branch("PulseWidthSimple", "std::vector<std::vector<double> >",
		      &PulseWidthSimple);
   ResultTree->Branch("InterpDiff", "vector< vector<double> >", &InterpDiff);
   ResultTree->Branch("PulseHeight", "vector< vector<double> >", &PulseHeight);


}

void BaselineRemovalv5::BLsub(int chn,
		    std::vector<UShort_t>& ChVals,
		    std::vector<double>& BL,
		    std::vector<ULong64_t>& CumSum){

  bool hazTriggered = false;
  unsigned short ThisTrigStart, ThisTrigStop;
  int StartIdx = 0;
  int StopIdx = 0;
  int UnTrigCounter = 0;

  std::vector<unsigned short> TrigStop;
  std::vector<unsigned short> TrigStart;

  //Initialise the baseline estimate parameter.
  double ThisBL = avg_baseline[chn];
  if((chn==3)&&(MaxCh3<(avg_baseline[3]+200))){
    ThisBL = MaxCh3;
  }

  bool BLisOK = false;

  int PreTrigOffset = max(trig_pre_samples[chn], avg_post_samples[chn]);

  for(int k=0; k<kNsamples; k++){

    //Also the moving average BL (without ROIs).
    if((k>(avg_pre_samples[chn]-1))&&
       (k<(kNsamples-1-avg_post_samples[chn]))){
      //process normally.
      BL.at(k) =
	static_cast<double>(CumSum.at(k+avg_post_samples[chn])-
	 CumSum.at(k-avg_pre_samples[chn]))/
	static_cast<double>((k+avg_post_samples[chn])-(k-avg_pre_samples[chn]));
    }
    else if(k<(kNsamples-1-avg_post_samples[chn])){
      //at beginning of wfm, calculate avg between
      //[0, avg_post_samples+k]
      BL.at(k) =
	static_cast<double>(CumSum.at(k+avg_post_samples[chn])-CumSum.at(0))/
	static_cast<double>(avg_post_samples[chn]+k);
    }
    else{
      //at end of wfm, calc avg btw [(k)-avg_post_samples, kNsamples-1]
      BL.at(k) =
	static_cast<double>(CumSum.at(kNsamples-1)-
	 CumSum.at(k-avg_pre_samples[chn]))/
	static_cast<double>((kNsamples-1)-(k-avg_pre_samples[chn]));
    }


    //See if I've got a good read for the BL yet...
    if(k>(PreTrigOffset-1)){
      if((abs(BL.at(k-PreTrigOffset)-ThisBL)<max_amplitude[chn])&&
	 (!BLisOK)){
	BLisOK = true;
	BLOKidx.at(chn) = k;
      }
    }



    //Step through waveform and get every trigger
    //Check to see if the BL has been calculated 'properly' yet.
    if(!BLisOK){
      //Haven't got a good read on the BL yet.
      //Use the estimated value.
      if(abs(ChVals.at(k)-ThisBL)>max_amplitude[chn]){
	UnTrigCounter = 0;
	  if(!hazTriggered){
	    //We have a trigger!
	    TrigStart.push_back(k);
	    hazTriggered = true;
	  }
      }
      //Now check if we're already in a trigger and value is sub-threshold
      if((abs(ChVals.at(k)-ThisBL)<max_amplitude[chn])&&
	 (hazTriggered)){
	UnTrigCounter++;
	if(UnTrigCounter>UnTrigSettleLength[chn]){
	  TrigStop.push_back(k-UnTrigCounter);
	  hazTriggered = false;
	  UnTrigCounter = 0;
	}
      }
    }
    else{//process normally
      if(abs(ChVals.at(k)-BL.at(k-PreTrigOffset))>max_amplitude[chn]){
	UnTrigCounter = 0;
	if(!hazTriggered){
	  //We have a trigger!
	  TrigStart.push_back(k);
	  hazTriggered = true;
	  ThisBL = BL.at(k-PreTrigOffset);
	}
      }
      //Now check if we're already in a trigger and value is sub-threshold
      ///////////////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////////////
      //To implement the new algorithm, I just need to make max_amplitude
      //into some other (smaller) parameter here.
      ///////////////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////////////
      if((abs(ChVals.at(k)-ThisBL)<max_amplitude[chn])&&
	 (hazTriggered)){
	UnTrigCounter++;
	if(UnTrigCounter>UnTrigSettleLength[chn]){
	  TrigStop.push_back(k-UnTrigCounter);
	  hazTriggered = false;
	  UnTrigCounter = 0;
	}
      }
    }
 }

  //Check if the loop left the trigger on, and add a trigger stop at last idx.
  if(hazTriggered){TrigStop.push_back(kNsamples);}


  //Now I can just iterate over the triggered sections
  std::vector<unsigned short>::iterator itStart = TrigStart.begin();
  std::vector<unsigned short>::iterator itStop = TrigStop.begin();

  //These variables are for checking the time between triggers is long enough to
  //get a moving average.
  unsigned short PrevStopIdx = 0;
  unsigned short PrevStartIdx = 0;

  double PulseSum = 0;

  if(TrigStart.size()!=TrigStop.size()){
    printf("ERROR: TrigStart is not same size as TrigStop!\n");
    printf("TrigStart.size() = %i, TrigStop.size() = %i \n",
	   TrigStart.size(), TrigStop.size());
    printf("TrigStop.at(0) = %i\n", TrigStop.at(0));
    printf("TrigStop.back() = %i\n", TrigStop.back());
  }
  else{//process normally
    while(itStart != TrigStart.end()){
      ThisTrigStart = *itStart;
      ThisTrigStop = *itStop;
      
      ////////////////////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////////////////////
      //To implement the new algorithm, I need to modify the lines below...
      //StartIdx and StopIdx are the points between which the BL is interpolated
      ////////////////////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////////////////////
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

      if(((ThisTrigStart)>(PrevStopIdx+trig_pre_samples[chn]))||
	 (PrevStopIdx==0)){//The second condition is for initialisation.
	

	//Get timing info and pulse width
	TimeSimple.at(chn).push_back(ThisTrigStart);
	PulseWidthSimple.at(chn).push_back(ThisTrigStop-ThisTrigStart);
	//Fill out the interp difference
	InterpDiff.at(chn).push_back(BL.at(StopIdx) - BL.at(StartIdx));
	double PulseMin = 16384;
	double BLatMin = 0;

	for(int q=StartIdx;
	    q<StopIdx;
	    q++){
	  //Adjust BL to be interpolated between ROI
	  BL.at(q) = BL.at(StartIdx-1) + 
	    (BL.at(StopIdx+1) - BL.at(StartIdx-1))*
	    (q - (StartIdx - 1))/(StopIdx+1 - (StartIdx - 1));

	  PulseSum += -(ChVals.at(q)-BL.at(q));

	  if((ChVals.at(q))<PulseMin){
	    PulseMin = ChVals.at(q);
	    BLatMin = BL.at(q);
	  }
	  
	}
	//printf("foo\n");
	//Get simplified pulse parameters
	QDCsimple.at(chn).push_back(PulseSum);
	PulseHeight.at(chn).push_back(-(PulseMin - BLatMin));
	PulseSum = 0;

      }
      else{
	//next pulse has arrived too soon!
	//This shouldn't be happening if I've set UnTrigSettleLength correctly!
	printf("ERROR: Two pulses arriving within each others' index length are being counted separately! Something is wrong with the code!! Investigate!!!\n");
	printf("PrevStartIdx = %i, ", PrevStartIdx);
	printf("PrevStopIdx = %i, ", PrevStopIdx);
	printf("StartIdx = %i,  ", StartIdx);
	printf("StopIdx = %i\n", StopIdx);
      }
      PrevStopIdx = StopIdx;
      PrevStartIdx = StartIdx;
      ++itStart;
      ++itStop;
    }
  }

}


Bool_t BaselineRemovalv5::Process(Long64_t entry)
{
   // The Process() function is called for each entry in the tree to be 
   // processed. The entry argument specifies which entry in the currently
   // loaded tree is to be processed.
  //double thisFraction = 10*(entry/nentries);
  //printf("thisFraction = %i, floor(thisFraction) = %i \n", thisFraction,
  //	 floor(thisFraction));

  //if(thisFraction==floor(thisFraction)){
  //printf("processing root file... %f percent complete\n",100*entry/nentries);
  //printf("entry = %i, nentries = %i\n", entry, nentries);
  //}

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
     vecChanVals.at(1).at(j) = fCh1[j];
     vecChanVals.at(2).at(j) = fCh2[j];
     vecChanVals.at(3).at(j) = fCh3[j];
     vecCumSum.at(0).at(j) = fCumSumCh0[j]-fCumSumCh0[50];
     vecCumSum.at(1).at(j) = fCumSumCh1[j]-fCumSumCh1[50];
     vecCumSum.at(2).at(j) = fCumSumCh2[j]-fCumSumCh2[50];
     vecCumSum.at(3).at(j) = fCumSumCh3[j]-fCumSumCh3[50];
     Idx[j] = j+40;//1st 40 values weren't used

     //Get the starting estimate for Ch3's baseline (see notes).
     if(fCh3[j]>MaxCh3){MaxCh3 = fCh3[j];}

   }

   
   //AcqNum is now a unique identifier for each trace.
   AcqNum = entry + SourceTree->GetChainOffset();
   AcqNumInSpill = entry;


   // *** 2. *** Do the actual analysis
   for(int i=0; i<4; i++){
     //Pulse parameters have unspecified column lengths
     QDCsimple.push_back(ColDouble);
     TimeSimple.push_back(ColDouble);
     PulseWidthSimple.push_back(ColDouble);
     InterpDiff.push_back(ColDouble);
     PulseHeight.push_back(ColDouble);
     //Call the pulse processing algorithm for each channel
     BLsub(i, vecChanVals.at(i), vecBaseline.at(i), vecCumSum.at(i));

     //Fill the signal vector
     for(int j = 0; j<kNsamples; j++){
       vecSignal.at(i).at(j) = 
	 -(vecChanVals.at(i).at(j) - vecBaseline.at(i).at(j));

       vecAvgSignal.at(i).at(j) += vecSignal.at(i).at(j);
       vecAvgSignal.at(i).at(j) = vecAvgSignal.at(i).at(j)/2;

     }
   }

   for(int i=0; i<4; i++){
     for(int j = 0; j<kNsamples; j++){
       //Iterate over TimeSimple and see if there is a value that meets the cut
       std::vector<double>::iterator itTime = TimeSimple.at(i).begin();
       for(itTime; itTime != TimeSimple.at(i).end(); ++itTime){
	 //Then do the same for the hodoscope channels.
	 if((325<*itTime)&&(*itTime<375)){
	   std::vector<double>::iterator itH1Time = TimeSimple.at(2).begin();
	   for(itH1Time; itH1Time != TimeSimple.at(2).end(); ++itH1Time){
	     if((390<*itH1Time)&&(*itH1Time<450)){
	       std::vector<double>::iterator itH2Time=TimeSimple.at(3).begin();
	       for(itH2Time; itH2Time != TimeSimple.at(3).end(); ++itH2Time){
		 if((390<*itH2Time)&&(*itH2Time<450)){
		   //Store the raw wfm
		   vecAvgSignalwCuts.at(i).at(j) += vecSignal.at(i).at(j);
		   vecAvgSignalwCuts.at(i).at(j) =
		     vecAvgSignalwCuts.at(i).at(j)/2;
		   //Store the shifted wfm
		   double shift = 400 - *itH1Time;
		   if((0<(j-shift))&&((j-shift)<kNsamples)){
		     vecAvgShiftSigwCuts.at(i).at(j) += 
		       vecSignal.at(i).at(j-shift);
		     vecAvgShiftSigwCuts.at(i).at(j) =
		       vecAvgShiftSigwCuts.at(i).at(j)/2;
		   }
		 }
	       }
	     }
	   }
	 }
       }
     }
   }

   ResultTree->Fill();
   
   //Pulse parameters have unspecified column lengths
   QDCsimple.clear();
   TimeSimple.clear();
   PulseWidthSimple.clear();
   InterpDiff.clear();
   PulseHeight.clear();
   
   return kTRUE;
}

void BaselineRemovalv5::Terminate()
{
   // The Terminate() function is the last function to be called during the
   // analysis of a tree with a selector. It always runs on the client, it can
   // be used to present the results graphically or save the results to file
 
  ResultTree->Write();
  ChnFile->Close();

  AvgFile = new TFile("AvgWfms.root", "RECREATE");
  AvgSigTree = new TTree("AvgSignal", "The Avg Wfm Signal");
  
  AvgSigTree->Branch("AvgSig", "std::vector<std::vector<double> >",
		     &vecAvgSignal);
  AvgSigTree->Branch("AvgSigwCuts", "std::vector<std::vector<double> >",
		     &vecAvgSignalwCuts);
  AvgSigTree->Branch("Idx", &Idx, "Idx[2520]/s");
  AvgSigTree->Branch("AvgShiftSigwCuts", "std::vector<std::vector<double> >",
		     &vecAvgShiftSigwCuts);
  AvgSigTree->Branch("avg_baseline", &avg_baseline, "avg_baseline[4]/D");
  AvgSigTree->Branch("max_amplitude", &max_amplitude, "max_amplitude[4]/D");
  AvgSigTree->Branch("avg_pre_samples", &avg_pre_samples,
		     "avg_pre_samples[4]/I");
  AvgSigTree->Branch("avg_post_samples", &avg_post_samples,
		     "avg_post_samples[4]/I");
  AvgSigTree->Branch("trig_pre_samples", &trig_pre_samples,
		     "trig_pre_samples[4]/I");
  AvgSigTree->Branch("trig_post_samples", &trig_post_samples,
		     "trig_post_samples[4]/I");
  AvgSigTree->Branch("UnTrigSettleLength", &UnTrigSettleLength,
		     "UnTrigSettleLength[4]/I");



  AvgSigTree->Fill();
  AvgSigTree->Write();
  //AvgSigTree->SaveAs("AvgWfms.root", "RECREATE");
  AvgFile->Close();
}
