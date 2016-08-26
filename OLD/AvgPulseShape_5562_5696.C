//This is a macro to get the average pulse shape in Channel A/B and compare the
//shape at low light levels (single PE) to high light levels (beam on).
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

void AvgPulseShape_5562_5696(){


  TBranch* QDCBranch;
  TBranch* SignalBranch;
  TBranch* TimeBranch;
  TBranch* PulseWidthBranch;
  TBranch* IdxBranch;
  TBranch* AcqNumInSpillBranch;//For getting LED testing pulses.
  //TBranch* AcqNumBranch;
  TBranch* BLOKidxBranch;

  std::vector< std::vector< double > > vecQDC;
  std::vector< std::vector< double > > vecTime;
  std::vector< std::vector< double > > vecPulseWidth;
  std::vector< std::vector< double > > vecSignal;
  TH1D* LEDONQDC = new TH1D("QDC, LED ON", "QDC Histogram for LED ON events",
			    10000, 0, 1000);
  TH1D* BeamONQDC = new TH1D("QDC, Beam ON","QDC Histogram for Beam ON events",
			     10000, 0, 10000);

  unsigned short Idx[2520];
  Long64_t AcqNumInSpill;
  Long64_t nentries;
  Long64_t AcqNum;

  std::vector< std::vector< std::vector< double > > > LEDONSignal;
  std::vector< std::vector< std::vector< double > > > BeamONSignal;
  std::vector< std::vector< std::vector< double > > > DarkSignal;
  std::vector< std::vector<double> > vecCol;

  for(int i=0; i<4; i++){
    LEDONSignal.push_back(vecCol);
    BeamONSignal.push_back(vecCol);
    DarkSignal.push_back(vecCol);
  }

  std::vector<UShort_t> BLOKidx;

  //TLeaf* AcqNumInSpillLeaf;

  TFile* f = TFile::Open("BLsubtractedv3_5562_5696.root");

  TTree* ResultsTree = (TTree*)f->Get("Results");

  //for testing
  ResultsTree->Print();

  QDCBranch = (TBranch*)ResultsTree->GetBranch("QDCsimple");
  SignalBranch = (TBranch*)ResultsTree->GetBranch("Signal");
  IdxBranch = (TBranch*)ResultsTree->GetBranch("Idx");
  PulseWidthBranch = (TBranch*)ResultsTree->GetBranch("PulseWidthSimple");
  TimeBranch = (TBranch*)ResultsTree->GetBranch("TimeSimple");
  AcqNumInSpillBranch = (TBranch*)ResultsTree->GetBranch("AcqNumInSpill");
  //AcqNumBranch = (TBranch*)ResultsTree->GetBranch("AcqNum");
  BLOKidxBranch = (TBranch*)ResultsTree->GetBranch("BLOKidx");

  //  ResultsTree->SetBranchAddress(

  nentries = ResultsTree->GetEntries();


  //Testing
  //AcqNum = *(Long64_t*)(GetPointerToValue(AcqNumBranch, 56, AcqNumBranch->GetName()));
  //vecQDC = *(std::vector<std::vector<double> >*)
  //(GetPointerToValue(QDCBranch, 56, QDCBranch->GetName()));
  //printf("AcqNum = %i\n", AcqNum);
  //printf("QDC.at(1).at(1) in event 56 = %f\n", vecQDC.at(1).at(1));	  
  
  for(int i = 0; i<nentries; i++){
    
    //Get AcqNumInSpill for entry i
    //AcqNumInSpill = *(Long64_t*)
    //(GetPointerToValue(AcqNumInSpillBranch, i,
    //			 AcqNumInSpillBranch->GetName()));

    //if(AcqNumInSpill>180){//Definitely out-of-spill
      //Find entries which correspond to LED cuts.
      //Requirements: TimeSimple[PMT] > 1005,
      //TimeSimple[PMT]<1055, 1025<Ch2 time<1075,
    //135000<Ch2 QDC<160000
    
    //Get the vector of vectors for TimeSimple and PulseWidth
    vecTime = *(std::vector<std::vector<double> >*)
      (GetPointerToValue(TimeBranch, i, TimeBranch->GetName()));
    vecPulseWidth = *(std::vector<std::vector<double> >*)
      (GetPointerToValue(PulseWidthBranch, i, PulseWidthBranch->GetName()));
    
    //First Check if there's a Ch2 pulse @ 1025<Time<1075
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
	  for(int j = 0; j<1; j++){//loop over the PMTs
	    //use vector iterator to step over values...
	    std::vector<double>::iterator itTime = vecTime.at(j).begin();
	    std::vector<double>::iterator itPulseWidth =
	      vecPulseWidth.at(j).begin();
	    int k = 0;
	    
	    for(; itTime!=vecTime.at(j).end(); ++itTime){
	      //iterate through vector.
	      if((*itTime>1020)&&(*itTime<1040)){
		//LED is on!
		//Double-check pulse height is OK
		if(vecQDC.at(j).at(k)>5){
		  //it's all legit, process the data.
		  vecSignal = *(std::vector<std::vector<double> >*)
		    (GetPointerToValue(SignalBranch, i, 
				       SignalBranch->GetName()));
		  
		  LEDONSignal.at(j).push_back((vecSignal.at(j)));
		  
		  LEDONQDC->Fill(vecQDC.at(j).at(k));
		  
		  
		}
	      }
	      else if(*itTime>1055){
		//we've gone past, so break
		break;
	      }
	      k++;//keep track of pulse # in waveform
	    }
	  }
	}
      }
      m++;
    }
    
    //}//end of acqnuminspill condition
    //else{//In spill...
    //Trigger conditions: 324.5<TimeSimple[PMT]<390.5, BLOKidx[ALL]<100,
    //390<TimeSimple[H1/H2]<440, PulseWidthSimple[H1]<60,
    //PulseWidthSimple[H2]<30.
    
    //BLOKidx = *(std::vector<UShort_t>*)(GetPointerToValue(BLOKidxBranch,i,BLOKidxBranch->GetName()));
    
    //std:vector<UShort_t> BLtest (4,100); 
    
    //if(BLOKidx<BLtest){
    
    //Get the vector of vectors for TimeSimple and PulseWidth
    //vecTime = *(std::vector<std::vector<double> >*)
    //(GetPointerToValue(TimeBranch, i, TimeBranch->GetName()));
    //vecPulseWidth = *(std::vector<std::vector<double> >*)
    //(GetPointerToValue(PulseWidthBranch,i,PulseWidthBranch->GetName()));
    std::vector<double>::iterator itH1Time = vecTime.at(2).begin();
    std::vector<double>::iterator itH2Time = vecTime.at(3).begin();
    
    for(int j = 0; j<1; j++){
      //use vector iterator to step over values...
      std::vector<double>::iterator itTime = vecTime.at(j).begin();
      std::vector<double>::iterator itPulseWidth =
	vecPulseWidth.at(j).begin();
      int k = 0;
      
      for(itTime; itTime!=vecTime.at(j).end(); ++itTime){
	//iterate through vector.
	if((*itTime>324.5)&&(*itTime<353.5)){
	  //correct time for PMT, check whether there were triggers for
	  //H1 and H2
	  for(itH1Time; itH1Time!=vecTime.at(2).end(); ++itH1Time){
	    if((*itH1Time>390)&&(*itH1Time<440)){
	      //success! now check H2
	      for(itH2Time; itH2Time!=vecTime.at(2).end(); ++itH2Time){
		if((*itH2Time>390)&&(*itH2Time<440)){
		  //Trigger is met
		  //NB: NOT looking at pulse width
		  //Double-check pulse height is OK
		  vecQDC = *(std::vector<std::vector<double> >*)
		    (GetPointerToValue(QDCBranch,i,QDCBranch->GetName()));
		  if((vecQDC.at(j).at(k)<10000)&&
		     (vecQDC.at(j).at(k)>1000)){
		    //Process...
		    vecSignal = *(std::vector<std::vector<double> >*)
		      (GetPointerToValue(SignalBranch,
					 i, SignalBranch->GetName()));
		    
		    BeamONSignal.at(j).push_back(vecSignal.at(j));
		    
		    BeamONQDC->Fill(vecQDC.at(j).at(k));
		    
		    
		  }
		  //break after processing, can't trigger more than once
		  break;
		}
		else if(*itH2Time>440){
		  //we've gone past, so break
		  break;
		}
	      }
	      //we've found the trigger, no need to keep going
	      //break;
	    }
	    else if(*itH1Time>440){
	      //we've gone past, so break
	      break;
	    }
	  }
	}
	else if(*itTime>390.5){
	  //we've gone past, so break
	  break;
	}
	k++;
      }	  
    }
    //}//BLOKidx condition
    //}//else condition
  }
  
  
  //ResultsTree->SetBranchAddress("QDCsimple", &vecQDC, &QDCBranch);
  //ResultsTree->SetBranchAddress("Signal", &vecSignal, &SignalBranch);
  //ResultsTree->SetBranchAddress("Idx", Idx, &IdxBranch);
  //ResultsTree->SetBranchAddress("PulseWidthSimple",
  //				&vecPulseWidth, &PulseWidthBranch);
  //ResultsTree->SetBranchAddress("TimeSimple", &vecTime, &TimeBranch);
  //ResultsTree->SetBranchAddress("AcqNumInSpill",
  //				&AcqNumInSpill, &AcqNumInSpillBranch);

  //printf("Finished Event Loop, closing file");

  f->Close();

  //At this point it may be nice to output some plots.
  //Outputting the QDC histograms for the cuts implemented.

  TCanvas* c1 = new TCanvas("QDC, LED ON", "QDC Histogram with LED ON", 2);
  LEDONQDC->GetXaxis()->SetTitle("Charge Collected (A.U.)");
  LEDONQDC->Draw("");

  TCanvas* c2 = new TCanvas("QDC, Beam ON", "QDC Histogram with Beam ON", 3);
  BeamONQDC->GetXaxis()->SetTitle("Charge Collected (A.U.)");
  BeamONQDC->Draw("");

  //OK, now take the average of all waveforms in each vector.
  //Vectors will be 4x2520
  std::vector< std::vector<double> > avgLEDONSignal;
  std::vector< std::vector<double> > avgBeamONSignal;
  std::vector<double> dummy;
  dummy.resize(2520,0);
  avgLEDONSignal.resize(4,dummy);
  avgBeamONSignal.resize(4,dummy);

  //Calculate average LEDON wfms
  for(int i=0; i<4; i++){
    std::vector< std::vector<double> >::iterator itLEDONwfm =
      LEDONSignal.at(i).begin();
    std::vector<double>::iterator itLEDONelement;
    bool isFirst = true;
    for(;itLEDONwfm!=(LEDONSignal.at(i).end());++itLEDONwfm){
      int j = 0;
      for(itLEDONelement = itLEDONwfm->begin();
  	  itLEDONelement != itLEDONwfm->end();
  	  itLEDONelement++){
  	if(!isFirst){
  	  avgLEDONSignal.at(i).at(j) += *(itLEDONelement);
  	  avgLEDONSignal.at(i).at(j) = avgLEDONSignal.at(i).at(j)/2;
  	}
  	else{
  	  avgLEDONSignal.at(i).at(j) += *(itLEDONelement);
  	}
  	j++;
      }
      isFirst = false;
    }
  }

  //Calculate average BeamON wfms
  for(int i=0; i<4; i++){
    std::vector< std::vector<double> >::iterator itBeamONwfm =
      BeamONSignal.at(i).begin();
    std::vector<double>::iterator itBeamONelement;
    bool isFirst = true;
    for(;itBeamONwfm!=(BeamONSignal.at(i).end());++itBeamONwfm){
      int j = 0;
      for(itBeamONelement = itBeamONwfm->begin();
  	  itBeamONelement != itBeamONwfm->end();
  	  itBeamONelement++){
  	if(!isFirst){
  	  avgBeamONSignal.at(i).at(j) += *(itBeamONelement);
  	  avgBeamONSignal.at(i).at(j) = avgBeamONSignal.at(i).at(j)/2;
  	}
  	else{
  	  avgBeamONSignal.at(i).at(j) += *(itBeamONelement);
  	}
  	j++;
      }
      isFirst = false;
    }
  }

  Double_t xVals[2520];
  Double_t AvgLEDONch0[2520];
  Double_t AvgBeamONch0[2520]; 
  for(int i=0; i<2520; i++){
    xVals[i] = i;
    AvgLEDONch0[i] = avgLEDONSignal.at(0).at(i);
    AvgBeamONch0[i] = avgBeamONSignal.at(0).at(i);
  }

  //Peak locations for LEDON and BeamON at 1032 and 354, respectively
  Double_t xVals2[100];
  Double_t BeamDivLED[100];
  Double_t ScaleLEDON[100];
  Double_t ScaleBeamON[100];
  for(int i=0; i<100; i++){
    BeamDivLED[i] = AvgBeamONch0[i+(351-50)]/AvgLEDONch0[i+(1032-50)];
    xVals2[i] = i;
    ScaleLEDON[i] = AvgLEDONch0[i+(1032-50)]*(100./28);
    ScaleBeamON[i] = (AvgBeamONch0[i+(351-50)])*(100./670.);
    //printf("ScaleBeamON[%i] = %f...    ", i, ScaleBeamON[i]);
    //printf("AvgBeamONch0[%i+(354-50)] = %f...   ", i, AvgBeamONch0[i+(354-50)]);
  }

