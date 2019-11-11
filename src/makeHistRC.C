//cpp
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

//ROOT
#include "TDatime.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TTree.h"

//Local
#include "include/checkMakeDir.h"
#include "include/histDefUtility.h"
#include "include/paramPropagator.h"
#include "include/plotUtilities.h"

int makeHistRC(const std::string inFileName)
{
  if(!checkFile(inFileName) || inFileName.find(".root") == std::string::npos){
    std::cout << "Given inFileName \'" << inFileName << "\' is not valid. return 1" << std::endl;
    return 1;
  }

  TDatime* date = new TDatime();
  const std::string dateStr = std::to_string(date->GetDate());
  delete date;

  std::cout << "LINE: " << __LINE__ << std::endl;

  Float_t ptRC_, ptRhoRC_, ptSubRC_;
  UInt_t etaPos_;
  Int_t centRC_, centPos_;

  TFile* inFile_p = new TFile(inFileName.c_str(), "READ");
  paramPropagator params;
  params.setupFromROOT(inFile_p);
  const Int_t nCentBinsMax = 10;
  const Int_t nEtaBinsMax = 10;

  if(!params.checkNEtaBinsMax(nEtaBinsMax)) return 1;
  if(!params.checkNCentBinsMax(nCentBinsMax)) return 1;

  Int_t nCentBins = params.getNCentBins();
  Int_t nEtaBins = params.getNEtaBins();

  TTree* rcTree_p = (TTree*)inFile_p->Get("rcTree");
  Double_t rcPtMin = rcTree_p->GetMinimum("ptRC");
  Double_t rcPtMax = rcTree_p->GetMaximum("ptRC");
  Double_t rhoPtMin = rcTree_p->GetMinimum("ptRhoRC");
  Double_t rhoPtMax = rcTree_p->GetMaximum("ptRhoRC");
  Double_t rcMinRhoPtMin = rcTree_p->GetMinimum("ptSubRC");
  Double_t rcMinRhoPtMax = rcTree_p->GetMaximum("ptSubRC");

  std::vector<std::vector<Double_t> > rcPtVals, rhoPtVals, rcMinRhoPtVals;
  for(Int_t cI = 0; cI < nCentBins; ++cI){
    rcPtVals.push_back({});
    rhoPtVals.push_back({});
    rcMinRhoPtVals.push_back({});
  }

  rcTree_p->SetBranchStatus("*", 0);
  rcTree_p->SetBranchStatus("ptRC", 1);
  rcTree_p->SetBranchStatus("ptRhoRC", 1);
  rcTree_p->SetBranchStatus("ptSubRC", 1);
  rcTree_p->SetBranchStatus("centPos", 1);

  rcTree_p->SetBranchAddress("ptRC", &ptRC_);
  rcTree_p->SetBranchAddress("ptRhoRC", &ptRhoRC_);
  rcTree_p->SetBranchAddress("ptSubRC", &ptSubRC_);
  rcTree_p->SetBranchAddress("centPos", &centPos_);

  const Int_t nEntries = rcTree_p->GetEntries();
  const Int_t nDiv = TMath::Max(1, nEntries/20);

  std::cout << "Pre-Processing " << nEntries << "..." << std::endl;
  for(Int_t entry = 0; entry < nEntries; ++entry){
    rcTree_p->GetEntry(entry);

    rcPtVals[centPos_].push_back(ptRC_);
    rhoPtVals[centPos_].push_back(ptRhoRC_);
    rcMinRhoPtVals[centPos_].push_back(ptSubRC_);
  }

  for(Int_t cI = 0; cI < nCentBins; ++cI){
    std::vector<Double_t> tempVect = rcPtVals[cI];
    std::sort(std::begin(tempVect), std::end(tempVect));
    rcPtVals[cI] = tempVect;
    
    tempVect = rhoPtVals[cI];
    std::sort(std::begin(tempVect), std::end(tempVect));
    rhoPtVals[cI] = tempVect;

    tempVect = rcMinRhoPtVals[cI];
    std::sort(std::begin(tempVect), std::end(tempVect));
    rcMinRhoPtVals[cI] = tempVect;
  }

  Double_t threeSigmaMax = -1;
  for(Int_t cI = 0; cI < nCentBins; ++cI){
    Int_t threeSigmaPos = 997*rcMinRhoPtVals[cI].size()/1000;

    if(rcMinRhoPtVals[cI][threeSigmaPos] > threeSigmaMax) threeSigmaMax = rcMinRhoPtVals[cI][threeSigmaPos];
  }
  
  //  rcPtMax = rcPtVals[threeSigmaPos];
  //  rhoPtMax = rhoPtVals[threeSigmaPos];
  ++rcPtMax;
  ++rhoPtMax;
  rcMinRhoPtMax = TMath::Max(threeSigmaMax, TMath::Abs(rcMinRhoPtMin-1));
  rcPtMin = TMath::Max(0., rcPtMin-1);
  rhoPtMin = TMath::Max(0., rhoPtMin-1);
  rcMinRhoPtMin -= 1;
    
  rcPtVals.clear();
  rhoPtVals.clear();
  rcMinRhoPtVals.clear();
  
  inFile_p->Close();
  delete inFile_p;

  std::vector<double> centBinsLow = params.getCentBinsLow();
  std::vector<double> centBinsHigh = params.getCentBinsHigh();

  std::vector<double> etaBinsLow = params.getEtaBinsLow();
  std::vector<double> etaBinsHigh = params.getEtaBinsHigh();

  std::string outFileName = inFileName;
  while(outFileName.find("/") != std::string::npos){outFileName.replace(0, outFileName.find("/")+1, "");}
  if(outFileName.find(".") != std::string::npos) outFileName = outFileName.substr(0, outFileName.rfind("."));
  outFileName = "output/" + dateStr + "/" + outFileName + "_HISTRC_" + dateStr + ".root";
  checkMakeDir("output");
  checkMakeDir("output/" + dateStr);
  TFile* outFile_p = new TFile(outFileName.c_str(), "RECREATE");
  TH1D* histRC_p[nCentBinsMax][nEtaBinsMax];
  TH1D* histRho_p[nCentBinsMax][nEtaBinsMax];
  TH1D* histRCMinRho_p[nCentBinsMax][nEtaBinsMax];

  TH2D* histRC_VCent_p[nEtaBinsMax];
  TH2D* histRho_VCent_p[nEtaBinsMax];
  TH2D* histRCMinRho_VCent_p[nEtaBinsMax];

  for(Int_t eI = 0; eI < nEtaBins; ++eI){
    std::string etaStr = "Eta" + prettyString(etaBinsLow[eI], 1, true) + "to" + prettyString(etaBinsHigh[eI], 1, true);

    histRC_VCent_p[eI] = new TH2D(("histRC_VCent_" + etaStr + "_h").c_str(), ";Centrality (%);Random Cone #Sigma p_{T}", 100, -0.5, 99.5, 100, rcPtMin, rcPtMax);
    histRho_VCent_p[eI] = new TH2D(("histRho_VCent_" + etaStr + "_h").c_str(), ";Centrality (%);Random Cone #rho#times A", 100, -0.5, 99.5, 100, rhoPtMin, rhoPtMax);
    histRCMinRho_VCent_p[eI] = new TH2D(("histRCMinRho_VCent_" + etaStr + "_h").c_str(), ";Centrality (%);Random Cone #Sigma p_{T} - #rho#times A", 100, -0.5, 99.5, 100, rcMinRhoPtMin, rcMinRhoPtMax);
    centerTitles({histRC_VCent_p[eI], histRho_VCent_p[eI], histRCMinRho_VCent_p[eI]});

    for(Int_t cI = 0; cI < nCentBins; ++cI){
      std::string centStr = "Cent" + std::to_string(((int)centBinsLow[cI])) + "to" + std::to_string(((int)centBinsHigh[cI]));
      
      histRC_p[cI][eI] = new TH1D(("histRC_" + centStr + "_" + etaStr + "_h").c_str(), ";Random Cone #Sigma p_{T};Counts", 100, rcPtMin, rcPtMax);
      histRho_p[cI][eI] = new TH1D(("histRho_" + centStr + "_" + etaStr + "_h").c_str(), ";Random Cone #rho#times A;Counts", 100, rhoPtMin, rhoPtMax);
      histRCMinRho_p[cI][eI] = new TH1D(("histRCMinRho_" + centStr + "_" + etaStr + "_h").c_str(), ";Random Cone #Sigma p_{T} - #rho#times A;Counts", 100, rcMinRhoPtMin, rcMinRhoPtMax);
      centerTitles({histRC_p[cI][eI], histRho_p[cI][eI], histRCMinRho_p[cI][eI]});
    }
  }

  inFile_p = new TFile(inFileName.c_str(), "READ");
  rcTree_p = (TTree*)inFile_p->Get("rcTree");
  
  rcTree_p->SetBranchStatus("*", 0);
  rcTree_p->SetBranchStatus("centPos", 1);
  rcTree_p->SetBranchStatus("etaPos", 1);
  rcTree_p->SetBranchStatus("centRC", 1);
  rcTree_p->SetBranchStatus("ptRC", 1);
  rcTree_p->SetBranchStatus("ptRhoRC", 1);
  rcTree_p->SetBranchStatus("ptSubRC", 1);

  rcTree_p->SetBranchAddress("centPos", &centPos_);
  rcTree_p->SetBranchAddress("etaPos", &etaPos_);
  rcTree_p->SetBranchAddress("centRC", &centRC_);
  rcTree_p->SetBranchAddress("ptRC", &ptRC_);
  rcTree_p->SetBranchAddress("ptRhoRC", &ptRhoRC_);
  rcTree_p->SetBranchAddress("ptSubRC", &ptSubRC_);

  std::cout << "Processing " << nEntries << "..." << std::endl;
  for(Int_t entry = 0; entry < nEntries; ++entry){
    if(entry%nDiv == 0) std::cout << " Entry " << entry << "/" << nEntries << std::endl;
    rcTree_p->GetEntry(entry);

    histRC_p[centPos_][etaPos_]->Fill(ptRC_);
    histRho_p[centPos_][etaPos_]->Fill(ptRhoRC_);
    histRCMinRho_p[centPos_][etaPos_]->Fill(ptSubRC_);

    histRC_VCent_p[etaPos_]->Fill(centRC_, ptRC_);
    histRho_VCent_p[etaPos_]->Fill(centRC_, ptRhoRC_);
    histRCMinRho_VCent_p[etaPos_]->Fill(centRC_, ptSubRC_);
  }
  
  inFile_p->Close();
  delete inFile_p;
  
  outFile_p->cd();

  for(Int_t eI = 0; eI < nEtaBins; ++eI){
    histRC_VCent_p[eI]->Write("", TObject::kOverwrite);
    delete histRC_VCent_p[eI];
    
    histRho_VCent_p[eI]->Write("", TObject::kOverwrite);
    delete histRho_VCent_p[eI];
    
    histRCMinRho_VCent_p[eI]->Write("", TObject::kOverwrite);
    delete histRCMinRho_VCent_p[eI];
    
    for(Int_t cI = 0; cI < nCentBins; ++cI){
      histRC_p[cI][eI]->Write("", TObject::kOverwrite);
      delete histRC_p[cI][eI];

      histRho_p[cI][eI]->Write("", TObject::kOverwrite);
      delete histRho_p[cI][eI];

      histRCMinRho_p[cI][eI]->Write("", TObject::kOverwrite);
      delete histRCMinRho_p[cI][eI];
    }
  }

  std::map<std::string, std::string> paramFound = params.getParamFound();
  TDirectoryFile* rcParamDir_p = (TDirectoryFile*)outFile_p->mkdir("rcParamDir");
  rcParamDir_p->cd();
  for(auto const & iter : paramFound){
    TNamed tempName(iter.first.c_str(), iter.second.c_str());
    tempName.Write("", TObject::kOverwrite);
  }
  
  outFile_p->Close();
  delete outFile_p;
  
  return 0;
}

int main(int argc, char* argv[])
{
  if(argc != 2){
    std::cout << "Usage: ./bin/makeHistRC.exe <inFileName>" << std::endl;
    return 1;
  }
  
  int retVal = 0;
  retVal += makeHistRC(argv[1]);
  return retVal;
}
