//This function will extract a robust estimate of the mean for a given file.
#include "TROOT.h"
#include "TH1.h"
#include "TMath.h"
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include <iostream>
#include "TString.h"
#include <fstream>

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


void GetTrimmed(double tfrac, TH1D* theHist, TH1D* &TrimHist, TH1D* &WindHist,
		TH1D* &BisqHist, double &BisqMean){
  //Calculate the trimmed mean. First I need to eliminate the data that are in
  //the trimmed part.

  double totIntegral = theHist->Integral();

  TH1D* DummyHist = (TH1D*)theHist->Clone("TrimHist");

  TrimHist = DummyHist;

  double LowLimit = totIntegral*(tfrac/2);
  double UpLimit = totIntegral*(1-(tfrac/2));

  int LowBin = 0;
  int UpBin = 0;
  int MeanBin = 0;

  double thisIntegral = 0;

  bool HazPassedLower = false;
  bool HazPassedUpper = false;
  bool HazFoundMean = false;

  for(int i = 0; i<theHist->GetNbinsX(); i++){

    thisIntegral = theHist->Integral(0, i);

    //Check the lower limit
    if(!HazPassedLower){
      if(thisIntegral>LowLimit){
      //Set the lower limit for the truncation.
      LowBin = i;
      HazPassedLower = true;
      }
      else{ 
	TrimHist->SetBinContent(i, 0.);
      }
    }

    //Check the upper limit
    if((UpLimit<thisIntegral)&&(!HazPassedUpper)){
      UpBin = i;
      HazPassedUpper = true;
    }
    else if(HazPassedUpper){
      TrimHist->SetBinContent(i, 0.);
    }

    //Find the mean bin.
    if((!HazFoundMean)&&((thisIntegral/totIntegral)>0.5)){
      MeanBin = i;
      HazFoundMean = true;
    }
  }

  //Calculate Windsorized mean
  DummyHist = (TH1D*)TrimHist->Clone("WindHist");
  WindHist = DummyHist;
  //LowLimit is just the number of counts that constitutes the trimmed fraction.
  WindHist->AddBinContent(LowBin, floor(LowLimit));
  WindHist->AddBinContent(UpBin, floor(LowLimit));

  //cout << "LowBin = " << LowBin << endl
  //   << "MeanBin = " << MeanBin << endl
  //   << "UpBin = " << UpBin << endl;

  //This part of the code is broken, hence the commenting. I think I could fix
  //it, but I have since read that the Bisquare mean is not a Maximum Likelyhood
  //Estimator.
  //////////////////////////////////////////////////////////////////////////
  //Calculate Bisquare mean
  //DummyHist = (TH1D*)TrimHist->Clone("BisqHist");
  //BisqHist = DummyHist;
  //TH1D* WeightHist = (TH1D*)BisqHist->Clone("WeightHist");

  //for(int i = LowBin-1; i<UpBin+1; i++){
    //Calculate the weights, accumulate the mean.
    //if(i < MeanBin){
  //WeightHist->SetBinContent(i, -pow(1 - pow((i - MeanBin)/LowBin, 2), 2));
  //}
  //else{
  //  WeightHist->SetBinContent(i, pow(1 - pow((i - MeanBin)/UpBin, 2), 2));
  //}
  //}

  //DummyHist->Multiply(BisqHist, WeightHist);
  //BisqMean = (DummyHist->Integral())/(WeightHist->Integral());

}


