//This is not meant to be compiled.
#include "TROOT.h"
#include "TSystem.h"

void ProcessData(){

gROOT->ProcessLine(".x CollateRawDatav2.C+(\"/mnt/hgfs/share/beamrun/RawData/\", 1575, 1656)");
gSystem->Exec("mv SimpleIntegral.root Water_2000MeV.root");

gROOT->ProcessLine(".x CollateRawDatav2.C+(\"/mnt/hgfs/share/beamrun/RawData/\", 1658, 1830)");
gSystem->Exec("mv SimpleIntegral.root Water_475MeV.root");

gROOT->ProcessLine(".x CollateRawDatav2.C+(\"/mnt/hgfs/share/beamrun/RawData/\", 1831, 1979)");
gSystem->Exec("mv SimpleIntegral.root Water_210MeV.root");

gROOT->ProcessLine(".x CollateRawDatav2.C+(\"/mnt/hgfs/share/beamrun/RawData/\", 1980, 2177)");
gSystem->Exec("mv SimpleIntegral.root WbLSp4Pc_210MeV.root");

gROOT->ProcessLine(".x CollateRawDatav2.C+(\"/mnt/hgfs/share/beamrun/RawData/\", 2178, 2383)");
gSystem->Exec("mv SimpleIntegral.root WbLSp4Pc_475MeV.root");

gROOT->ProcessLine(".x CollateRawDatav2.C+(\"/mnt/hgfs/share/beamrun/RawData/\", 2384, 2571)");
gSystem->Exec("mv SimpleIntegral.root WbLSp4Pc_2000MeV.root");

gROOT->ProcessLine(".x CollateRawDatav2.C+(\"/mnt/hgfs/share/beamrun/RawData/\", 2572, 2740)");
gSystem->Exec("mv SimpleIntegral.root WbLS1Pc_2000MeV.root");

gROOT->ProcessLine(".x CollateRawDatav2.C+(\"/mnt/hgfs/share/beamrun/RawData/\", 2743, 2923)");
gSystem->Exec("mv SimpleIntegral.root WbLSp1Pc_475MeV.root");

gROOT->ProcessLine(".x CollateRawDatav2.C+(\"/mnt/hgfs/share/beamrun/RawData/\", 2924, 3101)");
gSystem->Exec("mv SimpleIntegral.root WbLSp1Pc_210MeV.root");

gROOT->ProcessLine(".x CollateRawDatav2.C+(\"/mnt/hgfs/share/beamrun/RawData/\", 3121, 3314)");
gSystem->Exec("mv SimpleIntegral.root LS_210MeV.root");

gROOT->ProcessLine(".x CollateRawDatav2.C+(\"/mnt/hgfs/share/beamrun/RawData/\", 3315, 3506)");
gSystem->Exec("mv SimpleIntegral.root LS_475MeV.root");

gROOT->ProcessLine(".x CollateRawDatav2.C+(\"/mnt/hgfs/share/beamrun/RawData/\", 3507, 3662)");
gSystem->Exec("mv SimpleIntegral.root LS_2000MeV.root");

gROOT->ProcessLine(".x CollateRawDatav2.C+(\"/mnt/hgfs/share/beamrun/RawData/\", 3663, 3663)");
gSystem->Exec("mv SimpleIntegral.root PMTCalib_T1H3.root");

gROOT->ProcessLine(".x CollateRawDatav2.C+(\"/mnt/hgfs/share/beamrun/RawData/\", 3664, 3664)");
gSystem->Exec("mv SimpleIntegral.root PMTCalib_T2.root");

gROOT->ProcessLine(".x CollateRawDatav2.C+(\"/mnt/hgfs/share/beamrun/RawData/\", 3665, 3665)");
gSystem->Exec("mv SimpleIntegral.root PMTCalib_H1.root");

gROOT->ProcessLine(".x CollateRawDatav2.C+(\"/mnt/hgfs/share/beamrun/RawData/\", 3666, 3666)");
gSystem->Exec("mv SimpleIntegral.root PMTCalib_H2.root");
}
