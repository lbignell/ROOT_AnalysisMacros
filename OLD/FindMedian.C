//This macro will: Open the specified ROOT file, grab the relevant data, apply
//cuts (for BeamON and LEDON conditions), and determine the median value of the
//resulting histogram. The rationale here is that the median is a more robust
//estimate of the centre of the distribution than the mean (which is skewed by
//the smattering of events at very large QDC).

#include "TROOT.h"
#include "TChain.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TLeaf.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TVectorD.h"
#include <vector>

using namespace std;

void* GetPointerToValue(TBranch* theBranch, int entry,
				const char* name){
  theBranch->GetEntry(entry);
  TLeaf* theLeaf = theBranch->GetLeaf(name);
  return theLeaf->GetValuePointer();
}

Double_t FindMedian(const char* filename){

  //Open the file
  TFile* f = new TFile(filename);

  TTree* ResultsTree = (TTree*)f->Get("Results");

  TBranch* QDCBranch = (TBranch*)ResultsTree->GetBranch("QDCsimple");
  TBranch* SignalBranch = (TBranch*)ResultsTree->GetBranch("Signal");
  //TBranch* IdxBranch = (TBranch*)ResultsTree->GetBranch("Idx");
  TBranch* TimeBranch = (TBranch*)ResultsTree->GetBranch("TimeSimple");
  TBranch* BLOKidxBranch = (TBranch*)ResultsTree->GetBranch("BLOKidx");

  Long64_t nentries = ResultsTree->GetEntries();

  std::vector< std::vector< double > > vecQDC;
  std::vector< std::vector< double > > vecTime;
  std::vector< std::vector< double > > vecPulseWidth;
  std::vector< std::vector< double > > vecSignal;

  std::vector< std::vector< std::vector< double > > > LEDONSignal;
  std::vector< std::vector< std::vector< double > > > BeamONSignal;
  std::vector< std::vector<double> > vecCol;

  for(int i=0; i<4; i++){
    LEDONSignal.push_back(vecCol);
    BeamONSignal.push_back(vecCol);
  }

  for(int i = 0; i<nentries; i++){
    //Want to do LED cuts first...
    //Get the vector of vectors for TimeSimple
    vecTime = *(std::vector<std::vector<double> >*)
      (GetPointerToValue(TimeBranch, i, TimeBranch->GetName()));
    //Check if there's a Ch2 pulse @ 1039<Time<1115
    std::vector<double>::iterator itCh2 = vecTime.at(2).begin();
    int m = 0;
    for(; itCh2!=vecTime.at(2).end(); ++itCh2){
      if((*itCh2>1025)&&(*itCh2<1075)){//Ch2 Pulse at right spot
	vecQDC = *(std::vector<std::vector<double> >*)
	  (GetPointerToValue(QDCBranch, i, QDCBranch->GetName()));
	//Now check if there's a Ch2 pulse @ 135000<QDC<160000
	double QDCch2 = vecQDC.at(2).at(m);
	if((QDCch2>135000)&&(QDCch2<160000)){
	  //There's definitely an LED pulse, check for trigger in channel.
	  for(int j = 0; j<2; j++){//loop over the PMTs
	    //use vector iterator to step over values...
	    std::vector<double>::iterator itTime = vecTime.at(j).begin();
	    std::vector<double>::iterator itPulseWidth =
	      vecPulseWidth.at(j).begin();
	    int k = 0;
	    for(; itTime!=vecTime.at(j).end(); ++itTime){
	      //iterate through vector.
	      if((*itTime>1024)&&(*itTime<1035)){
		//LED is on!
		//Double-check pulse height is OK
		if(vecQDC.at(j).at(k)>5){
		  //it's all legit, process the data.

		}
	      }
	      k++;
	    }
	  }
	}
      }
      m++;
    }//End of LED ON cuts.

    
    std::vector<double>::iterator itH1Time = vecTime.at(2).begin();
    std::vector<double>::iterator itH2Time = vecTime.at(3).begin();

    for(int j=0; j<2; j++){
      //use vector iterator to step over values...
      std::vector<double>::iterator itTime = vecTime.at(j).begin();
      int k = 0;
      
      for(itTime; itTime!=vecTime.at(j).end(); ++itTime){
	if((*itTime>324.5)&&(*itTime<354.5)){
	  //correct time for PMT, check whether there were triggers for H1 & H2
	  for(itH1Time; itH1Time!=vecTime.at(2).end(); ++itH1Time){
	    if((*itH1Time>390)&&(*itH1Time<440)){
	      //success! now check H2
	      for(itH2Time; itH2Time!=vecTime.at(2).end(); ++itH2Time){
		if((*itH2Time>390)&&(*itH2Time<440)){
		  //Trigger is met!
		}
	      }
	    }
	  }
	}
      }
    }
  }// end of loop on tree entries

}
