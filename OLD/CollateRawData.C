//This program will scan through a directory (passed as a parameter), check the
//header of every raw .root files saved by the FADC DAQ, put it in a TChain, and
//pass to another program for analysis (+saving?).
#include "TROOT.h"
#include "TChain.h"
#include "tinydir.h"
#include "TBranch.h"
#include "TLeaf.h"

using namespace std;


void* GetPointerToValue(TBranch* theBranch, int entry,
				const char* name){
  theBranch->GetEntry(entry);
  TLeaf* theLeaf = theBranch->GetLeaf(name);
  return theLeaf->GetValuePointer();
}

void CollateRawData(string path, Int_t RunType, Int_t SampleType){

  string theFile;
  tinydir_dir dir;
  tinydir_file file;
  TFile* f;
  TTree* theTree;
  TBranch* theBranch;
  Int_t* dummy;

  //Need to sort this one out.
  TFile* Collected; //= new TFile("CollectedMedianData.root", "RECREATE");


  TChain* theChain = new TChain("Results");

  tinydir_open(&dir, path.c_str());//expects const char* path as second arg.
  printf(path.c_str());
  printf("\n");
  while (dir.has_next)
    {
      printf("Getting files in this directory... \n");
      
      tinydir_readfile(&dir, &file);
      
      printf("%s", file.name);
      
      if (file.is_dir)
	{
	  printf("/");
	}
      printf("\n");
      
      //Insert my code here:
      if(strstr(file.name, "rootoutputfile")){
	//This is where the file is opened, and checked to see if it's the run
	//I want...
	theFile = path + "/" + file.name;
	f = TFile::Open(theFile);
	theTree = (TTree*)f->Get("Header");

	//Get the sample type and run type and see if they match.
	theBranch = (TBranch*)theTree->GetBranch("RunType");
	//I'll assume below that the RunType is constant during the run, and
	//just pick the 0th entry.
	dummy = (Int_t*)
	  (GetPointerToValue(theBranch, 0, theBranch->GetName()));
	
	//First check tree was not totally empty
	if(dummy){
	  if((*dummy)==RunType){
	    //now check the sample type.
	    theBranch = (TBranch*)theTree->GetBranch("SampleType");
	    dummy = (Int_t*)
	      (GetPointerToValue(theBranch, 0, theBranch->GetName()));
	    if(dummy){
	      if((*dummy)==SampleType){
		//OK, we can add this file to the chain.
		theChain->Add(theFile.c_str());
	      }
	    }
	  }
	}
	else{
	  cout << "Warning: empty file? Filename = "
	       << file.name << endl;
	}


      }  
      
      tinydir_next(&dir);
      
    }

  tinydir_close(&dir);

  //Now that I've searched the whole directory, send the TChain off to be
  //processed (cumsum is the main thing to do).
  theChain->Process("");//INSERT NAME OF ANALYSIS FILE HERE.

  //WRITE SOME CODE TO CHANGE THE FILE NAME OF THE ANALYSIS OUTPUT SO THAT IT
  //DOESN'T GET OVERWRITTEN.
  TString command;//make up a command to move the file name, or whatever.
  gSystem->Exec(command);
}
