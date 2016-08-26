//This function runs the algorithm, with the current parameter settings.
//Then it runs the median function. The AvgWfm and Median... results are saved.
#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TSystem.h"

void RunAlgorithm(int startNum, int stopNum){
  TString s = TString::Format("echo Running data from %i to %i",
			      startNum, stopNum);
  gSystem->Exec(s);
  gSystem->Exec("echo New Data set! The time is...");
  gSystem->Exec("date");

  TFile* f1 = new TFile(TString::Format("../SimpleIntegral_%i_%i.root",
					startNum, stopNum));
  TTree* Results = (TTree*)f1->Get("Results");

  Results->Process("BaselineRemovalv5.C+");

  TString command = TString::Format("mv AvgWfms.root AvgWfms_%i_%i.root",
				    startNum, stopNum);
  gSystem->Exec(command);

  TString command1 = TString::Format("mv BLsubtractedv5.root BLsub_%i_%i.root",
				     startNum, stopNum);
  gSystem->Exec(command1);

  TString command1a =
    TString::Format(".x MeanRobust.C+(\"BLsub_%i_%i.root\", 0.05)",
		    startNum, stopNum);
  gROOT->ProcessLine(command1a);

  TString command2 =
    TString::Format("mv RobustMeanBLsub* RobustMean_%i_%i.root",
		    startNum, stopNum);
  gSystem->Exec(command2);

  f1->Close();

}
