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

void AvgPulseShape_PulseWidth(){


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
  std::vector< std::vector<UShort_t> > vecIdx;
  std::vector<UShort_t> Idx;
  TH1D* QDCFirstBunchCh2 =
    new TH1D("QDCFirstBunchCh2","QDC Histogram for Ch2, First PulseWidth bunch",
	     100000, 0, 100000);
  TH1D* QDCSecondBunchCh2 =
    new TH1D("QDCSecondBunchCh2",
	     "QDC Histogram for Ch2, Second PulseWidth bunch",
	     100000, 0, 100000);
  TH1D* QDCHigherBunchesCh2 =
    new TH1D("QDCHigherBunchesCh2",
	     "QDC Histogram for Ch2, Higher PulseWidth bunches",
	     100000, 0, 100000);
  TH1D* QDCFirstBunchCh3 =
    new TH1D("QDCFirstBunchCh3","QDC Histogram for Ch3, First PulseWidth bunch",
	     100000, 0, 100000);
  TH1D* QDCSecondBunchCh3 =
    new TH1D("QDCSecondBunchCh3",
	     "QDC Histogram for Ch3, Second PulseWidth bunch",
	     100000, 0, 100000);
  TH1D* QDCHigherBunchesCh3 =
    new TH1D("QDCHigherBunchesCh3",
	     "QDC Histogram for Ch3, Higher PulseWidth bunches",
	     100000, 0, 100000);

  //unsigned short Idx[2520];
  for(int i = 0; i<2520; i++){
    Idx[i] = i;
  }

  Long64_t AcqNumInSpill;
  Long64_t nentries;
  Long64_t AcqNum;

  std::vector< std::vector< std::vector< double > > > PulseWidthFirstBunch;
  std::vector< std::vector< std::vector< double > > > PulseWidthSecondBunch;
  std::vector< std::vector< std::vector< double > > > PulseWidthHigherBunches;
  std::vector< std::vector<double> > vecCol;
  std::vector< std::vector<double> > PWFirst;
  std::vector< std::vector<double> > PWSecond;
  std::vector< std::vector<double> > PWHigher;
  std::vector< std::vector<UShort_t> > ShrtCol;
  std::vector<double> dblVec;

  for(int i=0; i<4; i++){
    PulseWidthFirstBunch.push_back(vecCol);
    PulseWidthSecondBunch.push_back(vecCol);
    PulseWidthHigherBunches.push_back(vecCol);
    PWFirst.push_back(dblVec);
    PWSecond.push_back(dblVec);
    PWHigher.push_back(dblVec);
    vecIdx.push_back(Idx);
  }

  std::vector<UShort_t> BLOKidx;


  TFile* f = TFile::Open("BLsubtractedv3_5461_5561.root");

  TTree* ResultsTree = (TTree*)f->Get("Results");

  //for testing
  ResultsTree->Print();

  QDCBranch = (TBranch*)ResultsTree->GetBranch("QDCsimple");
  SignalBranch = (TBranch*)ResultsTree->GetBranch("Signal");
  IdxBranch = (TBranch*)ResultsTree->GetBranch("Idx");
  PulseWidthBranch = (TBranch*)ResultsTree->GetBranch("PulseWidthSimple");
  TimeBranch = (TBranch*)ResultsTree->GetBranch("TimeSimple");
  AcqNumInSpillBranch = (TBranch*)ResultsTree->GetBranch("AcqNumInSpill");
  BLOKidxBranch = (TBranch*)ResultsTree->GetBranch("BLOKidx");

  TFile* File = new TFile("PulseWidthSortedTriggerWfms.root", "RECREATE");
  TTree* TheTree = new TTree("TriggeredWfms",
			     "Triggered Wfms, sorted by PulseWidth");
  TheTree->Branch("Idx", &Idx, "Idx[2520]/s");
  TheTree->Branch("vecIdx", "std::vector< std::vector<UShort_t> >", &vecIdx);
  TheTree->Branch("FirstBunch", "std::vector< std::vector<double> >", &PWFirst);
  TheTree->Branch("SecondBunch", "std::vector< std::vector<double> >",
		  &PWSecond);
  TheTree->Branch("HigherBunches", "std::vector< std::vector<double> >",
		  &PWHigher);

  nentries = ResultsTree->GetEntries();
  
  for(int i = 0; i<nentries; i++){
    
    //Chan3 cuts: BLOKidx<100, 390<TimeCh3<440, 390<TimeCh2<440. Cluster 1
    //PulseWidth<30, cluster 2 30<PulseWidth<40, another bunch at ~PulseWidth~60
    //Chan2 cuts: same as chan3. Cluster 1 PulseWidth<60, cluster 2 
    //60<PulseWidth<77, other clusters at ~85, 100, 115, 130, 145, 165.
    //I'll leave it with these two for now. I've checked the PulseWidth spectrum
    //for the PMTs; there's structure, but it doesn't appear to be as pronounced
    BLOKidx = *(std::vector<UShort_t>*)
      (GetPointerToValue(BLOKidxBranch,i,BLOKidxBranch->GetName()));
    
    if((BLOKidx.at(0)<100)&&(BLOKidx.at(1)<100)&&
       (BLOKidx.at(2)<100)&&(BLOKidx.at(3)<100)){
      
      //Get the vector of vectors for TimeSimple
      vecTime = *(std::vector<std::vector<double> >*)
	(GetPointerToValue(TimeBranch, i, TimeBranch->GetName()));
      
      //First Check if there's a Ch2 pulse @ 390<Time<400
      std::vector<double>::iterator itCh2Time = vecTime.at(2).begin();
      int m = 0;
      for(; itCh2Time!=vecTime.at(2).end(); ++itCh2Time){
	if((*itCh2Time>420)&&(*itCh2Time<440)){//Ch2 pulse at right spot
	  //The mth Ch2 event is the correct one...
	  std::vector<double>::iterator itCh3Time = vecTime.at(3).begin();
	  int n = 0;
	  for(; itCh3Time!=vecTime.at(3).end(); ++itCh3Time){
	    if((*itCh3Time>420)&&(*itCh3Time<440)){//Ch3 pulse at right spot
	      //Record event!
	      vecPulseWidth = *(std::vector<std::vector<double> >*)
		(GetPointerToValue(PulseWidthBranch, i,
				   PulseWidthBranch->GetName()));
	      vecSignal = *(std::vector<std::vector<double> >*)
		(GetPointerToValue(SignalBranch, i, SignalBranch->GetName()));
	      vecQDC = *(std::vector<std::vector<double> >*)
		(GetPointerToValue(QDCBranch, i, QDCBranch->GetName()));

	      //Fill Chan2	    
	      if(vecPulseWidth.at(2).at(m)<60){
		//Chan2, 1st cluster
		PulseWidthFirstBunch.at(2).push_back(vecSignal.at(2));
		QDCFirstBunchCh2->Fill(vecQDC.at(2).at(m));
		PWFirst.at(2) = vecSignal.at(2);
	      }
	      else if(vecPulseWidth.at(2).at(m)<77){
		//Chan2, 2nd cluster
		PulseWidthSecondBunch.at(2).push_back(vecSignal.at(2));
		QDCSecondBunchCh2->Fill(vecQDC.at(2).at(m));
		PWSecond.at(2) = vecSignal.at(2);
	      }
	      else{
		//chan2, 3rd cluster
		PulseWidthHigherBunches.at(2).push_back(vecSignal.at(2));
		QDCHigherBunchesCh2->Fill(vecQDC.at(2).at(m));
		PWHigher.at(2) = vecSignal.at(2);
	      }
	      
	      //Fill Chan3
	      if(vecPulseWidth.at(3).at(m)<60){
		//Chan2, 1st cluster
		PulseWidthFirstBunch.at(3).push_back(vecSignal.at(3));
		QDCFirstBunchCh3->Fill(vecQDC.at(3).at(m));
		PWFirst.at(3) = vecSignal.at(3);
	      }
	      else if(vecPulseWidth.at(3).at(m)<77){
		//Chan2, 2nd cluster
		PulseWidthSecondBunch.at(3).push_back(vecSignal.at(3));
		QDCSecondBunchCh3->Fill(vecQDC.at(3).at(m));
		PWSecond.at(3) = vecSignal.at(3);
	      }
	      else{
		//chan2, 3rd cluster
		PulseWidthHigherBunches.at(3).push_back(vecSignal.at(3));
		QDCHigherBunchesCh3->Fill(vecQDC.at(3).at(m));
		PWHigher.at(3) = vecSignal.at(3);
	      }
	      
	      TheTree->Fill();
	      //CLEAR all the waveform vectors...
	      //for(int q = 0; q<4; q++){
	      //	PWFirst.at(q).clear();
	      //PWSecond.at(q).clear();
	      //PWHigher.at(q).clear();
	      //}	      
	    }//end of condition checking if Ch3 is at right time
	    n++;
	  }//Loop on itCh3Time
	}//end of condition checking if Ch2 is at right time
	m++;
      }//Loop on itCh2Time
    }//BLOKidx condition
  }
  
  

  f->Close();

  //OK, now take the average of all waveforms in each vector.
  //Vectors will be 4x2520
  std::vector< std::vector<double> > avgPulseWidthFirstBunch;
  std::vector< std::vector<double> > avgPulseWidthSecondBunch;
  std::vector< std::vector<double> > avgPulseWidthHigherBunches;
  std::vector<double> dummy;
  dummy.resize(2520,0);
  avgPulseWidthFirstBunch.resize(4,dummy);
  avgPulseWidthSecondBunch.resize(4,dummy);
  avgPulseWidthHigherBunches.resize(4,dummy);

 
  for(int i=0; i<4; i++){
    std::vector< std::vector<double> >::iterator itFirstBunchwfm =
      PulseWidthFirstBunch.at(i).begin();
    std::vector<double>::iterator itFirstBunchelement;
    bool isFirst = true;
    for(;itFirstBunchwfm!=(PulseWidthFirstBunch.at(i).end());++itFirstBunchwfm){
      int j = 0;
      for(itFirstBunchelement = itFirstBunchwfm->begin();
  	  itFirstBunchelement != itFirstBunchwfm->end();
  	  itFirstBunchelement++){
  	if(!isFirst){
  	  avgPulseWidthFirstBunch.at(i).at(j) += *(itFirstBunchelement);
  	  avgPulseWidthFirstBunch.at(i).at(j) =
	    avgPulseWidthFirstBunch.at(i).at(j)/2;
  	}
  	else{
  	  avgPulseWidthFirstBunch.at(i).at(j) += *(itFirstBunchelement);
  	}
  	j++;
      }
      isFirst = false;
    }
  }


  for(int i=0; i<4; i++){
    std::vector< std::vector<double> >::iterator itSecondBunchwfm =
      PulseWidthSecondBunch.at(i).begin();
    std::vector<double>::iterator itSecondBunchelement;
    bool isFirst = true;
    for(;itSecondBunchwfm!=(PulseWidthSecondBunch.at(i).end());
	++itSecondBunchwfm){
      int j = 0;
      for(itSecondBunchelement = itSecondBunchwfm->begin();
  	  itSecondBunchelement != itSecondBunchwfm->end();
  	  itSecondBunchelement++){
  	if(!isFirst){
  	  avgPulseWidthSecondBunch.at(i).at(j) += *(itSecondBunchelement);
  	  avgPulseWidthSecondBunch.at(i).at(j) =
	    avgPulseWidthSecondBunch.at(i).at(j)/2;
  	}
  	else{
  	  avgPulseWidthSecondBunch.at(i).at(j) += *(itSecondBunchelement);
  	}
  	j++;
      }
      isFirst = false;
    }
  }


  for(int i=0; i<4; i++){
    std::vector< std::vector<double> >::iterator itHigherBuncheswfm =
      PulseWidthHigherBunches.at(i).begin();
    std::vector<double>::iterator itHigherBuncheselement;
    bool isFirst = true;
    for(;itHigherBuncheswfm!=(PulseWidthHigherBunches.at(i).end());
	++itHigherBuncheswfm){
      int j = 0;
      for(itHigherBuncheselement = itHigherBuncheswfm->begin();
  	  itHigherBuncheselement != itHigherBuncheswfm->end();
  	  itHigherBuncheselement++){
  	if(!isFirst){
  	  avgPulseWidthHigherBunches.at(i).at(j) += *(itHigherBuncheselement);
  	  avgPulseWidthHigherBunches.at(i).at(j) =
	    avgPulseWidthHigherBunches.at(i).at(j)/2;
  	}
  	else{
  	  avgPulseWidthHigherBunches.at(i).at(j) += *(itHigherBuncheselement);
  	}
  	j++;
      }
      isFirst = false;
    }
  }


  Double_t xVals[2520];
  Double_t AvgPWFirstBunchch2[2520];
  Double_t AvgPWSecondBunchch2[2520];
  Double_t AvgPWHigherBunchesch2[2520];
  Double_t AvgPWFirstBunchch3[2520];
  Double_t AvgPWSecondBunchch3[2520];
  Double_t AvgPWHigherBunchesch3[2520];

  for(int i=0; i<2520; i++){
    xVals[i] = i;
    AvgPWFirstBunchch2[i] = avgPulseWidthFirstBunch.at(2).at(i);
    AvgPWSecondBunchch2[i] = avgPulseWidthSecondBunch.at(2).at(i);
    AvgPWHigherBunchesch2[i] = avgPulseWidthHigherBunches.at(2).at(i);
    AvgPWFirstBunchch3[i] = avgPulseWidthFirstBunch.at(3).at(i);
    AvgPWSecondBunchch3[i] = avgPulseWidthSecondBunch.at(3).at(i);
    AvgPWHigherBunchesch3[i] = avgPulseWidthHigherBunches.at(3).at(i);
  }

  printf("Number of waveforms for average; Ch2, 1st bunch = %i\n",
	 PulseWidthFirstBunch.at(2).size());
  printf("Number of waveforms for average; Ch2, 2nd bunch = %i\n",
	 PulseWidthSecondBunch.at(2).size());
  printf("Number of waveforms for average; Ch2, other bunches = %i\n",
	 PulseWidthHigherBunches.at(2).size());
  printf("Number of waveforms for average; Ch3, 1st bunch = %i\n",
	 PulseWidthFirstBunch.at(3).size());
  printf("Number of waveforms for average; Ch3, 2nd bunch = %i\n",
	 PulseWidthSecondBunch.at(3).size());
  printf("Number of waveforms for average; Ch3, other bunches = %i\n",
	 PulseWidthHigherBunches.at(3).size());

  //Output the histograms first
  TCanvas* c1 = new TCanvas("QDC ch2",
			    "QDC histograms for the PulseWidth bunches, Ch2"
			    , 2);
  QDCFirstBunchCh2->GetXaxis()->SetTitle("QDC, Ch2");
  QDCFirstBunchCh2->Draw("E");
  QDCSecondBunchCh2->Draw("SAME");
  QDCHigherBunchesCh2->Draw("SAME");

  TCanvas* c2 = new TCanvas("QDC ch3",
			    "QDC histograms for the PulseWidth bunches, Ch3"
			    , 3);
  QDCFirstBunchCh3->GetXaxis()->SetTitle("QDC, Ch3");
  QDCFirstBunchCh3->Draw("E");
  QDCSecondBunchCh3->Draw("SAME");
  QDCHigherBunchesCh3->Draw("SAME");

  //output waveforms for the various PW bunches, each channel on different plot
  TCanvas* c3 = new TCanvas("Channel 2",
			    "Average waveforms for the PulseWidth bunches, Ch2"
			    , 4);
  TGraph* PWFirstBunchch2graph = new TGraph(2520, xVals, AvgPWFirstBunchch2);
  PWFirstBunchch2graph->GetXaxis()->SetTitle("Time (ns)");
  PWFirstBunchch2graph->SetLineStyle(1);
  PWFirstBunchch2graph->Draw("AL");
  TGraph* PWSecondBunchch2graph = new TGraph(2520, xVals, AvgPWSecondBunchch2);
  PWSecondBunchch2graph->SetLineStyle(2);
  PWSecondBunchch2graph->Draw("SAME");
  TGraph* PWHigherBunchesch2graph =
    new TGraph(2520, xVals, AvgPWHigherBunchesch2);
  PWHigherBunchesch2graph->SetLineStyle(4);
  PWHigherBunchesch2graph->Draw("SAME");


  TCanvas* c4 = new TCanvas("Channel 3",
			    "Average waveforms for the PulseWidth bunches, Ch3",
			    5);
  TGraph* PWFirstBunchch3graph = new TGraph(2520, xVals, AvgPWFirstBunchch3);
  PWFirstBunchch3graph->GetXaxis()->SetTitle("Time (ns)");
  PWFirstBunchch3graph->SetLineStyle(1);
  PWFirstBunchch3graph->Draw("AL");
  TGraph* PWSecondBunchch3graph = new TGraph(2520, xVals, AvgPWSecondBunchch3);
  PWSecondBunchch3graph->SetLineStyle(2);
  PWSecondBunchch3graph->Draw("SAME");
  TGraph* PWHigherBunchesch3graph =
    new TGraph(2520, xVals, AvgPWHigherBunchesch3);
  PWHigherBunchesch3graph->SetLineStyle(4);
  PWHigherBunchesch3graph->Draw("SAME");


}

