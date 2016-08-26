#include "TChain.h"
#include "TString.h"

void ReadoutSimpleIntegral()
{
TChain* TheChain = new TChain("FADCData");


 for(int num = 5562; num<5697; num++){
   TheChain->Add(TString::Format("rootoutputfile%i.root",num));
 }
 TheChain->Process("SimpleSumIntegral.C+");
}
