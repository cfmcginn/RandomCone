//AUTHOR: Chris McGinn

//cpp
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

//ROOT
#include "TDatime.h"
#include "TDirectoryFile.h"
#include "TFile.h"
#include "TNamed.h"
#include "TTree.h"

//Local
#include "include/centralityFromInput.h"
#include "include/checkMakeDir.h"
#include "include/etaPhiFunc.h"
#include "include/getLinBins.h"
#include "include/getLogBins.h"
#include "include/ncollFunctions_5TeV.h"
#include "include/paramPropagator.h"
#include "include/plotUtilities.h"
#include "include/returnRootFileContentsList.h"
#include "include/stringUtil.h"

int processForestForImb(std::string inFileName, std::string paramFileName)
{
  TDatime* date = new TDatime();
  const std::string dateStr = std::to_string(date->GetDate());
  delete date;
  
  //CHECK THAT YOUR FILES EXIST AND ARE SENSIBLE
  if(!checkFile(inFileName) || inFileName.find(".root") == std::string::npos){
    std::cout << "Given inFileName \'" << inFileName << "\' is invalid. return 1" << std::endl;
    return 1;
  }

  if(!checkFile(paramFileName) || paramFileName.find(".txt") == std::string::npos){
    std::cout << "Given paramFileName \'" << paramFileName << "\' is invalid. return 1" << std::endl;
    return 1;
  }

  //DEFINE PARAMS AND SOME DEFAULTS
  paramPropagator params;
  params.setupFromTXT(paramFileName);
  std::map<std::string, std::string> paramFound = params.getParamFound();

  //Imb in tree
  std::string imbInStr = params.getImbInStr();
  bool isVectorImb = params.getIsVectorImb();
  bool isDoubleImb = params.getIsDoubleImb();
  std::string imbPthatStr = params.getPthatValStr();
  std::string imbNStr = params.getImbNStr();
  std::string imbPtStr = params.getImbPtStr();
  std::string imbPhiStr = params.getImbPhiStr();
  std::string imbEtaStr = params.getImbEtaStr();
  std::string imbRefPtStr = params.getImbRefPtStr();
  std::string imbWeightStr = params.getImbWeightStr();

  //Cent tree vars
  std::string centInStr = params.getCentInStr();
  std::string centTableStr = params.getCentTableStr();
  std::string centVarType = params.getCentVarType();
  std::vector<std::string> centVarStr = params.getCentVarStr();
  std::vector<double> centVarD;
  std::vector<float> centVarF;
  std::vector<int> centVarI;

  bool doNCollWeights = params.getDoNCollWeights();
  
  std::string rStr = imbInStr.substr(0, imbInStr.find("PF"));
  while(rStr.find("/") != std::string::npos){
    rStr.replace(0, rStr.find("/")+1, "");
  }

  std::cout << "ImbRefPtStr: " << imbRefPtStr << std::endl;
  
  std::string imbConstNStr = params.getImbConstNStr();
  std::string imbConstPtStr = params.getImbConstPtStr();
  std::string imbConstPhiStr = params.getImbConstPhiStr();
  std::string imbConstEtaStr = params.getImbConstEtaStr();
  std::string imbConstjtposStr = params.getImbConstjtposStr();

  Float_t pthat_;
  Float_t pthatWeight_;
  
  std::vector<Double_t> pthats = params.getPthats();
  std::vector<Double_t> pthatWeights = params.getPthatWeights();

  std::cout << "Pthats: " << std::endl;
  for(unsigned int pI = 0; pI < pthats.size(); ++pI){
    std::cout << pthats[pI] << std::endl;
  }

  std::cout << "PthatWeights: " << std::endl;
  for(unsigned int pI = 0; pI < pthatWeights.size(); ++pI){
    std::cout << pthatWeights[pI] << std::endl;
  }

  //  return 1;
  
  //VECTOR IF RC INPUT IS VECTOR FLOAT OR DOUBLE
  std::vector<float>* jtptF_p=NULL;
  std::vector<float>* jtphiF_p=NULL;
  std::vector<float>* jtetaF_p=NULL;
  std::vector<float>* refptF_p=NULL;
  std::vector<float>* jtweightF_p=NULL;

  std::vector<double>* jtptD_p=NULL;
  std::vector<double>* jtphiD_p=NULL;
  std::vector<double>* jtetaD_p=NULL;
  std::vector<double>* refptD_p=NULL;  
  std::vector<double>* jtweightD_p=NULL;

  std::vector<float>* constptF_p=NULL;
  std::vector<float>* constphiF_p=NULL;
  std::vector<float>* constetaF_p=NULL;

  std::vector<double>* constptD_p=NULL;
  std::vector<double>* constphiD_p=NULL;
  std::vector<double>* constetaD_p=NULL;

  std::vector<int>* constjtpos_p=NULL;

  std::cout << __FILE__ << ", " << __LINE__ << std::endl;
  
  //ARRAY IF IMB INPUT IS ARRAY FLOAT OR DOUBLE
  const Int_t nMaxJet = 500;
  Int_t nref_;
  Float_t jtptF_[nMaxJet];
  Float_t jtphiF_[nMaxJet];
  Float_t jtetaF_[nMaxJet];
  Float_t refptF_[nMaxJet];
  Float_t jtweightF_[nMaxJet];

  Double_t jtptD_[nMaxJet];
  Double_t jtphiD_[nMaxJet];
  Double_t jtetaD_[nMaxJet];
  Double_t refptD_[nMaxJet];
  Double_t jtweightD_[nMaxJet];

  const Int_t nMaxParticles = 50000;
  Int_t nconst_;
  Float_t constptF_[nMaxParticles];
  Float_t constphiF_[nMaxParticles];
  Float_t constetaF_[nMaxParticles]; 

  Float_t constptD_[nMaxParticles];
  Float_t constphiD_[nMaxParticles];
  Float_t constetaD_[nMaxParticles];
  Int_t constjtpos_[nMaxParticles];

  Int_t centRC;
  Float_t centWeight;
  Float_t centWeightJustNColl;
  
  if(paramFound["IMBINPUT"].size() == 0){
    std::cout << "Given config \'" << paramFileName << "\' must define IMBINPUT. return 1" << std::endl;
    return 1;
  }
  if(paramFound["CENTINPUT"].size() == 0 && doNCollWeights){
    std::cout << "Given config \'" << paramFileName << "\' must define CENTINPUT. return 1" << std::endl;
    return 1;
  }

  //Create an output filename
  std::string outFileName = inFileName;
  while(outFileName.find("/") != std::string::npos){outFileName.replace(0, outFileName.find("/")+1, "");}
  if(outFileName.find(".") != std::string::npos) outFileName = outFileName.substr(0, outFileName.rfind("."));
  checkMakeDir("output");
  checkMakeDir("output/" + dateStr);  
  outFileName = "output/" + dateStr + "/" + outFileName + "_" + rStr + "PF_IMBForest_" + dateStr + ".root";  

  //Declare outFile, outTree, and tnamed
  TFile* outFile_p = new TFile(outFileName.c_str(), "RECREATE");
  TDirectoryFile* imbParamDir_p = (TDirectoryFile*)outFile_p->mkdir("imbParamDir");

  TTree* outTree_p = new TTree("imbTree", "");

  if(doNCollWeights){
    outTree_p->Branch("cent", &centRC, "cent/I");
    outTree_p->Branch("centWeight", &centWeight, "centWeight/F");
    outTree_p->Branch("centWeightJustNColl", &centWeightJustNColl, "centWeightJustNColl/F");
  }

  if(imbPthatStr.size() != 0){
    outTree_p->Branch("pthat", &pthat_, "pthat/F");
    outTree_p->Branch("pthatWeight", &pthatWeight_, "pthatWeight/F");
  }

  outTree_p->Branch("nref", &nref_, "nref/I");
  outTree_p->Branch("jtpt", jtptF_, "jtpt[nref]/F");
  outTree_p->Branch("jtphi", jtphiF_, "jtphi[nref]/F");
  outTree_p->Branch("jteta", jtetaF_, "jteta[nref]/F");
  if(imbRefPtStr.size() != 0) outTree_p->Branch("refpt", refptF_, "refpt[nref]/F");
  outTree_p->Branch("jtweight", jtweightF_, "jtweight[nref]/F");

  if(imbConstPtStr.size() != 0){
    outTree_p->Branch("nconst", &nconst_, "nconst/I");
    outTree_p->Branch("constpt", constptF_, "constpt[nconst]/F");
    outTree_p->Branch("constphi", constphiF_, "constphi[nconst]/F");
    outTree_p->Branch("consteta", constetaF_, "consteta[nconst]/F");
    outTree_p->Branch("constjtpos", constjtpos_, "constjtpos[nconst]/I");
  }
  
  std::cout << __FILE__ << ", " << __LINE__ << std::endl;

  std::vector<std::string> imbBranchNames = {imbPtStr, imbPhiStr, imbEtaStr};
  if(imbRefPtStr.size() != 0) imbBranchNames.push_back(imbRefPtStr);
  if(imbConstPtStr.size() != 0){
    imbBranchNames.push_back(imbConstPtStr);
    imbBranchNames.push_back(imbConstPhiStr);
    imbBranchNames.push_back(imbConstEtaStr);
    imbBranchNames.push_back(imbConstjtposStr);
  }

  if(imbWeightStr.size() != 0) imbBranchNames.push_back(imbWeightStr);
  if(imbPthatStr.size() != 0) imbBranchNames.push_back(imbPthatStr);
  if(!isVectorImb){   
    imbBranchNames.push_back(imbNStr);
    if(imbConstPtStr.size() != 0) imbBranchNames.push_back(imbConstNStr);
  }
    
  TFile* inFile_p = new TFile(inFileName.c_str(), "READ");
  std::vector<std::string> ttrees = returnRootFileContentsList(inFile_p, "TTree");
  bool imbIsFound = false;
  bool centIsFound = false;
  for(auto const & iter : ttrees){
    if(isStrSame(imbInStr, iter)) imbIsFound = true;
    if(isStrSame(centInStr, iter)) centIsFound = true;
    
    if(imbIsFound && centIsFound) break;
  }

  if(!imbIsFound){
    std::cout << "INPUT TREE FOR IMB \'" << imbInStr << "\' is not found. return 1" << std::endl;
    return 1;
  }
  if(!centIsFound && doNCollWeights){
    std::cout << "INPUT TREE FOR CENT \'" << centInStr << "\' is not found. return 1" << std::endl;
    return 1;
  }

  if(!checkFile(centTableStr) && doNCollWeights){
    std::cout << "CentTableStr not found return 1" << std::endl;
    return 1;
  }
  centralityFromInput centTable;
  if(doNCollWeights) centTable.SetTable(centTableStr);
  
  TTree* imbTreeIn_p = (TTree*)inFile_p->Get(imbInStr.c_str());
  TObjArray* imbListOfBranchesArr = imbTreeIn_p->GetListOfBranches();
  std::map<std::string, bool> imbListOfBranchesMap;
  for(Int_t bI = 0; bI < imbListOfBranchesArr->GetEntries(); ++bI){
    imbListOfBranchesMap[imbListOfBranchesArr->At(bI)->GetName()] = true;
  }
  
  imbTreeIn_p->SetBranchStatus("*", 0);
  for(auto const & iter : imbBranchNames){
    std::cout << __FILE__ << ", " << __LINE__ << ", " << iter << std::endl;

    if(iter.size() == 0) continue;
    
    if(imbListOfBranchesMap.count(iter) == 0){
      std::cout << "Requested branch \'" << iter << "\' not found in ttree \'" << imbInStr << "\'. please check. return 1" << std::endl;
      return 1;
    }
    imbTreeIn_p->SetBranchStatus(iter.c_str(), 1);
  }

  
  TTree* centTreeIn_p = NULL;
  TObjArray* centListOfBranchesArr = NULL;
  std::map<std::string, bool> centListOfBranchesMap;

  if(doNCollWeights){
    centTreeIn_p = (TTree*)inFile_p->Get(centInStr.c_str());
    centListOfBranchesArr = centTreeIn_p->GetListOfBranches();
    for(Int_t bI = 0; bI < centListOfBranchesArr->GetEntries(); ++bI){
      centListOfBranchesMap[centListOfBranchesArr->At(bI)->GetName()] = true;
    }
  

    centTreeIn_p->SetBranchStatus("*", 0);
    for(auto const & iter : centVarStr){
      if(centListOfBranchesMap.count(iter) == 0){
	std::cout << "Requested branch \'" << iter << "\' not found in ttree \'" << centInStr << "\'. please check. return 1" << std::endl;
	return 1;
      }
      centTreeIn_p->SetBranchStatus(iter.c_str(), 1);
    }
  }

  
  if(imbPthatStr.size() != 0) imbTreeIn_p->SetBranchAddress(imbPthatStr.c_str(), &pthat_);

  if(isVectorImb){
    if(isDoubleImb){
      imbTreeIn_p->SetBranchAddress(imbPtStr.c_str(), &jtptD_p);
      imbTreeIn_p->SetBranchAddress(imbPhiStr.c_str(), &jtphiD_p);
      imbTreeIn_p->SetBranchAddress(imbEtaStr.c_str(), &jtetaD_p);
      if(imbRefPtStr.size() != 0) imbTreeIn_p->SetBranchAddress(imbRefPtStr.c_str(), &refptD_p);

      if(imbWeightStr.size() != 0) imbTreeIn_p->SetBranchAddress(imbWeightStr.c_str(), &jtweightD_p);

      if(imbConstPtStr.size() != 0){
	imbTreeIn_p->SetBranchAddress(imbConstPtStr.c_str(), &constptD_p);
	imbTreeIn_p->SetBranchAddress(imbConstPhiStr.c_str(), &constphiD_p);
	imbTreeIn_p->SetBranchAddress(imbConstEtaStr.c_str(), &constetaD_p);
	imbTreeIn_p->SetBranchAddress(imbConstjtposStr.c_str(), &constjtpos_p);
      }
    }
    else{
      imbTreeIn_p->SetBranchAddress(imbPtStr.c_str(), &jtptF_p);
      imbTreeIn_p->SetBranchAddress(imbPhiStr.c_str(), &jtphiF_p);
      imbTreeIn_p->SetBranchAddress(imbEtaStr.c_str(), &jtetaF_p);
      if(imbRefPtStr.size() != 0) imbTreeIn_p->SetBranchAddress(imbRefPtStr.c_str(), &refptF_p);

      if(imbWeightStr.size() != 0) imbTreeIn_p->SetBranchAddress(imbWeightStr.c_str(), &jtweightF_p);

      if(imbConstPtStr.size() != 0){
	imbTreeIn_p->SetBranchAddress(imbConstPtStr.c_str(), &constptF_p);
	imbTreeIn_p->SetBranchAddress(imbConstPhiStr.c_str(), &constphiF_p);
	imbTreeIn_p->SetBranchAddress(imbConstEtaStr.c_str(), &constetaF_p);
	imbTreeIn_p->SetBranchAddress(imbConstjtposStr.c_str(), &constjtpos_p);
      }
    }
  }
  else{
    imbTreeIn_p->SetBranchAddress(imbNStr.c_str(), &nref_);
    if(imbConstPtStr.size() != 0) imbTreeIn_p->SetBranchAddress(imbConstNStr.c_str(), &nconst_);

    if(isDoubleImb){
      imbTreeIn_p->SetBranchAddress(imbPtStr.c_str(), jtptD_);
      imbTreeIn_p->SetBranchAddress(imbPhiStr.c_str(), jtphiD_);
      imbTreeIn_p->SetBranchAddress(imbEtaStr.c_str(), jtetaD_);
      if(imbRefPtStr.size() != 0) imbTreeIn_p->SetBranchAddress(imbRefPtStr.c_str(), refptD_);
      if(imbWeightStr.size() != 0) imbTreeIn_p->SetBranchAddress(imbWeightStr.c_str(), jtweightD_);

      if(imbConstPtStr.size() != 0){
	imbTreeIn_p->SetBranchAddress(imbConstPtStr.c_str(), constptD_);
	imbTreeIn_p->SetBranchAddress(imbConstPhiStr.c_str(), constphiD_);
	imbTreeIn_p->SetBranchAddress(imbConstEtaStr.c_str(), constetaD_);

	imbTreeIn_p->SetBranchAddress(imbConstjtposStr.c_str(), constjtpos_);
      }
    }
    else{
      imbTreeIn_p->SetBranchAddress(imbPtStr.c_str(), jtptF_);
      imbTreeIn_p->SetBranchAddress(imbPhiStr.c_str(), jtphiF_);
      imbTreeIn_p->SetBranchAddress(imbEtaStr.c_str(), jtetaF_);
      if(imbRefPtStr.size() != 0) imbTreeIn_p->SetBranchAddress(imbRefPtStr.c_str(), refptF_);
      if(imbWeightStr.size() != 0) imbTreeIn_p->SetBranchAddress(imbWeightStr.c_str(), jtweightF_);

      if(imbConstPtStr.size() != 0){
	imbTreeIn_p->SetBranchAddress(imbConstPtStr.c_str(), constptF_);
	imbTreeIn_p->SetBranchAddress(imbConstPhiStr.c_str(), constphiF_);
	imbTreeIn_p->SetBranchAddress(imbConstEtaStr.c_str(), constetaF_);
	imbTreeIn_p->SetBranchAddress(imbConstjtposStr.c_str(), constjtpos_);
      }
    }
  }


  if(doNCollWeights){
    if(isStrSame(centVarType, "int")){
      for(unsigned int cI = 0; cI < centVarStr.size(); ++cI){
	centVarI.push_back(0.0);
	centTreeIn_p->SetBranchAddress(centVarStr[cI].c_str(), &(centVarI[centVarI.size()-1]));
      }
    }
    else if(isStrSame(centVarType, "float")){
      for(unsigned int cI = 0; cI < centVarStr.size(); ++cI){
	centVarF.push_back(0.0);
	centTreeIn_p->SetBranchAddress(centVarStr[cI].c_str(), &(centVarF[centVarF.size()-1]));
      }
    }
    else if(isStrSame(centVarType, "double")){
      for(unsigned int cI = 0; cI < centVarStr.size(); ++cI){
	centVarD.push_back(0.0);
	centTreeIn_p->SetBranchAddress(centVarStr[cI].c_str(), &(centVarD[centVarD.size()-1]));
      }
    }
    else{
      std::cout << "CENTVARTYPE \'" << centVarType << "\' not found. return 1" << std::endl;
      return 1;
    }
  }
  
  const Int_t nEntries = imbTreeIn_p->GetEntries();
  const Int_t nDiv = TMath::Max(1, nEntries/20);

  std::vector<double> centVals, centWeights, centWeightsJustNColl;

  if(doNCollWeights){
    for(Int_t cI = 0; cI < 100; ++cI){
      centVals.push_back(0.0);
    }
    std::cout << "Pre-Processing " << nEntries << "..." << std::endl;

    for(Int_t entry = 0; entry < nEntries; ++entry){
      if(entry%nDiv == 0) std::cout << " Entry " << entry << "/" << nEntries << std::endl;
      centTreeIn_p->GetEntry(entry);
      int centVal = 0;

      for(unsigned int cI = 0; cI < centVarStr.size(); ++cI){
        if(isStrSame(centVarType, "int")) centVal += centVarI[cI];
        else if(isStrSame(centVarType, "float")) centVal += centVarF[cI];
        else if(isStrSame(centVarType, "double")) centVal += centVarD[cI];
      }
      centRC = (Int_t)centTable.getCent(centVal);

      ++(centVals[centRC]);
    }

    for(Int_t cI = 0; cI < 100; ++cI){
      double centWeightVal = (findNcoll_Renorm(cI*2) + findNcoll_Renorm(cI*2+1))/2.;
      centWeightsJustNColl.push_back(centWeightVal);
      centWeightVal /= centVals[cI];
      centWeights.push_back(centWeightVal);
    }
  }
  
  
  std::cout << "Processing " << nEntries << "..." << std::endl;
  for(Int_t entry = 0; entry < nEntries; ++entry){
    if(entry%nDiv == 0) std::cout << " Entry " << entry << "/" << nEntries << std::endl;
    imbTreeIn_p->GetEntry(entry);
    if(doNCollWeights) centTreeIn_p->GetEntry(entry);

    double centVal = 0;
    centWeight = 1.0;
    centWeightJustNColl = 1.0;
    
    if(doNCollWeights){
      for(unsigned int cI = 0; cI < centVarStr.size(); ++cI){
	if(isStrSame(centVarType, "int")) centVal += centVarI[cI];
	else if(isStrSame(centVarType, "float")) centVal += centVarF[cI];
	else if(isStrSame(centVarType, "double")) centVal += centVarD[cI];
      }
      centRC = centTable.getCent(centVal);
      centWeight = centWeights[centRC];
      centWeightJustNColl = centWeightsJustNColl[centRC];
    }
    
    if(isVectorImb){
      nref_ = 0;

      if(isDoubleImb){
        for(unsigned int pI = 0; pI < jtptD_p->size(); ++pI){
          jtptF_[nref_] = (Float_t)jtptD_p->at(pI);
          jtphiF_[nref_] = (Float_t)jtphiD_p->at(pI);
          jtetaF_[nref_] = (Float_t)jtetaD_p->at(pI);
	  if(imbRefPtStr.size() != 0) refptF_[nref_] = (Float_t)refptD_p->at(pI);
	  
          if(imbWeightStr.size() != 0) jtweightF_[nref_] = (Float_t)jtweightD_p->at(pI);
	  else jtweightF_[nref_] = 1.0;

	  jtweightF_[nref_] = centWeight;
	  
          ++nref_;
        }
      }	
      else{
        for(unsigned int pI = 0; pI < jtptF_p->size(); ++pI){
          jtptF_[nref_] = (Float_t)jtptF_p->at(pI);
          jtphiF_[nref_] = (Float_t)jtphiF_p->at(pI);
          jtetaF_[nref_] = (Float_t)jtetaF_p->at(pI);
	  if(imbRefPtStr.size() != 0) refptF_[nref_] = (Float_t)refptF_p->at(pI);

          if(imbWeightStr.size() != 0) jtweightF_[nref_] = (Float_t)jtweightF_p->at(pI);
	  else jtweightF_[nref_] = 1.0;

	  jtweightF_[nref_] = centWeight;

          ++nref_;
	}
      }
    }
    else if(isDoubleImb){
      for(Int_t pI = 0; pI < nref_; ++pI){
        jtptF_[pI] = (Float_t)jtptD_[pI];
        jtphiF_[pI] = (Float_t)jtphiD_[pI];
        jtetaF_[pI] = (Float_t)jtetaD_[pI];
	if(imbRefPtStr.size() != 0) refptF_[nref_] = (Float_t)refptD_[pI];

	jtweightF_[nref_] = centWeight;

        if(imbWeightStr.size() != 0) jtweightF_[pI] = (Float_t)jtweightD_[pI];
	else jtweightF_[nref_] = 1.0;
      }
    }
    else if(imbWeightStr.size() == 0){
      for(Int_t pI = 0; pI < nref_; ++pI){
	jtweightF_[pI] = centWeight;
      }
    }

    if(imbPthatStr.size() != 0){
      if(pthats.size() != 0){
	
	Int_t pthatPos = -1;
	for(unsigned int pI = 0; pI < pthats.size()-1; ++pI){
	  if(pthat_ >= pthats[pI] && pthat_ < pthats[pI+1]){
	    pthatPos = pI;
	    break;
	  }
	}
	if(pthatPos < 0){
	  if(pthat_ > pthats[pthats.size()-1]) pthatPos = pthats.size()-1;
	  else{
	    std::cout << "WARNING: pthat \'" << pthat_ << "\' is not found in pthats: ";
	    for(unsigned int pI = 0; pI < pthats.size()-1; ++pI){
	      std::cout << pthats[pI] << ", ";
	    }
	    std::cout << pthats[pthats.size()-1] << "." << std::endl;
	  }
	}
	
	if(pthatPos >= 0){
	  pthatWeight_ = pthatWeights[pthatPos];
	  for(Int_t pI = 0; pI < nref_; ++pI){
	    jtweightF_[pI] *= pthatWeights[pthatPos];
	  }	  
	}
      }
      else return 1;
    }
    
    if(isVectorImb && imbConstPtStr.size() != 0){
      nconst_ = 0;
      
      if(isDoubleImb){
	for(unsigned int pI = 0; pI < constptD_p->size(); ++pI){
	  constptF_[nconst_] = (Float_t)constptD_p->at(pI);
	  constphiF_[nconst_] = (Float_t)constphiD_p->at(pI);
	  constetaF_[nconst_] = (Float_t)constetaD_p->at(pI);
	  
	  ++nconst_;
	}
      }	
      else{
	for(unsigned int pI = 0; pI < constptF_p->size(); ++pI){
	  constptF_[nconst_] = (Float_t)constptF_p->at(pI);
	  constphiF_[nconst_] = (Float_t)constphiF_p->at(pI);
	  constetaF_[nconst_] = (Float_t)constetaF_p->at(pI);
	  
	  ++nconst_;
	}
      }
    }
    else if(isDoubleImb){
      for(Int_t pI = 0; pI < nconst_; ++pI){
	constptF_[pI] = (Float_t)constptD_[pI];
	constphiF_[pI] = (Float_t)constphiD_[pI];
	constetaF_[pI] = (Float_t)constetaD_[pI];
      }
    }
    
    outTree_p->Fill();
  }
  
  inFile_p->Close();
  delete inFile_p;

  outFile_p->cd();

  std::cout << __FILE__ << ", " << __LINE__ << std::endl;

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

  return 0;
}


int main(int argc, char* argv[])
{
  if(argc != 3){
    std::cout << "Usage: processForestForImb.exe <inFileName> <paramFileName>. return 1" << std::endl;
    return 1;
  }
  
  int retVal = 0;
  retVal += processForestForImb(argv[1], argv[2]);
  return retVal;
}
