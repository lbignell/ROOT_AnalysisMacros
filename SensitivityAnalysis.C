//This macro will analyse the raw data, extracting the pulse information.
//(BaselineRemovalv5.C) with various algorithm parameters. Re-analysing with
//various parameters allows a sensitivity analysis of the results.
//
//Calculating the uncertainty of the input parameters is tricky.
//
#include "TROOT.h"
#include "TString.h"
#include "TFile.h"
#include <fstream>
#include "TSystem.h"


void SensitivityAnalysis(){
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
  ampl.push_back(10);
  ampl.push_back(10);
  ampl.push_back(50);
  ampl.push_back(100);
  
  for(int i = 0; i<4; i++){
    avg_pre.push_back(4);
    avg_post.push_back(4);
    trig_pre.push_back(13);
  }
  
  trig_post.push_back(10);
  trig_post.push_back(10);
  trig_post.push_back(10);
  trig_post.push_back(60);
  
  UTrigLen.push_back(21);
  UTrigLen.push_back(21);
  UTrigLen.push_back(21);
  UTrigLen.push_back(71);
  UTrigLen.at(0) = trig_post.at(0) + trig_pre.at(0) + 1;
  UTrigLen.at(1) = trig_post.at(1) + trig_pre.at(1) + 1;
  UTrigLen.at(2) = trig_post.at(2) + trig_pre.at(2) + 1;
  UTrigLen.at(3) = trig_post.at(3) + trig_pre.at(3) + 1;

  //Initialise the iteration counter
  int runnum = 0;

  fstream logfile;
  logfile.open("runlog.txt", ios_base::in | ios_base::out | ios_base::trunc);

  //Start of runs
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

  TString command1 = TString::Format(".! mkdir AllData_run%i", runnum);
  TString command2 = TString::Format(".! mv Median_* AllData_run%i", runnum);
  TString command3 = TString::Format(".! mv AvgWfms_* AllData_run%i", runnum);
  gROOT->ProcessLine(command1);
  gROOT->ProcessLine(command2);
  gROOT->ProcessLine(command3);

  
  //Loop again over all values, this time going until you reach 125.
  while(runnum<15){
    //close and re-open params.txt to clear contents.
    file.close();
    file.open("params.txt", ios_base::in | ios_base::out | ios_base::trunc);

    runnum = runnum + 1;

    ampl.at(0) = runnum + 10;
    ampl.at(1) = runnum + 10;

    //Adjust algorithm parameters
    for(int i = 0; i<4; i++){
      
      file << BL.at(i) << "\t" << ampl.at(i) << "\t" << avg_pre.at(i) << "\t"
	   << avg_post.at(i) << "\t"<< trig_pre.at(i) << "\t" << trig_post.at(i)
	   << "\t" << UTrigLen.at(i) << endl;
      
      logfile << runnum << "\t"
	      << BL.at(i) << "\t" << ampl.at(i) << "\t" << avg_pre.at(i) << "\t"
	      << avg_post.at(i) <<"\t"<< trig_pre.at(i) << "\t"<<trig_post.at(i)
	      << "\t" << UTrigLen.at(i) << endl;
    }

    //Run Algorithm
    gROOT->ProcessLine(".x ProcessAllDataMyAlgo.C");

    //Save, etc.
    command1 = TString::Format(".! mkdir AllData_run%i", runnum);
    command2 = TString::Format(".! mv Median_* AllData_run%i", runnum);
    command3 = TString::Format(".! mv AvgWfms_* AllData_run%i", runnum);
    gROOT->ProcessLine(command1);
    gROOT->ProcessLine(command2);
    gROOT->ProcessLine(command3);

  }

  //  trig_post.at(0) = 10;
  //trig_post.at(1) = 10;
  
  //UTrigLen.at(0) = trig_post.at(0) + trig_pre.at(0) + 1;
  //UTrigLen.at(1) = trig_post.at(0) + trig_pre.at(0) + 1;


  //while(runnum<85){
    //close and re-open params.txt to clear contents.
    //file.close();
    //file.open("params.txt", ios_base::in | ios_base::out | ios_base::trunc);

    //runnum = runnum + 1;

  //    trig_pre.at(0) = runnum-60;
  //trig_pre.at(1) = runnum-60;

  //UTrigLen.at(0) = trig_post.at(0) + trig_pre.at(0) + 1;
  //UTrigLen.at(1) = trig_post.at(0) + trig_pre.at(0) + 1;
  
    //Adjust algorithm parameters
    //for(int i = 0; i<4; i++){
      
  //file << BL.at(i) << "\t" << ampl.at(i) << "\t" << avg_pre.at(i) << "\t"
  //	   << avg_post.at(i) << "\t"<< trig_pre.at(i) << "\t" << trig_post.at(i)
  //	   << "\t" << UTrigLen.at(i) << endl;
  //  
  //  logfile << runnum << "\t"
  //	      << BL.at(i) << "\t" << ampl.at(i) << "\t" << avg_pre.at(i) << "\t"
  //	      << avg_post.at(i) <<"\t"<< trig_pre.at(i) << "\t"<<trig_post.at(i)
  //	      << "\t" << UTrigLen.at(i) << endl;
  //}

    //Run Algorithm
    //gROOT->ProcessLine(".x ProcessAllDataMyAlgo.C");

    //Save, etc.
    //command1 = TString::Format(".! mkdir AllData_run%i", runnum);
    //command2 = TString::Format(".! mv Median_* AllData_run%i", runnum);
    //command3 = TString::Format(".! mv AvgWfms_* AllData_run%i", runnum);
    //gROOT->ProcessLine(command1);
    //gROOT->ProcessLine(command2);
    //gROOT->ProcessLine(command3);

  //}
}
