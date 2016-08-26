//This function will just skip the header info and make the file easier
//to work with by making it a ROOT file.
#include <fstream>
#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include <string>
#include <iostream>
#include <cstdlib>

using namespace std;

void ReformatFile(string filename){
  //Code currently isn't working. The problem is that I am using a string as the
  //filename whereas root expects a char array (there's a couple of later errors
  //caused by string useage too).
  string extension = ".root";
  int lastindex = filename.find_last_of("."); 
  string rawname = filename.substr(0, lastindex); 
  string ROOTfname = rawname + extension;

  TFile* f = TFile::Open(ROOTfname.c_str(), "RECREATE");
  TTree* tree = new TTree("FluorSpec",
			  "Data from Minfang's fluorescence spectrometer");

  vector<double> wavelength;
  vector<double> intensity;
  double lambda_ex = 0;
  tree->Branch("Wavelength", "vector<double>", &wavelength);
  tree->Branch("Intensity", "vector<double>", &intensity);
  tree->Branch("Lambda_ex", &lambda_ex, "Lambda_ex/D");

  //Open file (read only)
  fstream thefile;
  thefile.open(filename.c_str(), ios_base::in);
  char dummy[256];
  thefile.getline(dummy, 256);
  //cout << "First line: " << dummy << endl;
  thefile.getline(dummy, 256);
  //cout << "Second line: " << dummy << endl;
  long int numlines = strtol(dummy, NULL, 10);
  //cout << "Second line = " << numlines << endl;

  //define a pointer to keep track of index of next number:
  char* pNext;

  //header lines with excitation wavelength.
  thefile.getline(dummy, 256);
  //Skip over the first 3 characters ("D1 ") of the header line.
  pNext = &(dummy[3]);
  lambda_ex = strtod(pNext,&pNext);
  cout << "lambda_ex = " << lambda_ex << endl;

  //Skip header line
  thefile.getline(dummy, 256);

  double wl;
  double inten;

  for(int i = 0; i<numlines; i++){
    thefile.getline(dummy, 256);
    wl = strtod(dummy, &pNext);
    inten = strtod(pNext, NULL);
    //cout << "Wavelength = " << wl << ", Intensity = " << inten << endl; 
    wavelength.push_back(wl);
    intensity.push_back(inten);
  }

  tree->Fill();
  tree->Write();
  f->Close();

}