void MeanRobust(const char* filename, double TrimFraction){

  //Open up the file and grab the tree
  TFile *f = TFile::Open(filename);
  TTree* ResultsTree = (TTree*)f->Get("Results");

  TH1D* hnewLEDA;
  TCanvas* c2 = new TCanvas("LED ON, PMT A", filename, 2);
  //Get LED ON events for PMT A
  Long64_t nentriesLEDA = ResultsTree->Draw("QDCsimple[0]>>hnewLEDA(10000, 0., 0.)", "(130000<QDCsimple[2])&&(QDCsimple[2]<160000)&&(1000<TimeSimple[0])&&(TimeSimple[0]<1080)");
  hnewLEDA = (TH1D*)gDirectory->Get("hnewLEDA");
  Double_t medianLEDA = median1(hnewLEDA);
  Double_t meanLEDA = hnewLEDA->GetMean();
  Double_t StdErrLEDA = hnewLEDA->GetRMS()/sqrt(nentriesLEDA);

  //Calculate trimmed mean and Windsorized mean
  TH1D* TrimmedLEDA;
  TH1D* WindsorizedLEDA;
  TH1D* BisquareLEDA;
  Double_t meanBisqLEDA;
  GetTrimmed(TrimFraction, hnewLEDA, TrimmedLEDA, WindsorizedLEDA,
	     BisquareLEDA, meanBisqLEDA);
  Double_t meanTrimLEDA = TrimmedLEDA->GetMean();
  Double_t meanWindsorLEDA = WindsorizedLEDA->GetMean();

  //StdErr of Windsorized and Trimmed mean.
  Double_t stdErrWindsorLEDA = WindsorizedLEDA->GetRMS()/sqrt(nentriesLEDA);
  Double_t stdErrTrimLEDA = stdErrWindsorLEDA/(1-TrimFraction);

  //  cout << "Median = " << medianLEDA << endl
  //   << "Mean = " << meanLEDA << " +/- " 
  //   << hnewLEDA->GetRMS()/sqrt(nentriesLEDA) << endl
  //   << "Trimmed Mean = " << meanTrimLEDA << " +/- "
  //   << stdErrTrimLEDA << endl
  //   << "Windsorized Mean = " << meanWindsorLEDA << " +/- "
  //   << stdErrWindsorLEDA << endl
  //   << "Bisquare Mean = " << meanBisqLEDA << endl;

  TH1D* hnewLEDB;
  TCanvas* c3 = new TCanvas("LED ON, PMT B", filename, 3);
  //Get LED ON events for PMT B
  Long64_t nentriesLEDB = ResultsTree->Draw("QDCsimple[1]>>hnewLEDB(10000, 0., 0.)", "(130000<QDCsimple[2])&&(QDCsimple[2]<160000)&&(1000<TimeSimple[1])&&(TimeSimple[1]<1080)");
  hnewLEDB = (TH1D*)gDirectory->Get("hnewLEDB");
  Double_t medianLEDB = median1(hnewLEDB);
  Double_t meanLEDB = hnewLEDB->GetMean(); 
  Double_t StdErrLEDB = hnewLEDB->GetRMS()/sqrt(nentriesLEDB);

  TH1D* TrimmedLEDB;
  TH1D* WindsorizedLEDB;
  TH1D* BisquareLEDB;
  Double_t meanBisqLEDB;
  GetTrimmed(TrimFraction, hnewLEDB, TrimmedLEDB, WindsorizedLEDB,
	     BisquareLEDB, meanBisqLEDB);
  //GetTrimmed(TrimFraction, hnewLEDB, TrimmedLEDB, WindsorizedLEDB);
  Double_t meanTrimLEDB = TrimmedLEDB->GetMean();
  Double_t meanWindsorLEDB = WindsorizedLEDB->GetMean();

  //StdErr of Windsorized and Trimmed mean are actually the same.
  Double_t stdErrWindsorLEDB = WindsorizedLEDB->GetRMS()/sqrt(nentriesLEDB);
  Double_t stdErrTrimLEDB = stdErrWindsorLEDB/(1-TrimFraction);

  TH1D* hnewBeamA;
  TCanvas* c4 = new TCanvas("LED ON, Beam A", filename, 4);
  //Get BeamON events for PMT A
  Long64_t nentriesBeamA = ResultsTree->Draw("QDCsimple[0]>>hnewBeamA(10000, 0., 0.)", "(QDCsimple[0]>-100)&&(QDCsimple[0]<10000)&&(325<TimeSimple[0])&&(TimeSimple[0]<375)&&(390<TimeSimple[2])&&(TimeSimple[2]<450)&&(390<TimeSimple[3])&&(TimeSimple[3]<450)");
  hnewBeamA = (TH1D*)gDirectory->Get("hnewBeamA");
  Double_t medianBeamA = median1(hnewBeamA); 
  Double_t meanBeamA = hnewBeamA->GetMean();
  Double_t StdErrBeamA = hnewBeamA->GetRMS()/sqrt(nentriesBeamA);

  TH1D* TrimmedBeamA;
  TH1D* WindsorizedBeamA;
  TH1D* BisquareBeamA;
  Double_t meanBisqBeamA;
  GetTrimmed(TrimFraction, hnewBeamA, TrimmedBeamA, WindsorizedBeamA,
	     BisquareBeamA, meanBisqBeamA);
  //  GetTrimmed(TrimFraction, hnewBeamA, TrimmedBeamA, WindsorizedBeamA);
  Double_t meanTrimBeamA = TrimmedBeamA->GetMean();
  Double_t meanWindsorBeamA = WindsorizedBeamA->GetMean();

  //StdErr of Windsorized and Trimmed mean are actually the same.
  Double_t stdErrWindsorBeamA = WindsorizedBeamA->GetRMS()/sqrt(nentriesBeamA);
  Double_t stdErrTrimBeamA = stdErrWindsorBeamA/(1-TrimFraction);


  TH1D* hnewBeamB;
  TCanvas* c5 = new TCanvas("LED ON, Beam B", filename, 5);
  //Get BeamON events for PMT B
  Long64_t nentriesBeamB = ResultsTree->Draw("QDCsimple[1]>>hnewBeamB(10000, 0., 0.)", "(QDCsimple[1]>-100)&&(QDCsimple[1]<10000)&&(325<TimeSimple[1])&&(TimeSimple[1]<375)&&(390<TimeSimple[2])&&(TimeSimple[2]<450)&&(390<TimeSimple[3])&&(TimeSimple[3]<450)");
  hnewBeamB = (TH1D*)gDirectory->Get("hnewBeamB");
  Double_t medianBeamB = median1(hnewBeamB); 
  Double_t meanBeamB = hnewBeamB->GetMean();
  Double_t StdErrBeamB = hnewBeamB->GetRMS()/sqrt(nentriesBeamB);

  TH1D* TrimmedBeamB;
  TH1D* WindsorizedBeamB;
  TH1D* BisquareBeamB;
  Double_t meanBisqBeamB;
  GetTrimmed(TrimFraction, hnewBeamB, TrimmedBeamB, WindsorizedBeamB,
	     BisquareBeamB, meanBisqBeamB);
  //  GetTrimmed(TrimFraction, hnewBeamB, TrimmedBeamB, WindsorizedBeamB);
  Double_t meanTrimBeamB = TrimmedBeamB->GetMean();
  Double_t meanWindsorBeamB = WindsorizedBeamB->GetMean();

  //StdErr of Windsorized and Trimmed mean are actually the same.
  Double_t stdErrWindsorBeamB = WindsorizedBeamB->GetRMS()/sqrt(nentriesBeamB);
  Double_t stdErrTrimBeamB = stdErrWindsorBeamB/(1-TrimFraction);


  cout<<"---------------------------------------------------------------"<<endl;
  cout << "Median Data for file: ;\t" << filename << endl;
  cout <<";\tPMTA, LED ;\t'+/- ;\tPMTB, LED ;\t'+/- ;\tPMTA, Beam ;\t'+/- ;\tPMTB, Beam ;\t'+/-" << endl;
  cout << "Median ;\t" << medianLEDA << ";\t ;\t" << medianLEDB << ";\t ;\t"
       << medianBeamA << ";\t ;\t" << medianBeamB << endl;
  cout << "Mean ;\t" <<   meanLEDA << ";\t" << StdErrLEDA << ";\t" 
       << meanLEDB << ";\t" << StdErrLEDB << ";\t"
       << meanBeamA << ";\t" << StdErrBeamA << ";\t"
       << meanBeamB << ";\t" << StdErrBeamB << endl;
  cout << "Trim fraction: ;\t" << TrimFraction << endl;
  cout << "Trimmed Mean ;\t" << meanTrimLEDA << ";\t" << stdErrTrimLEDA << ";\t"
       << meanTrimLEDB << ";\t" << stdErrTrimLEDB << ";\t"
       << meanTrimBeamA << ";\t" << stdErrTrimBeamA << ";\t"
       << meanTrimBeamB << ";\t" << stdErrTrimBeamB << endl;
  cout << "Windsorized Mean: ;\t" << meanWindsorLEDA << ";\t" << stdErrWindsorLEDA
       << ";\t" << meanWindsorLEDB << ";\t" << stdErrWindsorLEDB << ";\t"
       << meanWindsorBeamA << ";\t" << stdErrWindsorBeamA << ";\t"
       << meanWindsorBeamB << ";\t" << stdErrWindsorBeamB << endl;
  //cout << "Bisquare Mean: ;\t" << meanBisqLEDA << ";\t ;\t" << meanBisqLEDB
  //   << ";\t ;\t" << meanBisqBeamA << ";\t ;\t" << meanBisqBeamB << endl;
  cout << "# events ;\t" << nentriesLEDA << ";\t ;\t" << nentriesLEDB
       << ";\t ;\t" << nentriesBeamA << ";\t ;\t" << nentriesBeamB << endl;
  cout<<"---------------------------------------------------------------"<<endl;

  //Also save as a root file.
  TString prefix = "RobustMean";
  TString ROOTfname = prefix + filename;
  TFile* theROOTfile = new TFile(ROOTfname, "RECREATE");
  TTree* MedianTree = new TTree("Results",
				"The median, robust mean, and # events");
  MedianTree->Branch("MedianLEDA", &medianLEDA, "MedianLEDA/D");
  MedianTree->Branch("MedianLEDB", &medianLEDB, "MedianLEDB/D");
  MedianTree->Branch("MedianBeamA", &medianBeamA, "MedianBeamA/D");
  MedianTree->Branch("MedianBeamB", &medianBeamB, "MedianBeamB/D");
  MedianTree->Branch("MeanLEDA", &meanLEDA, "MeanLEDA/D");
  MedianTree->Branch("MeanLEDB", &meanLEDB, "MeanLEDB/D");
  MedianTree->Branch("MeanBeamA", &meanBeamA, "MeanBeamA/D");
  MedianTree->Branch("MeanBeamB", &meanBeamB, "MeanBeamB/D");
  MedianTree->Branch("NentriesLEDA", &nentriesLEDA, "NentriesLEDA/L");
  MedianTree->Branch("NentriesLEDB", &nentriesLEDB, "NentriesLEDB/L");
  MedianTree->Branch("NentriesBeamA", &nentriesBeamA, "NentriesBeamA/L");
  MedianTree->Branch("NentriesBeamB", &nentriesBeamB, "NentriesBeamB/L");
  MedianTree->Branch("TrimMeanLEDA",  &meanTrimLEDA, "MeanTrimLEDA/D");
  MedianTree->Branch("TrimMeanLEDB",  &meanTrimLEDB, "MeanTrimLEDB/D");
  MedianTree->Branch("TrimMeanBeamA",  &meanTrimBeamA, "MeanTrimBeamA/D");
  MedianTree->Branch("TrimMeanBeamB",  &meanTrimBeamB, "MeanTrimBeamB/D");
  MedianTree->Branch("WindMeanLEDA",  &meanWindsorLEDA, "MeanWindLEDA/D");
  MedianTree->Branch("WindMeanLEDB",  &meanWindsorLEDB, "MeanWindLEDB/D");
  MedianTree->Branch("WindMeanBeamA",  &meanWindsorBeamA, "MeanWindBeamA/D");
  MedianTree->Branch("WindMeanBeamB",  &meanWindsorBeamB, "MeanWindBeamB/D");
  MedianTree->Branch("TrimFraction", &TrimFraction, "TrimFraction/D");

  //Get the current parameter values from the parameter value file.
  //The nominal default values are commented.
  double avg_baseline[4];// = {8133, 8166, 8165, 8000};
  double max_amplitude[4];// = {13, 17, 50, 100};
  int avg_pre_samples[4];// = {4, 4, 4, 4};
  int avg_post_samples[4];// = {4, 4, 4, 4};
  int trig_pre_samples[4];// = {10, 10, 10, 10};
  int trig_post_samples[4];// = {10, 10, 10, 60};
  int UnTrigSettleLength[4];// = {21, 21, 21, 71};

  fstream file;
  file.open("params.txt", ios_base::in | ios_base::out | ios_base::app);

  //Fill the algorithm parameter arrays
  for(int i = 0; i<4; i++){
    //Read the next line in to the variables.       
    file >> avg_baseline[i] >> max_amplitude[i] >> avg_pre_samples[i]
	 >> avg_post_samples[i] >> trig_pre_samples[i]
	 >> trig_post_samples[i] >> UnTrigSettleLength[i];
  }

  MedianTree->Branch("avg_baseline", &avg_baseline, "avg_baseline[4]/D");
  MedianTree->Branch("max_amplitude", &max_amplitude, "max_amplitude[4]/D");
  MedianTree->Branch("avg_pre_samples", &avg_pre_samples,
		     "avg_pre_samples[4]/I");
  MedianTree->Branch("avg_post_samples", &avg_post_samples,
		     "avg_post_samples[4]/I");
  MedianTree->Branch("trig_pre_samples", &trig_pre_samples,
		     "trig_pre_samples[4]/I");
  MedianTree->Branch("trig_post_samples", &trig_post_samples,
		     "trig_post_samples[4]/I");
  MedianTree->Branch("UnTrigSettleLength", &UnTrigSettleLength,
		     "UnTrigSettleLength[4]/I");

  MedianTree->Fill();
  MedianTree->Write();
  theROOTfile->Close();
  c2->Close();
  c3->Close();
  c4->Close();
  c5->Close();
  f->Close();
 

}
