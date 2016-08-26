//This program will scan through a directory (passed as a parameter), check the
//header of every raw .root files saved by the FADC DAQ, put it in a TChain, and
//pass to another program for analysis (+saving?).
#include "TROOT.h"
#include "TChain.h"
#include "tinydir.h"
#include "TBranch.h"
#include "TLeaf.h"
#include <fstream>
#include "TFile.h"
#include <iostream>
#include "TString.h"
#include "TSystem.h"

using namespace std;

void* GetPointerToValue(TBranch* thisBranch, int entry,
				const char* name){
  thisBranch->GetEntry(entry);
  TLeaf* theLeaf = thisBranch->GetLeaf(name);
  return theLeaf->GetValuePointer();
}

void CollateRawDatav2(string path, Int_t StartNum, Int_t EndNum){

  string theFile;
  tinydir_dir dir;
  tinydir_file file;
  TTree* theTree;
  TBranch* theBranch;
  Int_t* dummy;

  //Need to sort this one out.
  TFile* Collected; //= new TFile("CollectedMedianData.root", "RECREATE");

  TChain* theChain = new TChain("FADCData");

  tinydir_open(&dir, path.c_str());//expects const char* path as second arg.
  printf(path.c_str());
  printf("\n");
  while (dir.has_next)
    {
      //printf("Getting files in this directory... \n");
      
      tinydir_readfile(&dir, &file);
      
      //printf("%s", file.name);
      
      if (file.is_dir)
	{
	  printf("/");
	}
      //printf("\n");
      
      //Insert my code here:
      if(strstr(file.name, "rootoutputfile")){
	//This is where the file is opened, and checked to see if it's the run
	//I want...
	theFile = path + file.name;
	//f = TFile::Open(theFile.c_str());
	TFile* f = TFile::Open(file.name);
	theTree = (TTree*)f->Get("Header");
	//Get the sample type and run type and see if they match.
	theBranch = (TBranch*)theTree->GetBranch("RunNumber");
	//I'll assume below that the RunType is constant during the run, and
	//just pick the 0th entry.
	if(theTree->GetEntries()!=0){
	  dummy = (Int_t*)
	    (GetPointerToValue(theBranch, 0, "RunNumber"));
	}
	else{ dummy = NULL;}
	//First check tree was not totally empty
	if(dummy){
	  if((*dummy<=EndNum)&&(StartNum<=*dummy)){
	    //OK, we can add this file to the chain.
	    theChain->Add(theFile.c_str());
	    cout << "Adding file " << file.name << " to chain." << endl;
	  }
	}
	else{
	  cout << "Warning: empty file? Filename = "
	       << file.name << endl;
	}

	f->Close();
	delete f;
      }

       
      tinydir_next(&dir);
      
    }

  tinydir_close(&dir);

  //Now that I've searched the whole directory, send the TChain off to be
  //processed (cumsum is the main thing to do).
  theChain->Process("SimpleSumIntegralv2.C+");//INSERT NAME OF ANALYSIS FILE HERE.

  //WRITE SOME CODE TO CHANGE THE FILE NAME OF THE ANALYSIS OUTPUT SO THAT IT
  //DOESN'T GET OVERWRITTEN.
  //TString command = TString::Format("mv SimpleIntegral.root Data%i_%i.root",
  //		    StartNum, EndNum);
  //gSystem->Exec(command);
  //This has been moved to ProcessData.C
}
