//Author: Chris McGinn (2019.11.05)

//cpp
#include <iostream>
#include <string>
#include <vector>

//ROOT
#include "TDatime.h"
#include "TDirectoryFile.h"
#include "TFile.h"
#include "TH1D.h"
#include "TLorentzVector.h"
#include "TMath.h"
#include "TTree.h"

//Local
#include "include/checkMakeDir.h"
#include "include/etaPhiFunc.h"
#include "include/getLinBins.h"
#include "include/getLogBins.h"
#include "include/plotUtilities.h"
#include "include/returnRootFileContentsList.h"

int makeHistImb(std::string inFileName, int codeSimEtaDeg)//0,none,1,constituent,2,jet
{
  if(!checkFileExt(inFileName, "root")) return 1;

  TDatime* date = new TDatime();
  const std::string dateStr = std::to_string(date->GetDate());
  delete date;

  checkMakeDir("output");
  checkMakeDir("output/" + dateStr);
  
  std::string outFileName = inFileName.substr(0, inFileName.find(".root"));
  while(outFileName.find("/") != std::string::npos){
    outFileName.replace(0, outFileName.find("/")+1, "");
  }
  outFileName = "output/" + dateStr + "/" + outFileName + "_ImbHist_codeSimEtaDeg" + std::to_string(codeSimEtaDeg) + "_" + dateStr + ".root";

  Float_t otherLowCut = 110;
  Float_t otherHighCut = 1600;
  
  Int_t nJtPtBins = 5;
  Float_t jtPtBinsLow = 160;
  Float_t jtPtBinsHigh = 1000;
  Double_t jtPtBins[nJtPtBins+1];
  getLogBins(jtPtBinsLow, jtPtBinsHigh, nJtPtBins, jtPtBins);
  std::string jtPtBinsStr[nJtPtBins];
  jtPtBins[nJtPtBins-1] = jtPtBins[nJtPtBins];
  --nJtPtBins;
  
  const Int_t nJtAbsEtaBins = 6;
  Float_t jtAbsEtaBinsLow = 0.2;
  Float_t jtAbsEtaBinsHigh = 2.0;
  Double_t jtAbsEtaBins[nJtAbsEtaBins+1];
  getLinBins(jtAbsEtaBinsLow, jtAbsEtaBinsHigh, nJtAbsEtaBins, jtAbsEtaBins);
  std::string jtAbsEtaBinsStr[nJtAbsEtaBins];
  
  TFile* outFile_p = new TFile(outFileName.c_str(), "RECREATE");
  TH1D* xjHist_p[nJtPtBins][nJtAbsEtaBins];

  for(Int_t jI = 0; jI < nJtPtBins; ++jI){
    jtPtBinsStr[jI] = "JtPt" + prettyString(jtPtBins[jI], 1, true) + "to" + prettyString(jtPtBins[jI+1], 1, true);
    for(Int_t jI2 = 0; jI2 < nJtAbsEtaBins; ++jI2){
      jtAbsEtaBinsStr[jI2] = "JtAbsEta" + prettyString(jtAbsEtaBins[jI2], 1, true) + "to" + prettyString(jtAbsEtaBins[jI2+1], 1, true);

      std::string histName = "xj_" + jtPtBinsStr[jI] + "_" + jtAbsEtaBinsStr[jI2] + "_h";

      xjHist_p[jI][jI2] = new TH1D(histName.c_str(), ";x_{J}=p_{T,Forward}/p_{T,Mid-Eta};Counts", 21, 0.0, 2.0);
    }
  }
  
  TFile* inFile_p = new TFile(inFileName.c_str(), "READ");
  std::vector<std::string> treeList = returnRootFileContentsList(inFile_p, "TTree");
  TTree* inTree_p = (TTree*)inFile_p->Get(treeList[0].c_str());

  TObjArray* imbListOfBranchesArr = inTree_p->GetListOfBranches();
  std::map<std::string, bool> imbListOfBranchesMap;
  for(Int_t bI = 0; bI < imbListOfBranchesArr->GetEntries(); ++bI){
    imbListOfBranchesMap[imbListOfBranchesArr->At(bI)->GetName()] = true;
  }

  std::cout << "BRANCHES: " << std::endl;
  for(Int_t bI = 0; bI < imbListOfBranchesArr->GetEntries(); ++bI){
    std::cout << " " << imbListOfBranchesArr->At(bI)->GetName() << std::endl;
  }

  const Int_t nMaxJets = 500;
  Int_t nref_;
  Float_t jtpt_[nMaxJets];
  Float_t jteta_[nMaxJets];
  Float_t jtphi_[nMaxJets];
  Float_t jtweight_[nMaxJets];
  Float_t refpt_[nMaxJets];

  const Int_t nMaxParticles = 50000;
  Int_t nconst_;
  Float_t constpt_[nMaxParticles];
  Float_t consteta_[nMaxParticles];
  Float_t constphi_[nMaxParticles];
  Int_t constjtpos_[nMaxParticles];

  inTree_p->SetBranchStatus("*", 0);
  inTree_p->SetBranchStatus("nref", 1);
  inTree_p->SetBranchStatus("jtpt", 1);
  inTree_p->SetBranchStatus("jteta", 1);
  inTree_p->SetBranchStatus("jtphi", 1);
  inTree_p->SetBranchStatus("jtweight", 1);
  if(imbListOfBranchesMap["refpt"]) inTree_p->SetBranchStatus("refpt", 1);
  
  inTree_p->SetBranchStatus("nconst", 1);
  inTree_p->SetBranchStatus("constpt", 1);
  inTree_p->SetBranchStatus("consteta", 1);
  inTree_p->SetBranchStatus("constphi", 1);
  inTree_p->SetBranchStatus("constjtpos", 1);

  inTree_p->SetBranchAddress("nref", &nref_);
  inTree_p->SetBranchAddress("jtpt", jtpt_);
  inTree_p->SetBranchAddress("jteta", jteta_);
  inTree_p->SetBranchAddress("jtphi", jtphi_);
  inTree_p->SetBranchAddress("jtweight", jtweight_);
  if(imbListOfBranchesMap["refpt"]) inTree_p->SetBranchAddress("refpt", refpt_);

  inTree_p->SetBranchAddress("nconst", &nconst_);
  inTree_p->SetBranchAddress("constpt", constpt_);
  inTree_p->SetBranchAddress("consteta", consteta_);
  inTree_p->SetBranchAddress("constphi", constphi_);
  inTree_p->SetBranchAddress("constjtpos", constjtpos_);

  const Int_t nEntries = inTree_p->GetEntries();
  const Int_t nDiv = TMath::Max(1, nEntries/20);

  std::cout << "Processing " << nEntries << "..." << std::endl;
  for(Int_t entry = 0; entry < nEntries; ++entry){
    if(entry%nDiv == 0) std::cout << " Entry " << entry << "/" << nEntries << "..." << std::endl;
    inTree_p->GetEntry(entry);    

    for(Int_t jI = 0; jI < nref_; ++jI){
      if(TMath::Abs(jteta_[jI]) >= jtAbsEtaBinsLow) continue;
      if(jtpt_[jI] < jtPtBinsLow) continue;
      if(jtpt_[jI] >= jtPtBinsHigh) continue;
      if(imbListOfBranchesMap["refpt"]){
	if(refpt_[jI] < 0.0) continue;
      }
      
      for(Int_t jI2 = 0; jI2 < nref_; ++jI2){
	if(jI == jI2) continue;
	if(TMath::Abs(jteta_[jI2]) < jtAbsEtaBinsLow) continue;
	if(TMath::Abs(jteta_[jI2]) >= jtAbsEtaBinsHigh) continue;
	if(imbListOfBranchesMap["refpt"]){
	  if(refpt_[jI] < 0.0) continue;
	}
	if(TMath::Abs(getDPHI(jtphi_[jI], jtphi_[jI2])) < 5.*TMath::Pi()/6.) continue;
		
	if(codeSimEtaDeg == 1){
	  TLorentzVector undegraded(0.0, 0.0, 0.0, 0.0);
	  TLorentzVector degraded(0.0, 0.0, 0.0, 0.0);

	  for(Int_t cI = 0; cI < nconst_; ++cI){
	    if(constjtpos_[cI] != jI2) continue;

	    TLorentzVector tempUndegraded;
	    tempUndegraded.SetPtEtaPhiM(constpt_[cI], consteta_[cI], constphi_[cI], 0.0);
	    
	    Double_t factor = 1.0;
	    if(TMath::Abs(consteta_[cI]) < 2.4) factor = (1.0 - 0.5*(TMath::Abs(consteta_[cI]) - 0.2)/2.8);
	    else factor = 1.0 + 0.5*TMath::Erf(0.4*(constpt_[cI] - 4));
	      
	    TLorentzVector tempDegraded(tempUndegraded.Px()*factor, tempUndegraded.Py()*factor, tempUndegraded.Pz()*factor, 0.0);
	    
	    undegraded += tempUndegraded;
	    degraded += tempDegraded;	    
	  }

	  jtpt_[jI2] *= degraded.Pt()/undegraded.Pt();
	}
	if(codeSimEtaDeg == 2){
	  //	  Double_t factor = (1.0 - 0.1*(TMath::Abs(jteta_[jI2]) - 0.2)/2.8);
	  Double_t factor = 1.0;
	  if(TMath::Abs(jteta_[jI]) < 2.4) factor = (1.0 - 0.5*(TMath::Abs(jteta_[jI]) - 0.2)/2.8);
	  else factor = 1.0 + 0.5*TMath::Erf(0.4*(jtpt_[jI] - 4));
	  jtpt_[jI2] *= factor;
	}
	
	if(jtpt_[jI2] < jtPtBinsLow) continue;
	if(jtpt_[jI2] >= jtPtBinsHigh) continue;	
	
	Int_t ptPos = -1;
	for(Int_t pI = 0; pI < nJtPtBins; ++pI){
	  if(jtpt_[jI2] >= jtPtBins[pI] && jtpt_[jI2] < jtPtBins[pI+1]){
	    ptPos = pI;
	    break;
	  }
	}

	Int_t absEtaPos = -1;
	for(Int_t pI = 0; pI < nJtAbsEtaBins; ++pI){
	  if(TMath::Abs(jteta_[jI2]) >= jtAbsEtaBins[pI] && TMath::Abs(jteta_[jI2]) < jtAbsEtaBins[pI+1]){
	    absEtaPos = pI;
	    break;
	  }
	}
	
	if(jtpt_[jI2] > otherLowCut && jtpt_[jI2] < otherHighCut) xjHist_p[ptPos][absEtaPos]->Fill(jtpt_[jI2]/jtpt_[jI], jtweight_[jI2]);
	
	break;
      }

      break;
    }
  }
  
  inFile_p->Close();
  delete inFile_p;

  outFile_p->cd();

  for(Int_t jI = 0; jI < nJtPtBins; ++jI){
    for(Int_t jI2 = 0; jI2 < nJtAbsEtaBins; ++jI2){
      xjHist_p[jI][jI2]->Write("", TObject::kOverwrite);
      delete xjHist_p[jI][jI2];
    }
  }

  TDirectoryFile* paramsDir_p = (TDirectoryFile*)outFile_p->mkdir("params");
  paramsDir_p->cd();

  std::string jtPtBinsStr2 = "";
  std::string jtAbsEtaBinsStr2 = "";

  for(Int_t jI = 0; jI < nJtPtBins; ++jI){
    jtPtBinsStr2 = jtPtBinsStr2 + jtPtBinsStr[jI] + ",";
  }
  for(Int_t jI = 0; jI < nJtAbsEtaBins; ++jI){
    jtAbsEtaBinsStr2 = jtAbsEtaBinsStr2 + jtAbsEtaBinsStr[jI] + ",";
  }

  std::map<std::string, std::string> paramMap;
  
  paramMap["nJtPtBins"] = std::to_string(nJtPtBins);
  paramMap["jtPtBins"] = jtPtBinsStr2;
  paramMap["nJtAbsEtaBins"] = std::to_string(nJtAbsEtaBins);
  paramMap["jtAbsEtaBins"] = jtAbsEtaBinsStr2;

  for(auto const & iter : paramMap){
    TNamed tempName(iter.first.c_str(), iter.second.c_str());
    tempName.Write("", TObject::kOverwrite);
  }    
  
  paramsDir_p->Close();
  delete paramsDir_p;
    
  outFile_p->Close();
  delete outFile_p;
  
  return 0;
}

int main(int argc, char* argv[])
{
  if(argc != 3){
    std::cout << "Usage: ./bin/makeHistImb.exe <inFileName> <codeSimEtaDeg>. return 1" << std::endl;
    return 1;
  }

  int retVal = 0;
  retVal += makeHistImb(argv[1], std::stoi(argv[2]));
  return retVal;
}
