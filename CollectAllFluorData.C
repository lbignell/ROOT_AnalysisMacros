//This macro will collect up all of the fluorescence spectrum data in the
//directory and write them to a single root file.
#include <fstream>
#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include <string>
#include <iostream>
#include <cstdlib>
#include "tinydir.h"

using namespace std;

void GrabFile(string fname, vector<double>& wl,
	      vector<double>& inten, double& lamb_ex, fstream& dump){
  
  //Open file (read only)
  fstream thefile;
  thefile.open(fname.c_str(), ios_base::in);
  char dummy[256];
  //skip first line
  thefile.getline(dummy, 256);
  //Second line contains number of data points
  thefile.getline(dummy, 256);
  long int numlines = strtol(dummy, NULL, 10);

  //define a pointer to keep track of index of next number:
  char* pNext;

  //header lines with excitation wavelength.
  thefile.getline(dummy, 256);
  //Skip over the first 3 characters ("D1 ") of the header line.
  pNext = &(dummy[3]);
  lamb_ex = strtod(pNext,&pNext);

  //Skip header line
  thefile.getline(dummy, 256);

  char* theWL;
  char* theInten;

  //Put the data into the vectors
  for(int i = 0; i<numlines; i++){
    thefile.getline(dummy, 256);
    //Original code for .txt files.
    //wl.push_back(strtod(dummy, &pNext));
    //inten.push_back(strtod(pNext, NULL));
    
    //Code for .csv files
    theWL = strtok(dummy, ",");
    theInten = strtok(NULL, ",");
    wl.push_back(strtod(theWL, NULL));
    inten.push_back(strtod(theInten, NULL));

    dump << wl.back() << "," << lamb_ex << "," << inten.back() << endl;
  }
}


void CollectAllFluorData(){
  string theFile;
  tinydir_dir dir;
  tinydir_file file;
  fstream filedump;
  filedump.open("DataDump.csv", ios_base::in | ios_base::out | ios_base::trunc);

  TFile* Collected = TFile::Open("CollectedFluorSpecData.root", "RECREATE");
  TTree* theTree = new TTree("FluorSpec", "Collected Fluorescence Spectra");
  vector<double> wavelength;
  vector<double> intensity;
  double lambda_ex = 0;
  theTree->Branch("Wavelength", "vector<double>", &wavelength);
  theTree->Branch("Intensity", "vector<double>", &intensity);
  theTree->Branch("Lambda_ex", &lambda_ex, "Lambda_ex/D");

  //make a loop over the file names:
  tinydir_open(&dir, ".");
  while (dir.has_next)
    {
      tinydir_readfile(&dir, &file);
      if((strstr(file.name, "BisMSB_PPO_S_LAB_H2O"))&&
	 (strstr(file.name, "corrected.csv"))){
	printf("Processing in file %s \n", file.name);
	GrabFile(file.name, wavelength, intensity, lambda_ex, filedump);
	theTree->Fill();
	wavelength.clear();
	intensity.clear();
      }  
      tinydir_next(&dir);
    }

  theTree->Write();
  Collected->Close();

}

