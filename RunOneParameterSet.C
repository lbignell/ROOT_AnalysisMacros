//This macro will analyse the raw data, extracting the pulse information.
//(BaselineRemovalv5.C) with a single set of algorithm parameters. The data
//files will be saved to a single folder.
//
#include "TROOT.h"
#include "TString.h"
#include "TFile.h"
#include <fstream>
#include "TSystem.h"


void RunOneParameterSet(){
  vector<double> BL;
  vector<double> ampl;
  vector<int> avg_pre;
  vector<int> avg_post;
  vector<int> trig_pre;
  vector<int> trig_post;
  vector<int> UTrigLen;
  
  //Not varying BL
  BL.push_back(8133);
  BL.push_back(8166);
  BL.push_back(8165);
  BL.push_back(8000);
  
  //Initialise vector values here to expected values.
  ampl.push_back(18);
  ampl.push_back(18);
  ampl.push_back(50);
  ampl.push_back(100);
  
  for(int i = 0; i<4; i++){
    avg_pre.push_back(4);
    avg_post.push_back(4);
    trig_pre.push_back(13);
  }
  
  trig_post.push_back(20);
  trig_post.push_back(20);
  trig_post.push_back(20);
  trig_post.push_back(60);
  
  UTrigLen.push_back(121);
  UTrigLen.push_back(121);
  UTrigLen.push_back(121);
  UTrigLen.push_back(171);
  UTrigLen.at(0) = trig_post.at(0) + trig_pre.at(0) + 1;
  UTrigLen.at(1) = trig_post.at(1) + trig_pre.at(1) + 1;
  UTrigLen.at(2) = trig_post.at(2) + trig_pre.at(2) + 1;
  UTrigLen.at(3) = trig_post.at(3) + trig_pre.at(3) + 1;

  fstream logfile;
  logfile.open("runlog.txt", ios_base::in | ios_base::out | ios_base::trunc);

  int runnum = 0;

  ////////////////////////////////////////////////////////////////////////////
  //I *TRIED* setting the algo parameters nicely using vectors, but I ROOT did
  //not want to pass arguments to my TSelector-derived class. My workaround is
  //to write to a file here...
  fstream file;
  file.open("params.txt", ios_base::in | ios_base::out | ios_base::trunc);
  //The format of the file will be to have parameters for one channel tab
  //separated on a single line, with an endl between channels.
  for(int i = 0; i<4; i++){
    
    file << BL.at(i) << "\t" << ampl.at(i) << "\t" << avg_pre.at(i) << "\t"
	 << avg_post.at(i) << "\t" << trig_pre.at(i) << "\t" << trig_post.at(i)
	 << "\t" << UTrigLen.at(i) << endl;
    
    logfile << runnum << "\t"
	    << BL.at(i) << "\t" << ampl.at(i) << "\t" << avg_pre.at(i) << "\t"
	    << avg_post.at(i) << "\t" << trig_pre.at(i) << "\t"<<trig_post.at(i)
	    << "\t" << UTrigLen.at(i) << endl;

  }

  
  //Vary parameter.
  //Need to put the calls to run over the data sets into another script...
  //Then call script here.
  gROOT->ProcessLine(".x ProcessAllDataMyAlgo.C");

  gROOT->ProcessLine(".! mkdir AnalysedData");
  gROOT->ProcessLine(".! mv Median_* AnalysedData_max_ampl_20");
  gROOT->ProcessLine(".! mv BLsub_* AnalysedData_max_ampl_20");
  gROOT->ProcessLine(".! mv AvgWfms_* AnalysedData_max_ampl_20");

}