//const TVectorD& xValsConst = xVals;
// const TVectorD& AvgLEDONch0 = avgLEDONSignal.at(0);
//const TVectorD& AvgBeamONch0 = avgBeamONSignal.at(0);

  TCanvas* c3 = new TCanvas("LED ON avg", "Average LED ON waveform", 4);
  TGraph* LEDONgraph = new TGraph(2520, xVals, AvgLEDONch0);
  LEDONgraph->GetXaxis()->SetTitle("Time (ns)");
  LEDONgraph->Draw("AL");

  TCanvas* c4 = new TCanvas("Beam ON avg", "Average Beam ON waveform", 5);
  TGraph* BeamONgraph = new TGraph(2520, xVals, AvgBeamONch0);
  BeamONgraph->GetXaxis()->SetTitle("Time (ns)");
  BeamONgraph->Draw("AL");

  TCanvas* cv5 = new TCanvas("Beam divided by LED",
			    "Avg Beam wfm divided by Avg LED wfm", 6);
  TGraph* BeamDivLEDgraph = new TGraph(100, xVals2, BeamDivLED);
  BeamDivLEDgraph->GetXaxis()->SetTitle("Time (ns)");
  BeamDivLEDgraph->GetXaxis()->SetRangeUser(30., 70.);
  BeamDivLEDgraph->GetYaxis()->SetRangeUser(-25., 125.);
  BeamDivLEDgraph->Draw("AL");

  TGraph* ScaleLEDONgraph = new TGraph(100, xVals2, ScaleLEDON);
  ScaleLEDONgraph->Draw("SAME");
  TGraph* ScaleBeamONgraph = new TGraph(100, xVals2, ScaleBeamON);
  ScaleBeamONgraph->Draw("SAME");
  //TCanvas* cv6 = new TCanvas("Test Canvas",
  //			    "Normalised avg wfms", 1);
  //ScaleLEDONgraph->Draw("AL");
  //ScaleBeamONgraph->Draw("SAME");


}

