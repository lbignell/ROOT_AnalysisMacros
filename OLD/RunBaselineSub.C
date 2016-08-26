#include "TFile.h"

void RunBaselineSub()
{
  TFile* f = TFile::Open("SimpleBLsub_5461_5561.root");
  Results->Process("BaselineRemoval.C+");
}
