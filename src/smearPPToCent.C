//Author: Chris McGinn

//cpp
#include <iostream>
#include <vector>

//ROOT
#include "TDatime.h"
#include "TDirectoryFile.h"
#include "TFile.h"
#include "TRandom3.h"
#include "TTree.h"

//Local
#include "include/checkMakeDir.h"
#include "include/csnParamReader.h"
#include "include/paramPropagator.h"
#include "include/returnRootFileContentsList.h"

int smearPPToCent(const std::string inFileName, const std::string paramFileName)
{
  if(!checkFileExt(inFileName, "root")) return 1;
  if(!checkFileExt(paramFileName, "txt")) return 1;

  TRandom3* randGen_p = new TRandom3(0);
  
  TDatime* date = new TDatime();
  const std::string dateStr = std::to_string(date->GetDate());
  delete date;

  paramPropagator params;
  params.setupFromTXT(paramFileName);
  std::map<std::string, std::string> paramFound = params.getParamFound();

  const std::string csnParamFileName = "input/csnParams_UPDATE_20190510.txt";
  if(!checkFileExt(csnParamFileName, ".txt")) return 1;

  csnParamReader csnParams(csnParamFileName);
  csnParams.Print();
  
  const Int_t nCentBinsMax = 10;
  const Int_t nSmears = 10;
  const std::string jetRStr = params.getJetRStr();  

  Int_t nCentBins = params.getNCentBins();
  Double_t centBinsLow[nCentBinsMax];
  Double_t centBinsHigh[nCentBinsMax];
  params.getCentBinsLow(centBinsLow);
  params.getCentBinsHigh(centBinsHigh);

  std::string imbInStr = params.getImbInStr();
  bool isVectorImb = params.getIsVectorImb();
  bool isDoubleImb = params.getIsDoubleImb();
  std::string imbNStr = params.getImbNStr();
  std::string imbPtStr = params.getImbPtStr();
  std::string imbPhiStr = params.getImbPhiStr();
  std::string imbEtaStr = params.getImbEtaStr();

  std::vector<std::string> imbBranchNames = {imbPtStr, imbPhiStr, imbEtaStr};

  
  //VECTOR IF RC INPUT IS VECTOR FLOAT OR DOUBLE
  std::vector<float>* jtptF_p=NULL;
  std::vector<float>* jtphiF_p=NULL;
  std::vector<float>* jtetaF_p=NULL;

  std::vector<double>* jtptD_p=NULL;
  std::vector<double>* jtphiD_p=NULL;
  std::vector<double>* jtetaD_p=NULL;

  const Int_t nMaxJet = 500;
  Int_t nref_;
  Float_t jtptF_[nMaxJet];
  Float_t jtphiF_[nMaxJet];
  Float_t jtetaF_[nMaxJet];
  Float_t jtptCentSmearF_[nMaxJet][nCentBinsMax][nSmears];
  
  Double_t jtptD_[nMaxJet];
  Double_t jtphiD_[nMaxJet];
  Double_t jtetaD_[nMaxJet];
  
  if(paramFound["IMBINPUT"].size() == 0){
    std::cout << "Given config \'" << paramFileName << "\' must define IMBINPUT. return 1" << std::endl;
    return 1;
  }

  //Create an output filename
  std::string outFileName = inFileName;
  while(outFileName.find("/") != std::string::npos){outFileName.replace(0, outFileName.find("/")+1, "");}
  if(outFileName.find(".") != std::string::npos) outFileName = outFileName.substr(0, outFileName.rfind("."));
  checkMakeDir("output");
  checkMakeDir("output/" + dateStr);
  outFileName = "output/" + dateStr + "/" + outFileName + "_" + jetRStr + "PF_SMEAR_" + dateStr + ".root";

  TFile* outFile_p = new TFile(outFileName.c_str(), "RECREATE");
  TDirectoryFile* imbParamDir_p = (TDirectoryFile*)outFile_p->mkdir("imbParamDir");
  TTree* outTree_p = new TTree("smearTree", "");

  outTree_p->Branch("nref", &nref_, "nref/I");
  outTree_p->Branch("jtpt", jtptF_, "jtpt[nref]/F");
  outTree_p->Branch("jtphi", jtphiF_, "jtphi[nref]/F");
  outTree_p->Branch("jteta", jtetaF_, "jteta[nref]/F");
  outTree_p->Branch("jtptCentSmear", jtptCentSmearF_, ("jtptCentSmear[nref][" + std::to_string(nCentBins) + "][" + std::to_string(nSmears) + "]/F").c_str());
  
  TFile* inFile_p = new TFile(inFileName.c_str(), "READ");
  std::vector<std::string> ttrees = returnRootFileContentsList(inFile_p, "TTree");
  bool imbIsFound = false;
  for(auto const & iter : ttrees){
    if(isStrSame(imbInStr, iter)) imbIsFound = true;
    
    if(imbIsFound) break;
  }

  if(!imbIsFound){
    std::cout << "INPUT TREE FOR IMB \'" << imbInStr << "\' is not found. return 1" << std::endl;
    return 1;
  }

  TTree* imbTreeIn_p = (TTree*)inFile_p->Get(imbInStr.c_str());
  TObjArray* imbListOfBranchesArr = imbTreeIn_p->GetListOfBranches();
  std::map<std::string, bool> imbListOfBranchesMap;
  for(Int_t bI = 0; bI < imbListOfBranchesArr->GetEntries(); ++bI){
    imbListOfBranchesMap[imbListOfBranchesArr->At(bI)->GetName()] = true;
  }

  imbTreeIn_p->SetBranchStatus("*", 0);
  for(auto const & iter : imbBranchNames){
    if(iter.size() == 0) continue;

    if(imbListOfBranchesMap.count(iter) == 0){
      std::cout << "Requested branch \'" << iter << "\' not found in ttree \'" << imbInStr << "\'. please check. return 1" << std::endl;
      return 1;
    }
    imbTreeIn_p->SetBranchStatus(iter.c_str(), 1);
  }

  if(isVectorImb){
    if(isDoubleImb){
      imbTreeIn_p->SetBranchAddress(imbPtStr.c_str(), &jtptD_p);
      imbTreeIn_p->SetBranchAddress(imbPhiStr.c_str(), &jtphiD_p);
      imbTreeIn_p->SetBranchAddress(imbEtaStr.c_str(), &jtetaD_p);
    }
    else{
      imbTreeIn_p->SetBranchAddress(imbPtStr.c_str(), &jtptF_p);
      imbTreeIn_p->SetBranchAddress(imbPhiStr.c_str(), &jtphiF_p);
      imbTreeIn_p->SetBranchAddress(imbEtaStr.c_str(), &jtetaF_p);
    }
  }
  else{
    imbTreeIn_p->SetBranchAddress(imbNStr.c_str(), &nref_);

    if(isDoubleImb){
      imbTreeIn_p->SetBranchAddress(imbPtStr.c_str(), jtptD_);
      imbTreeIn_p->SetBranchAddress(imbPhiStr.c_str(), jtphiD_);
      imbTreeIn_p->SetBranchAddress(imbEtaStr.c_str(), jtetaD_);
    }
    else{
      imbTreeIn_p->SetBranchAddress(imbPtStr.c_str(), jtptF_);
      imbTreeIn_p->SetBranchAddress(imbPhiStr.c_str(), jtphiF_);
      imbTreeIn_p->SetBranchAddress(imbEtaStr.c_str(), jtetaF_);
    }
  }

  const Int_t nEntries = imbTreeIn_p->GetEntries();
  const Int_t nDiv = TMath::Max(1, nEntries/20);
  
  std::cout << "Processing " << nEntries << "..." << std::endl;
  for(Int_t entry = 0; entry < nEntries; ++entry){
    if(entry%nDiv == 0) std::cout << " Entry " << entry << "/" << nEntries << std::endl;
    imbTreeIn_p->GetEntry(entry);

    if(isVectorImb){
      nref_ = 0;

      if(isDoubleImb){
        for(unsigned int pI = 0; pI < jtptD_p->size(); ++pI){
          jtptF_[nref_] = (Float_t)jtptD_p->at(pI);
          jtphiF_[nref_] = (Float_t)jtphiD_p->at(pI);
          jtetaF_[nref_] = (Float_t)jtetaD_p->at(pI);

          ++nref_;
        }
      }
      else{
        for(unsigned int pI = 0; pI < jtptF_p->size(); ++pI){
          jtptF_[nref_] = (Float_t)jtptF_p->at(pI);
          jtphiF_[nref_] = (Float_t)jtphiF_p->at(pI);
          jtetaF_[nref_] = (Float_t)jtetaF_p->at(pI);

          ++nref_;
        }
      }
    }
    else if(isDoubleImb){
      for(Int_t pI = 0; pI < nref_; ++pI){
        jtptF_[pI] = (Float_t)jtptD_[pI];
        jtphiF_[pI] = (Float_t)jtphiD_[pI];
        jtetaF_[pI] = (Float_t)jtetaD_[pI];
      }
    }
  
  
    for(Int_t pI = 0; pI < nref_; ++pI){
      double sigma1 = csnParams.sigmaFromPt(jtptF_[pI], 50, 90, std::stoi(jetRStr), false);

      for(Int_t pI2 = 0; pI2 < nCentBins; ++pI2){
	double sigma2 = csnParams.sigmaFromPt(jtptF_[pI], centBinsLow[pI2], centBinsHigh[pI2], std::stoi(jetRStr), true);

	double newSigma = TMath::Sqrt(TMath::Max(0.0, sigma2*sigma2 - sigma1*sigma1));

	for(Int_t sI = 0; sI < nSmears; ++sI){
	  double newPt = jtptF_[pI]*randGen_p->Gaus(1.0, newSigma);

	  jtptCentSmearF_[pI][pI2][sI] = newPt;
	}
      }
    }

    outTree_p->Fill();
  }
  
  inFile_p->Close();  
  delete inFile_p;

  outFile_p->cd();
  outTree_p->Write("", TObject::kOverwrite);
  delete outTree_p;

  imbParamDir_p->cd();

  for(auto const & iter : paramFound){
    TNamed tempName(iter.first.c_str(), iter.second.c_str());
    tempName.Write("", TObject::kOverwrite);
  }

  imbParamDir_p->Close();
  delete imbParamDir_p;

  outFile_p->Close();
  delete outFile_p;

  delete randGen_p;
  
  return 0;
}

int main(int argc, char* argv[])
{
  if(argc != 3){
    std::cout << "Usage: ./bin/smearPPToCent.exe <inFileName> <paramFileName>. return 1" << std::endl;
    return 1;
  }

  int retVal = 0;
  retVal += smearPPToCent(argv[1], argv[2]);
  return retVal;
}
