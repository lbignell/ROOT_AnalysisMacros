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
#include "TRandom.h"
#include <math.h>

void ParameterSampling(){
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
  //gROOT->ProcessLine(".x ProcessAllDataMyAlgo.C");

  TString command1 = TString::Format(".! mkdir AllData_run%i", runnum);
  TString command2 = TString::Format(".! mv RobustMean_* AllData_run%i", runnum);
  TString command3 = TString::Format(".! mv AvgWfms_* AllData_run%i", runnum);
  //gROOT->ProcessLine(command1);
  //gROOT->ProcessLine(command2);
  //gROOT->ProcessLine(command3);
  int RanNum = 0;
  bool HazPassed = false;
  time_t theTime = time(NULL);
  //Use system time as the seed.
  TRandom* GenRand = new TRandom(theTime);
  
  for(int j = 0; j<110; j++){
    //close and re-open params.txt to clear contents.
    file.close();
    file.open("params.txt", ios_base::in | ios_base::out | ios_base::trunc);

    runnum = runnum + 3;

    //Monte Carlo sample the parameters here...
    //Amplitude for ch0 and 1 is a Gaussian centred at 18, with a stdev of 2.
    //Values less than 15 are rejected (resampled).
    HazPassed = false;
    while(!HazPassed){
      RanNum = int(round(GenRand->Gaus(18,2)));
      if(RanNum>14){HazPassed = true;}
    }
    
    ampl.at(0) = RanNum;

    HazPassed = false;
    while(!HazPassed){
      RanNum = int(round(GenRand->Gaus(18,2)));
      if(RanNum>14){HazPassed = true;}
    }

    ampl.at(1) = RanNum;

    //trig_pre is Gaussian centred at 13, with std dev of 3, truncated at 5 and
    //20.
    HazPassed = false;
    while(!HazPassed){
      RanNum = int(round(GenRand->Gaus(13,3)));
      if((RanNum>5)&&(RanNum<21)){HazPassed = true;}
    }

    trig_pre.at(0) = RanNum;

    HazPassed = false;
    while(!HazPassed){
      RanNum = int(round(GenRand->Gaus(13,3)));
      if((RanNum>5)&&(RanNum<21)){HazPassed = true;}
    }

    trig_pre.at(1) = RanNum;

    //trig_post is Gaussian centred at 20, std dev of 2, truncated at 13 and 22.
    HazPassed = false;
    while(!HazPassed){
      RanNum = int(round(GenRand->Gaus(20,2)));
      if((RanNum>12)&&(RanNum<23)){HazPassed = true;}
    }

    trig_post.at(0) = RanNum;

    HazPassed = false;
    while(!HazPassed){
      RanNum = int(round(GenRand->Gaus(20,2)));
      if((RanNum>12)&&(RanNum<23)){HazPassed = true;}
    }

    trig_post.at(1) = RanNum;

    //Finally, set UnTrigLen
    UTrigLen.at(0) = trig_post.at(0) + trig_pre.at(0) + 1;
    UTrigLen.at(1) = trig_post.at(1) + trig_pre.at(1) + 1;
    UTrigLen.at(2) = trig_post.at(2) + trig_pre.at(2) + 1;
    UTrigLen.at(3) = trig_post.at(3) + trig_pre.at(3) + 1;


    //Adjust algorithm parameters in the file
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
    command2 = TString::Format(".! mv RobustMean_* AllData_run%i", runnum);
    command3 = TString::Format(".! mv AvgWfms_* AllData_run%i", runnum);
    gROOT->ProcessLine(command1);
    gROOT->ProcessLine(command2);
    gROOT->ProcessLine(command3);

  }

}
