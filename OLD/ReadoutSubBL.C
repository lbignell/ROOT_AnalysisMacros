#include "TChain.h"
#include "TString.h"

void ReadoutSubBL()
{
TChain* TheChain = new TChain("FADCData");

 for(int num = 5461; num<5561; num++){
   TheChain->Add(TString::Format("rootoutputfile%i.root",num));
 }
 TheChain->Process("SubtractBL_Simple.C+");
}
