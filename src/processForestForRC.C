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
#include "TLorentzVector.h"
#include "TNamed.h"
#include "TRandom3.h"
#include "TTree.h"

//Local
#include "include/centralityFromInput.h"
#include "include/checkMakeDir.h"
#include "include/etaPhiFunc.h"
#include "include/ncollFunctions_5TeV.h"
#include "include/paramPropagator.h"
#include "include/plotUtilities.h"
#include "include/returnRootFileContentsList.h"
#include "include/stringUtil.h"

int processForestForRC(std::string inFileName, std::string paramFileName)
{
  //Declare RNG for random cone positions
  TRandom3* randGen_p = new TRandom3(0);

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

  const Int_t nEtaBinsMax = 10;
  const Int_t nCentBinsMax = 10;

  Int_t nEtaBins = params.getNEtaBins();
  Double_t etaBinsLow[nEtaBinsMax];
  Double_t etaBinsHigh[nEtaBinsMax];
  params.getEtaBinsLow(etaBinsLow);
  params.getEtaBinsHigh(etaBinsHigh);
  
  Int_t nCentBins = params.getNCentBins();
  Double_t centBinsLow[nCentBinsMax];
  Double_t centBinsHigh[nCentBinsMax];
  params.getCentBinsLow(centBinsLow);
  params.getCentBinsHigh(centBinsHigh);

  if(!params.checkNEtaBinsMax(nEtaBinsMax)) return 1; 
  if(!params.checkNCentBinsMax(nCentBinsMax)) return 1;
 
  Int_t nRC = params.getNRC();
  Double_t rcR = params.getRCR();
  Double_t ptCut = params.getPtCut();

  Bool_t doNCollWeights = params.getDoNCollWeights();
  
  std::string rcInStr = params.getRCInStr();
  bool isVectorRC = params.getIsVectorRC();
  bool isDoubleRC = params.getIsDoubleRC();
  std::string rcNStr = params.getRCNStr();
  std::string rcPtStr = params.getRCPtStr();
  std::string rcPhiStr = params.getRCPhiStr();
  std::string rcEtaStr = params.getRCEtaStr();

  //Cent tree vars
  std::string centInStr = params.getCentInStr();
  std::string centTableStr = params.getCentTableStr();
  std::string centVarType = params.getCentVarType();
  std::vector<std::string> centVarStr = params.getCentVarStr();
  std::vector<double> centVarD;
  std::vector<float> centVarF;
  std::vector<int> centVarI;

  const Int_t nRhoMC = params.getNRhoMC();
  std::cout << "nRhoMC: " << nRhoMC << std::endl;
  
  //Rho tree vars
  std::string rhoInStr = params.getRhoInStr();
  bool isVectorRho = params.getIsVectorRho();
  bool isDoubleRho = params.getIsDoubleRho();
  std::string nRhoStr = params.getNRhoStr();
  std::string rhoStr = params.getRhoStr();
  std::string rhoEtaMinStr = params.getRhoEtaMinStr();
  std::string rhoEtaMaxStr = params.getRhoEtaMaxStr();

  std::vector<std::string> rhoVarStr = {rhoStr, rhoEtaMinStr};
  if(!isVectorRho) rhoVarStr.push_back(nRhoStr);
  if(rhoEtaMaxStr.size() != 0) rhoVarStr.push_back(rhoEtaMaxStr);
  
  //VECTOR IF RC INPUT IS VECTOR FLOAT OR DOUBLE
  std::vector<float>* ptF_p=NULL;
  std::vector<float>* phiF_p=NULL;
  std::vector<float>* etaF_p=NULL;
  
  std::vector<double>* ptD_p=NULL;
  std::vector<double>* phiD_p=NULL;
  std::vector<double>* etaD_p=NULL;

  //ARRAY IF RC INPUT IS ARRAY FLOAT OR DOUBLE
  const Int_t nMaxPart = 50000;
  Int_t nPart_;
  Float_t ptF_[nMaxPart];
  Float_t phiF_[nMaxPart];
  Float_t etaF_[nMaxPart];

  Double_t ptD_[nMaxPart];
  Double_t phiD_[nMaxPart];
  Double_t etaD_[nMaxPart];

  //VECTOR IF Rho INPUT IS VECTOR FLOAT OR DOUBLE
  std::vector<float>* rhoF_p=NULL;
  std::vector<float>* rhoEtaMinF_p=NULL;
  std::vector<float>* rhoEtaMaxF_p=NULL;
  
  std::vector<double>* rhoD_p=NULL;
  std::vector<double>* rhoEtaMinD_p=NULL;
  std::vector<double>* rhoEtaMaxD_p=NULL;

  //ARRAY IF RC INPUT IS ARRAY FLOAT OR DOUBLE
  const Int_t nMaxRho = 500;
  Int_t nRho_;
  Float_t rhoF_[nMaxRho];
  Float_t rhoEtaMinF_[nMaxRho];
  Float_t rhoEtaMaxF_[nMaxRho];

  Double_t rhoD_[nMaxRho];
  Double_t rhoEtaMinD_[nMaxRho];
  Double_t rhoEtaMaxD_[nMaxRho];

  //PROCESS INPUT PARAMS FILE
  params.getNEtaBins();
  
  if(paramFound["RCINPUT"].size() == 0){
    std::cout << "Given config \'" << paramFileName << "\' must define RCINPUT. return 1" << std::endl;
    return 1;
  }
  if(paramFound["CENTINPUT"].size() == 0){
    std::cout << "Given config \'" << paramFileName << "\' must define CENTINPUT. return 1" << std::endl;
    return 1;
  }
  if(paramFound["RHOINPUT"].size() == 0){
    std::cout << "Given config \'" << paramFileName << "\' must define RHOINPUT. return 1" << std::endl;
    return 1;
  }

  Double_t etaMax = -100;
  for(Int_t eI = 0; eI < nEtaBins; ++eI){
    if(etaMax < etaBinsHigh[eI]) etaMax = etaBinsHigh[eI];
  }
  Double_t etaMin = -etaMax;

  //std::cout << "LINE: " << __LINE__ << std::endl;

  //Create an output filename
  std::string outFileName = inFileName;
  while(outFileName.find("/") != std::string::npos){outFileName.replace(0, outFileName.find("/")+1, "");}
  if(outFileName.find(".") != std::string::npos) outFileName = outFileName.substr(0, outFileName.rfind("."));
  checkMakeDir("output");
  checkMakeDir("output/" + dateStr);  
  outFileName = "output/" + dateStr + "/" + outFileName + "_RCR" + prettyString(rcR, 1, true) + "_PtCut" + prettyString(ptCut, 2, true) + "_RCForest_" + dateStr + ".root";  

  //Declare outFile, outTree, and tnamed
  TFile* outFile_p = new TFile(outFileName.c_str(), "RECREATE");
  TTree* rcTreeOut_p = new TTree("rcTree", "");
  TDirectoryFile* rcParamDir_p = (TDirectoryFile*)outFile_p->mkdir("rcParamDir");

  UInt_t outEntry, etaPos = 0;
  Int_t centRC, centPos;
  Float_t weight, etaRC, phiRC, ptRC, ptRhoRC, ptSubRC, pt4VecRC, pt4VecRhoRC, pt4VecSubRC;
  
  rcTreeOut_p->Branch("outEntry", &outEntry, "outEntry/i");
  rcTreeOut_p->Branch("weight", &weight, "weight/F");
  rcTreeOut_p->Branch("etaPos", &etaPos, "etaPos/i");
  rcTreeOut_p->Branch("centPos", &centPos, "centPos/I");
  rcTreeOut_p->Branch("centRC", &centRC, "centRC/I");
  rcTreeOut_p->Branch("etaRC", &etaRC, "etaRC/F");
  rcTreeOut_p->Branch("phiRC", &phiRC, "phiRC/F");
  rcTreeOut_p->Branch("ptRC", &ptRC, "ptRC/F");
  rcTreeOut_p->Branch("ptRhoRC", &ptRhoRC, "ptRhoRC/F");
  rcTreeOut_p->Branch("ptSubRC", &ptSubRC, "ptSubRC/F");
  rcTreeOut_p->Branch("pt4VecRC", &pt4VecRC, "pt4VecRC/F");
  rcTreeOut_p->Branch("pt4VecRhoRC", &pt4VecRhoRC, "pt4VecRhoRC/F");
  rcTreeOut_p->Branch("pt4VecSubRC", &pt4VecSubRC, "pt4VecSubRC/F");
  
  std::vector<std::string> rcBranchNames = {rcPtStr, rcPhiStr, rcEtaStr};
  if(!isVectorRC) rcBranchNames.push_back(rcNStr);
  
  //Open file and check that our inputs are correct  
  TFile* inFile_p = new TFile(inFileName.c_str(), "READ");
  std::vector<std::string> ttrees = returnRootFileContentsList(inFile_p, "TTree");
  bool rcIsFound = false;
  bool centIsFound = false;
  for(auto const & iter : ttrees){
    if(isStrSame(rcInStr, iter)) rcIsFound = true;
    if(isStrSame(centInStr, iter)) centIsFound = true;

    if(rcIsFound && centIsFound) break;
  }

  if(!rcIsFound){
    std::cout << "INPUT TREE FOR RC \'" << rcInStr << "\' is not found. return 1" << std::endl;
    return 1;
  }
  if(!centIsFound){
    std::cout << "INPUT TREE FOR CENT \'" << centInStr << "\' is not found. return 1" << std::endl;
    return 1;
  }

  centralityFromInput centTable(centTableStr);

  TTree* rcTreeIn_p = (TTree*)inFile_p->Get(rcInStr.c_str());
  TObjArray* rcListOfBranchesArr = rcTreeIn_p->GetListOfBranches();
  std::map<std::string, bool> rcListOfBranchesMap;
  for(Int_t bI = 0; bI < rcListOfBranchesArr->GetEntries(); ++bI){
    rcListOfBranchesMap[rcListOfBranchesArr->At(bI)->GetName()] = true;
  }

  rcTreeIn_p->SetBranchStatus("*", 0);
  for(auto const & iter : rcBranchNames){
    if(rcListOfBranchesMap.count(iter) == 0){
      std::cout << "Requested branch \'" << iter << "\' not found in ttree \'" << rcInStr << "\'. please check. return 1" << std::endl;
      return 1;
    }
    rcTreeIn_p->SetBranchStatus(iter.c_str(), 1);
  }

  TTree* centTreeIn_p = (TTree*)inFile_p->Get(centInStr.c_str());
  TObjArray* centListOfBranchesArr = centTreeIn_p->GetListOfBranches();
  std::map<std::string, bool> centListOfBranchesMap;
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

  TTree* rhoTreeIn_p = (TTree*)inFile_p->Get(rhoInStr.c_str());
  TObjArray* rhoListOfBranchesArr = rhoTreeIn_p->GetListOfBranches();
  std::map<std::string, bool> rhoListOfBranchesMap;
  for(Int_t bI = 0; bI < rhoListOfBranchesArr->GetEntries(); ++bI){
    rhoListOfBranchesMap[rhoListOfBranchesArr->At(bI)->GetName()] = true;
  }

  rhoTreeIn_p->SetBranchStatus("*", 0);
  for(auto const & iter : rhoVarStr){
    if(rhoListOfBranchesMap.count(iter) == 0){
      std::cout << "Requested branch \'" << iter << "\' not found in ttree \'" << rhoInStr << "\'. please check. return 1" << std::endl;
      return 1;
    }
    rhoTreeIn_p->SetBranchStatus(iter.c_str(), 1);
  }
  
  
  if(isVectorRC){
    if(isDoubleRC){
      rcTreeIn_p->SetBranchAddress(rcPtStr.c_str(), &ptD_p);
      rcTreeIn_p->SetBranchAddress(rcPhiStr.c_str(), &phiD_p);
      rcTreeIn_p->SetBranchAddress(rcEtaStr.c_str(), &etaD_p);
    }
    else{
      rcTreeIn_p->SetBranchAddress(rcPtStr.c_str(), &ptF_p);
      rcTreeIn_p->SetBranchAddress(rcPhiStr.c_str(), &phiF_p);
      rcTreeIn_p->SetBranchAddress(rcEtaStr.c_str(), &etaF_p);
    }
  }
  else{
    rcTreeIn_p->SetBranchAddress(rcNStr.c_str(), &nPart_);

    if(isDoubleRC){
      rcTreeIn_p->SetBranchAddress(rcPtStr.c_str(), ptD_);
      rcTreeIn_p->SetBranchAddress(rcPhiStr.c_str(), phiD_);
      rcTreeIn_p->SetBranchAddress(rcEtaStr.c_str(), etaD_);
    }
    else{
      rcTreeIn_p->SetBranchAddress(rcPtStr.c_str(), ptF_);
      rcTreeIn_p->SetBranchAddress(rcPhiStr.c_str(), phiF_);
      rcTreeIn_p->SetBranchAddress(rcEtaStr.c_str(), etaF_);
    }
  }

  if(isVectorRho){
    if(isDoubleRho){
      rhoTreeIn_p->SetBranchAddress(rhoStr.c_str(), &rhoD_p);
      rhoTreeIn_p->SetBranchAddress(rhoEtaMinStr.c_str(), &rhoEtaMinD_p);
      if(rhoEtaMaxStr.size() != 0) rhoTreeIn_p->SetBranchAddress(rhoEtaMaxStr.c_str(), &rhoEtaMaxD_p);
    }
    else{
      rhoTreeIn_p->SetBranchAddress(rhoStr.c_str(), &rhoF_p);
      rhoTreeIn_p->SetBranchAddress(rhoEtaMinStr.c_str(), &rhoEtaMinF_p);
      if(rhoEtaMaxStr.size() != 0) rhoTreeIn_p->SetBranchAddress(rhoEtaMaxStr.c_str(), &rhoEtaMaxF_p);
    }
  }
  else{
    rhoTreeIn_p->SetBranchAddress(nRhoStr.c_str(), &nRho_);

    if(isDoubleRho){
      rhoTreeIn_p->SetBranchAddress(rhoStr.c_str(), rhoD_);
      rhoTreeIn_p->SetBranchAddress(rhoEtaMinStr.c_str(), rhoEtaMinD_);
      if(rhoEtaMaxStr.size() != 0) rhoTreeIn_p->SetBranchAddress(rhoEtaMaxStr.c_str(), rhoEtaMaxD_);
    }
    else{
      rhoTreeIn_p->SetBranchAddress(rhoStr.c_str(), rhoF_);
      rhoTreeIn_p->SetBranchAddress(rhoEtaMinStr.c_str(), rhoEtaMinF_);
      if(rhoEtaMaxStr.size() != 0) rhoTreeIn_p->SetBranchAddress(rhoEtaMaxStr.c_str(), rhoEtaMaxF_);
    }
  }

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

  const Int_t nEntries = rcTreeIn_p->GetEntries();
  const Int_t nDiv = TMath::Max(1, nEntries/200);

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
  
  const Double_t rhoArea = TMath::Pi()*rcR*rcR/((Double_t)nRhoMC);
  
  std::cout << "Processing " << nEntries << "..." << std::endl;
  for(Int_t entry = 0; entry < nEntries; ++entry){
    if(entry%nDiv == 0) std::cout << " Entry " << entry << "/" << nEntries << std::endl;
    rcTreeIn_p->GetEntry(entry);
    centTreeIn_p->GetEntry(entry);
    rhoTreeIn_p->GetEntry(entry);

    outEntry = entry;

    //Lets reassign values to float array for later simplicity
    if(isVectorRC){
      nPart_ = 0;

      if(isDoubleRC){
	for(unsigned int pI = 0; pI < ptD_p->size(); ++pI){
	  ptF_[nPart_] = (Float_t)ptD_p->at(pI);
	  phiF_[nPart_] = (Float_t)phiD_p->at(pI);
	  etaF_[nPart_] = (Float_t)etaD_p->at(pI);
	  
	  ++nPart_;
	}
      }
      else{
	for(unsigned int pI = 0; pI < ptF_p->size(); ++pI){
	  ptF_[nPart_] = (Float_t)ptF_p->at(pI);
	  phiF_[nPart_] = (Float_t)phiF_p->at(pI);
	  etaF_[nPart_] = (Float_t)etaF_p->at(pI);
	  
	  ++nPart_;
	}
      }
    }
    else if(isDoubleRC){
      for(Int_t pI = 0; pI < nPart_; ++pI){
	ptF_[pI] = (Float_t)ptD_[pI];
	phiF_[pI] = (Float_t)phiD_[pI];
	etaF_[pI] = (Float_t)etaD_[pI];
      }
    }

    if(isVectorRho){
      nRho_ = 0;

      if(isDoubleRho){
	for(unsigned int pI = 0; pI < rhoD_p->size(); ++pI){
	  rhoF_[nRho_] = (Float_t)rhoD_p->at(pI);
	  rhoEtaMinF_[nRho_] = (Float_t)rhoEtaMinD_p->at(pI);
	  if(rhoEtaMaxStr.size() != 0) rhoEtaMaxF_[nRho_] = (Float_t)rhoEtaMaxD_p->at(pI);
	  
	  ++nRho_;
	}
      }
      else{
	for(unsigned int pI = 0; pI < rhoF_p->size(); ++pI){
	  rhoF_[nRho_] = (Float_t)rhoF_p->at(pI);
	  rhoEtaMinF_[nRho_] = (Float_t)rhoEtaMinF_p->at(pI);
	  if(rhoEtaMaxStr.size() != 0) rhoEtaMaxF_[nRho_] = (Float_t)rhoEtaMaxF_p->at(pI);
	  
	  ++nRho_;
	}
      }
    }
    else{
      for(Int_t pI = 0; pI < nRho_; ++pI){
	rhoF_[pI] = (Float_t)rhoD_[pI];
	rhoEtaMinF_[pI] = (Float_t)rhoEtaMinD_[pI];
	if(rhoEtaMaxStr.size() != 0) rhoEtaMaxF_[pI] = (Float_t)rhoEtaMaxD_[pI];
      }
    }

    if(rhoEtaMaxStr.size() == 0){
      for(Int_t rI = 1; rI < nRho_+1; ++rI){
	rhoEtaMaxF_[rI-1] = rhoEtaMinF_[rI];
      }
    }
    
    std::vector<float> etaVals, phiVals;

    while(nRC > (Int_t)etaVals.size()){
      Double_t tempEta = randGen_p->Uniform(etaMin, etaMax);     
      Double_t tempPhi = randGen_p->Uniform(-TMath::Pi(), TMath::Pi());

      bool isGood = true;
      for(unsigned int eI = 0; eI < etaVals.size(); ++eI){
	if(getDR(etaVals[eI], phiVals[eI], tempEta, tempPhi) < rcR){
	  isGood = false;
	  break;
	}
      }

      if(!isGood) continue;

      etaVals.push_back(tempEta);
      phiVals.push_back(tempPhi);
    }

    double centVal = 0;
    for(unsigned int cI = 0; cI < centVarStr.size(); ++cI){
      if(isStrSame(centVarType, "int")) centVal += centVarI[cI];
      else if(isStrSame(centVarType, "float")) centVal += centVarF[cI];
      else if(isStrSame(centVarType, "double")) centVal += centVarD[cI];
    }
   
    centRC = centTable.getCent(centVal);
    weight = 1.0;
    if(doNCollWeights) weight = centWeights[centRC];

    centPos = -1;
    for(Int_t cI = 0; cI < nCentBins; ++cI){
      if(centBinsLow[cI] <= centRC && centRC < centBinsHigh[cI]){       
	centPos = cI;
	break;
      }
    }

    if(centPos < 0) continue;

    for(unsigned int valI = 0; valI < etaVals.size(); ++valI){
      for(Int_t eI = 0; eI < nEtaBins; ++eI){
	if(etaVals[valI] >= TMath::Abs(etaBinsLow[eI]) && etaVals[valI] <= TMath::Abs(etaBinsHigh[eI])){
	  etaPos = eI;
	  break;
	}
      }

      etaRC = etaVals[valI];
      phiRC = phiVals[valI];
      ptRC = 0;
      ptRhoRC = 0;

      TLorentzVector totalParticleTL(0.0, 0.0, 0.0, 0.0);
      TLorentzVector totalRhoTL(0.0, 0.0, 0.0, 0.0);
      TLorentzVector tempTL;
      for(Int_t pI = 0; pI < nPart_; ++pI){
	if(ptF_[pI] < ptCut) continue;
	if(getDR(etaRC, phiRC, etaF_[pI], phiF_[pI]) < rcR){
	  ptRC += ptF_[pI];
	  tempTL.SetPtEtaPhiM(ptF_[pI], etaF_[pI], phiF_[pI], 0.0);
	  totalParticleTL += tempTL;
	}
      }

      Int_t nRhoCounter = 0;
      while(nRhoCounter < nRhoMC){
	Double_t etaRho = randGen_p->Uniform(etaRC - rcR, etaRC + rcR);
	Double_t phiRho = randGen_p->Uniform(phiRC - rcR, phiRC + rcR);
	if(getDR(etaRC, phiRC, etaRho, phiRho) >= rcR) continue;

	Double_t rhoVal = -10000;
	for(Int_t rI = 0; rI < nRho_; ++rI){
	  if(etaRho >= rhoEtaMinF_[rI] && etaRho < rhoEtaMaxF_[rI]){
	    rhoVal = rhoF_[rI];
	    break;
	  }
	}
	if(rhoVal < 0){
	  std::cout << "ERROR: rhoVal \'" << rhoVal << "\' for eta \'" << etaRho << "\' is negative. return 1" <<std::endl;
	  return 1;
	}

	double pTOfEst = rhoArea*rhoVal;
	ptRhoRC += pTOfEst;

	tempTL.SetPtEtaPhiM(pTOfEst, etaRho, phiRho, 0.0);
	totalRhoTL += tempTL;	
	
	++nRhoCounter;
      }

      pt4VecRC = totalParticleTL.Pt();
      pt4VecRhoRC = totalRhoTL.Pt();
      
      ptSubRC = ptRC - ptRhoRC;
      pt4VecSubRC = pt4VecRC - pt4VecRhoRC;
      
      rcTreeOut_p->Fill();
    }
  }
  
  inFile_p->Close();
  delete inFile_p;

  outFile_p->cd();
  rcTreeOut_p->Write("", TObject::kOverwrite);
  delete rcTreeOut_p;

  rcParamDir_p->cd();

  for(auto const & iter : paramFound){
    TNamed tempName(iter.first.c_str(), iter.second.c_str());
    tempName.Write("", TObject::kOverwrite);
  }
  
  outFile_p->Close();
  delete outFile_p;

  delete randGen_p;
  
  return 0;
}


int main(int argc, char* argv[])
{
  if(argc != 3){
    std::cout << "Usage: processForestForRC.exe <inFileName> <paramFileName>. return 1" << std::endl;
    return 1;
  }
  
  int retVal = 0;
  retVal += processForestForRC(argv[1], argv[2]);
  return retVal;
}
