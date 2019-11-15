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

double getPhiFactor(std::vector<double>* rhoFlowFitParams_p, double phiRho)
{
  double par1 = rhoFlowFitParams_p->at(1);
  double par2 = rhoFlowFitParams_p->at(2);
  double par3 = rhoFlowFitParams_p->at(3);
  double par4 = rhoFlowFitParams_p->at(4);

  return 1.0*(1.0 + 2.0*(par1*TMath::Cos(2.*(phiRho - par2)) + par3*TMath::Cos(3.*(phiRho - par4))));
}

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

  const bool doRhoFlow = paramFound["RHOFLOWINPUT"].size() != 0;
  std::string rhoFlowInStr = params.getRhoFlowInStr();
  std::vector<double>* rhoFlowFitParams_p=NULL;
  
  std::vector<std::string> rhoVarStr = {rhoStr, rhoEtaMinStr};
  if(!isVectorRho) rhoVarStr.push_back(nRhoStr);
  if(rhoEtaMaxStr.size() != 0) rhoVarStr.push_back(rhoEtaMaxStr);
  if(doRhoFlow) rhoVarStr.push_back(rhoFlowInStr);

  //Imb in tree
  const bool doJet = paramFound["IMBINPUT"].size() != 0;  
  std::string imbInStr = params.getImbInStr();
  std::cout << "IMB IN STR: " << imbInStr << std::endl;
  bool isVectorImb = params.getIsVectorImb();
  bool isDoubleImb = params.getIsDoubleImb();
  std::string imbPthatStr = params.getPthatValStr();
  std::string imbNStr = params.getImbNStr();
  std::string imbPtStr = params.getImbPtStr();
  std::string imbPhiStr = params.getImbPhiStr();
  std::string imbEtaStr = params.getImbEtaStr();
  std::string imbRefPtStr = params.getImbRefPtStr();
  std::string imbWeightStr = params.getImbWeightStr();

  
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

  std::cout << "OUTFILENAME: " << outFileName << std::endl;
  
  //Declare outFile, outTree, and tnamed
  TFile* outFile_p = new TFile(outFileName.c_str(), "RECREATE");
  TTree* rcTreeOut_p = new TTree("rcTree", "");
  TTree* jetTreeOut_p = NULL;
  if(doJet) jetTreeOut_p = new TTree("jetTree", "");
  TTree* genJetTreeOut_p = NULL;
  if(doJet && imbRefPtStr.size() != 0) genJetTreeOut_p = new TTree("genJetTree", "");
  TDirectoryFile* rcParamDir_p = (TDirectoryFile*)outFile_p->mkdir("rcParamDir");

  UInt_t outEntry, etaPos, etaPosJet, etaPosRef;
  Int_t cent, centPos;
  Float_t weight, etaRC, phiRC, ptRC, ptRhoRC, ptRhoFlowRC, ptSubRC, ptSubFlowRC, pt4VecRC, pt4VecRhoRC, pt4VecRhoFlowRC, pt4VecSubRC, pt4VecSubFlowRC, ptJet, etaJet, phiJet, ptRhoJet, ptRhoFlowJet, pt4VecRhoJet, pt4VecRhoFlowJet, ptRef, etaRef, phiRef, ptRhoRef, ptRhoFlowRef, pt4VecRhoRef, pt4VecRhoFlowRef;
  
  rcTreeOut_p->Branch("outEntry", &outEntry, "outEntry/i");
  rcTreeOut_p->Branch("weight", &weight, "weight/F");
  rcTreeOut_p->Branch("etaPos", &etaPos, "etaPos/i");
  rcTreeOut_p->Branch("centPos", &centPos, "centPos/I");
  rcTreeOut_p->Branch("cent", &cent, "cent/I");
  rcTreeOut_p->Branch("etaRC", &etaRC, "etaRC/F");
  rcTreeOut_p->Branch("phiRC", &phiRC, "phiRC/F");
  rcTreeOut_p->Branch("ptRC", &ptRC, "ptRC/F");
  rcTreeOut_p->Branch("ptRhoRC", &ptRhoRC, "ptRhoRC/F");
  if(doRhoFlow) rcTreeOut_p->Branch("ptRhoFlowRC", &ptRhoFlowRC, "ptRhoFlowRC/F");
  rcTreeOut_p->Branch("ptSubRC", &ptSubRC, "ptSubRC/F");
  if(doRhoFlow) rcTreeOut_p->Branch("ptSubFlowRC", &ptSubFlowRC, "ptSubFlowRC/F");

  rcTreeOut_p->Branch("pt4VecRC", &pt4VecRC, "pt4VecRC/F");
  rcTreeOut_p->Branch("pt4VecRhoRC", &pt4VecRhoRC, "pt4VecRhoRC/F");
  if(doRhoFlow) rcTreeOut_p->Branch("pt4VecRhoFlowRC", &pt4VecRhoFlowRC, "pt4VecRhoFlowRC/F");
  rcTreeOut_p->Branch("pt4VecSubRC", &pt4VecSubRC, "pt4VecSubRC/F");
  if(doRhoFlow) rcTreeOut_p->Branch("pt4VecSubFlowRC", &pt4VecSubFlowRC, "pt4VecSubFlowRC/F");
  if(doJet){
    jetTreeOut_p->Branch("outEntry", &outEntry, "outEntry/i");
    jetTreeOut_p->Branch("weight", &weight, "weight/F");
    jetTreeOut_p->Branch("etaPos", &etaPosJet, "etaPos/i");
    jetTreeOut_p->Branch("centPos", &centPos, "centPos/I");
    jetTreeOut_p->Branch("cent", &cent, "cent/I");
    jetTreeOut_p->Branch("ptJet", &ptJet, "ptJet/F");
    jetTreeOut_p->Branch("etaJet", &etaJet, "etaJet/F");
    jetTreeOut_p->Branch("phiJet", &phiJet, "phiJet/F");
    jetTreeOut_p->Branch("ptRhoJet", &ptRhoJet, "ptRhoJet/F");
    if(doRhoFlow) jetTreeOut_p->Branch("ptRhoFlowJet", &ptRhoFlowJet, "ptRhoFlowJet/F");
    
    jetTreeOut_p->Branch("pt4VecRhoJet", &pt4VecRhoJet, "pt4VecRhoJet/F");
    if(doRhoFlow) jetTreeOut_p->Branch("pt4VecRhoFlowJet", &pt4VecRhoFlowJet, "pt4VecRhoFlowJet/F");

    if(imbRefPtStr.size() != 0){
      genJetTreeOut_p->Branch("outEntry", &outEntry, "outEntry/i");
      genJetTreeOut_p->Branch("weight", &weight, "weight/F");
      genJetTreeOut_p->Branch("etaPos", &etaPosRef, "etaPos/i");
      genJetTreeOut_p->Branch("centPos", &centPos, "centPos/I");
      genJetTreeOut_p->Branch("cent", &cent, "cent/I");
      genJetTreeOut_p->Branch("ptRef", &ptRef, "ptRef/F");
      genJetTreeOut_p->Branch("etaRef", &etaRef, "etaRef/F");
      genJetTreeOut_p->Branch("phiRef", &phiRef, "phiRef/F");
      genJetTreeOut_p->Branch("ptRhoRef", &ptRhoRef, "ptRhoRef/F");
      if(doRhoFlow) genJetTreeOut_p->Branch("ptRhoFlowRef", &ptRhoFlowRef, "ptRhoFlowRef/F");
      
      genJetTreeOut_p->Branch("pt4VecRhoRef", &pt4VecRhoRef, "pt4VecRhoRef/F");
      if(doRhoFlow) genJetTreeOut_p->Branch("pt4VecRhoFlowRef", &pt4VecRhoFlowRef, "pt4VecRhoFlowRef/F");
    }
  }
  
  std::vector<std::string> rcBranchNames = {rcPtStr, rcPhiStr, rcEtaStr};
  if(!isVectorRC) rcBranchNames.push_back(rcNStr);

  std::vector<std::string> imbBranchNames;
  if(doJet){
    imbBranchNames.push_back(imbPtStr);
    imbBranchNames.push_back(imbPhiStr);
    imbBranchNames.push_back(imbEtaStr);

    if(imbRefPtStr.size() != 0) imbBranchNames.push_back(imbRefPtStr);
    if(imbWeightStr.size() != 0) imbBranchNames.push_back(imbWeightStr);
  }

  //Open file and check that our inputs are correct  
  TFile* inFile_p = new TFile(inFileName.c_str(), "READ");
  std::vector<std::string> ttrees = returnRootFileContentsList(inFile_p, "TTree");
  bool rcIsFound = false;
  bool centIsFound = false;
  bool imbIsFound = false;
  for(auto const & iter : ttrees){
    if(isStrSame(rcInStr, iter)) rcIsFound = true;
    if(isStrSame(centInStr, iter)) centIsFound = true;
    if(doJet){
      if(isStrSame(imbInStr, iter)) imbIsFound = true;
    }
    
    if(rcIsFound && centIsFound && (imbIsFound || !doJet)) break;
  }

  if(!rcIsFound){
    std::cout << "INPUT TREE FOR RC \'" << rcInStr << "\' is not found. return 1" << std::endl;
    return 1;
  }
  if(!centIsFound){
    std::cout << "INPUT TREE FOR CENT \'" << centInStr << "\' is not found. return 1" << std::endl;
    return 1;
  }
  if(!imbIsFound && doJet){
    std::cout << "INPUT TREE FOR IMB \'" << imbInStr << "\' is not found. return 1" << std::endl;
    return 1;
  }

  centralityFromInput centTable(centTableStr);

  TTree* imbTreeIn_p = NULL;
  TObjArray* imbListOfBranchesArr = NULL;
  std::map<std::string, bool> imbListOfBranchesMap;
  TTree* rcTreeIn_p = (TTree*)inFile_p->Get(rcInStr.c_str());
  TObjArray* rcListOfBranchesArr = rcTreeIn_p->GetListOfBranches();
  std::map<std::string, bool> rcListOfBranchesMap;
  for(Int_t bI = 0; bI < rcListOfBranchesArr->GetEntries(); ++bI){
    rcListOfBranchesMap[rcListOfBranchesArr->At(bI)->GetName()] = true;
  }

  if(doJet){
    imbTreeIn_p = (TTree*)inFile_p->Get(imbInStr.c_str());
    imbListOfBranchesArr = imbTreeIn_p->GetListOfBranches();
    for(Int_t bI = 0; bI < imbListOfBranchesArr->GetEntries(); ++bI){
      imbListOfBranchesMap[imbListOfBranchesArr->At(bI)->GetName()] = true;
    }
  }

  
  rcTreeIn_p->SetBranchStatus("*", 0);
  for(auto const & iter : rcBranchNames){
    if(rcListOfBranchesMap.count(iter) == 0){
      std::cout << "Requested branch \'" << iter << "\' not found in ttree \'" << rcInStr << "\'. please check. return 1" << std::endl;
      return 1;
    }
    rcTreeIn_p->SetBranchStatus(iter.c_str(), 1);
  }

  if(doJet){
    imbTreeIn_p->SetBranchStatus("*", 0);
    for(auto const & iter : imbBranchNames){
      if(iter.size() == 0) continue;
      
      if(imbListOfBranchesMap.count(iter) == 0){
	std::cout << "Requested branch \'" << iter << "\' not found in ttree \'" << imbInStr << "\'. please check. return 1" << std::endl;
	return 1;
      }
      imbTreeIn_p->SetBranchStatus(iter.c_str(), 1);
    }
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

  if(doJet){
    if(isVectorImb){
      if(isDoubleImb){
	imbTreeIn_p->SetBranchAddress(imbPtStr.c_str(), &jtptD_p);
	imbTreeIn_p->SetBranchAddress(imbPhiStr.c_str(), &jtphiD_p);
	imbTreeIn_p->SetBranchAddress(imbEtaStr.c_str(), &jtetaD_p);
	if(imbRefPtStr.size() != 0) imbTreeIn_p->SetBranchAddress(imbRefPtStr.c_str(), &refptD_p);
	
	if(imbWeightStr.size() != 0) imbTreeIn_p->SetBranchAddress(imbWeightStr.c_str(), &jtweightD_p);
      }
      else{
	imbTreeIn_p->SetBranchAddress(imbPtStr.c_str(), &jtptF_p);
	imbTreeIn_p->SetBranchAddress(imbPhiStr.c_str(), &jtphiF_p);
	imbTreeIn_p->SetBranchAddress(imbEtaStr.c_str(), &jtetaF_p);
	if(imbRefPtStr.size() != 0) imbTreeIn_p->SetBranchAddress(imbRefPtStr.c_str(), &refptF_p);
	
	if(imbWeightStr.size() != 0) imbTreeIn_p->SetBranchAddress(imbWeightStr.c_str(), &jtweightF_p);
      }
    }
    else{
      imbTreeIn_p->SetBranchAddress(imbNStr.c_str(), &nref_);
      
      if(isDoubleImb){
	imbTreeIn_p->SetBranchAddress(imbPtStr.c_str(), jtptD_);
	imbTreeIn_p->SetBranchAddress(imbPhiStr.c_str(), jtphiD_);
	imbTreeIn_p->SetBranchAddress(imbEtaStr.c_str(), jtetaD_);
	if(imbRefPtStr.size() != 0) imbTreeIn_p->SetBranchAddress(imbRefPtStr.c_str(), refptD_);
	if(imbWeightStr.size() != 0) imbTreeIn_p->SetBranchAddress(imbWeightStr.c_str(), jtweightD_);
      }
      else{
	imbTreeIn_p->SetBranchAddress(imbPtStr.c_str(), jtptF_);
	imbTreeIn_p->SetBranchAddress(imbPhiStr.c_str(), jtphiF_);
	imbTreeIn_p->SetBranchAddress(imbEtaStr.c_str(), jtetaF_);
	if(imbRefPtStr.size() != 0) imbTreeIn_p->SetBranchAddress(imbRefPtStr.c_str(), refptF_);
	if(imbWeightStr.size() != 0) imbTreeIn_p->SetBranchAddress(imbWeightStr.c_str(), jtweightF_);
      }
    }
  }  
  
  if(doRhoFlow) rhoTreeIn_p->SetBranchAddress(rhoFlowInStr.c_str(), &rhoFlowFitParams_p);
  
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

  const Int_t nEntries = (Int_t)rcTreeIn_p->GetEntries();
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
      cent = (Int_t)centTable.getCent(centVal);

      ++(centVals[cent]);
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
    if(doJet) imbTreeIn_p->GetEntry(entry);
    
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
   
    cent = centTable.getCent(centVal);
    weight = 1.0;
    if(doNCollWeights) weight = centWeights[cent];

    centPos = -1;
    for(Int_t cI = 0; cI < nCentBins; ++cI){
      if(centBinsLow[cI] <= cent && cent < centBinsHigh[cI]){       
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
      ptRhoFlowRC = 0;
    
      TLorentzVector totalParticleTL(0.0, 0.0, 0.0, 0.0);
      TLorentzVector totalRhoTL(0.0, 0.0, 0.0, 0.0);
      TLorentzVector totalRhoFlowTL(0.0, 0.0, 0.0, 0.0);
      TLorentzVector tempTL;
      for(Int_t pI = 0; pI < nPart_; ++pI){
	if(ptF_[pI] < ptCut) continue;
	if(getDR(etaRC, phiRC, etaF_[pI], phiF_[pI]) < rcR){
	  ptRC += ptF_[pI];
	  tempTL.SetPtEtaPhiM(ptF_[pI], etaF_[pI], phiF_[pI], 0.0);
	  totalParticleTL += tempTL;
	}
      }

      if(true){
	Int_t nRhoCounter = 0;
	while(nRhoCounter < nRhoMC){
	  Double_t etaRho = randGen_p->Uniform(etaRC - rcR, etaRC + rcR);
	  Double_t phiRho = randGen_p->Uniform(phiRC - rcR, phiRC + rcR);
	  if(getDR(etaRC, phiRC, etaRho, phiRho) >= rcR) continue;

	  double factor = 1.0;
	  if(doRhoFlow) factor = getPhiFactor(rhoFlowFitParams_p, phiRho);
	  
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
	  ptRhoFlowRC += pTOfEst*factor;
	  
	  tempTL.SetPtEtaPhiM(pTOfEst, etaRho, phiRho, 0.0);
	  totalRhoTL += tempTL;	

	  tempTL.SetPtEtaPhiM(pTOfEst*factor, etaRho, phiRho, 0.0);
	  totalRhoFlowTL += tempTL;	
	
	  ++nRhoCounter;
	}
      }
      else{
	
      }                   

      pt4VecRC = totalParticleTL.Pt();
      pt4VecRhoRC = totalRhoTL.Pt();
      pt4VecRhoFlowRC = totalRhoFlowTL.Pt();
      
      ptSubRC = ptRC - ptRhoRC;
      ptSubFlowRC = ptRC - ptRhoFlowRC;
      pt4VecSubRC = pt4VecRC - pt4VecRhoRC;
      pt4VecSubFlowRC = pt4VecRC - pt4VecRhoFlowRC;
      
      rcTreeOut_p->Fill();
    }


    if(doJet){
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
	    
	    //	    jtweightF_[nref_] = centWeight;
	    
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
	    
	    //	      jtweightF_[nref_] = centWeight;
	    
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
	  
	  //	  jtweightF_[nref_] = centWeight;
	    
	  if(imbWeightStr.size() != 0) jtweightF_[pI] = (Float_t)jtweightD_[pI];
	  else jtweightF_[nref_] = 1.0;
	}
      }
      else if(imbWeightStr.size() == 0){
	for(Int_t pI = 0; pI < nref_; ++pI){
	  //	  jtweightF_[pI] = centWeight;
	}
      }
          
      for(Int_t jI = 0; jI < nref_; ++jI){
	if(jtptF_[jI] < 50.) continue;
	if(TMath::Abs(jtetaF_[jI]) > 2.) continue;
	
	ptJet = jtptF_[jI];
	etaJet = jtetaF_[jI];
	phiJet = jtphiF_[jI];	  

	for(Int_t eI = 0; eI < nEtaBins; ++eI){
	  if(etaJet >= TMath::Abs(etaBinsLow[eI]) && etaJet <= TMath::Abs(etaBinsHigh[eI])){
	    etaPosJet = eI;
	    break;
	  }
	}

	
	ptRhoJet = 0;
	ptRhoFlowJet = 0;

	TLorentzVector totalRhoTL(0.0, 0.0, 0.0, 0.0);
	TLorentzVector totalRhoFlowTL(0.0, 0.0, 0.0, 0.0);
	TLorentzVector tempTL;
	
	Int_t nRhoCounter = 0;
	while(nRhoCounter < nRhoMC){
	  Double_t etaRho = randGen_p->Uniform(etaJet - rcR, etaJet + rcR);
	  Double_t phiRho = randGen_p->Uniform(phiJet - rcR, phiJet + rcR);
	  if(getDR(etaJet, phiJet, etaRho, phiRho) >= rcR) continue;
	  
	  double factor = 1.0;
	  if(doRhoFlow) factor = getPhiFactor(rhoFlowFitParams_p, phiRho);
	  
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
	  ptRhoJet += pTOfEst;
	  ptRhoFlowJet += pTOfEst*factor;
	  
	  tempTL.SetPtEtaPhiM(pTOfEst, etaRho, phiRho, 0.0);
	  totalRhoTL += tempTL;
	  
	  tempTL.SetPtEtaPhiM(pTOfEst*factor, etaRho, phiRho, 0.0);
	  totalRhoFlowTL += tempTL;
	  
	  ++nRhoCounter;
	}
	
	pt4VecRhoJet = totalRhoTL.Pt();
	pt4VecRhoFlowJet = totalRhoFlowTL.Pt();
	
	jetTreeOut_p->Fill();	      
      }
      
      if(imbRefPtStr.size() != 0){
	for(Int_t jI = 0; jI < nref_; ++jI){
	  if(refptF_[jI] < 50.) continue;
	  if(TMath::Abs(jtetaF_[jI]) > 2.) continue;

	  ptRef = refptF_[jI];
	  etaRef = jtetaF_[jI];
	  phiRef = jtphiF_[jI];

	  for(Int_t eI = 0; eI < nEtaBins; ++eI){
	    if(etaRef >= TMath::Abs(etaBinsLow[eI]) && etaRef <= TMath::Abs(etaBinsHigh[eI])){
	      etaPosRef = eI;
	      break;
	    }
	  }


	  ptRhoRef = 0;
	  ptRhoFlowRef = 0;

	  TLorentzVector totalRhoTL(0.0, 0.0, 0.0, 0.0);
	  TLorentzVector totalRhoFlowTL(0.0, 0.0, 0.0, 0.0);
	  TLorentzVector tempTL;
	  
	  Int_t nRhoCounter = 0;
	  while(nRhoCounter < nRhoMC){
	    Double_t etaRho = randGen_p->Uniform(etaRef - rcR, etaRef + rcR);
	    Double_t phiRho = randGen_p->Uniform(phiRef - rcR, phiRef + rcR);
	    if(getDR(etaRef, phiRef, etaRho, phiRho) >= rcR) continue;

	    double factor = 1.0;
	    if(doRhoFlow) factor = getPhiFactor(rhoFlowFitParams_p, phiRho);
	    
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
	    ptRhoRef += pTOfEst;
	    ptRhoFlowRef += pTOfEst*factor;
	    
	    tempTL.SetPtEtaPhiM(pTOfEst, etaRho, phiRho, 0.0);
	    totalRhoTL += tempTL;
	    
	    tempTL.SetPtEtaPhiM(pTOfEst*factor, etaRho, phiRho, 0.0);
	    totalRhoFlowTL += tempTL;
	    
	    ++nRhoCounter;
	  }

	  pt4VecRhoRef = totalRhoTL.Pt();
	  pt4VecRhoFlowRef = totalRhoFlowTL.Pt();

	  genJetTreeOut_p->Fill();
	}
      }     		
    }
  }

  inFile_p->Close();
  delete inFile_p;
  
  outFile_p->cd();
  rcTreeOut_p->Write("", TObject::kOverwrite);
  delete rcTreeOut_p;
  
  if(doJet){
    jetTreeOut_p->Write("", TObject::kOverwrite);
    delete jetTreeOut_p;
    
    if(imbRefPtStr.size() != 0){
      genJetTreeOut_p->Write("", TObject::kOverwrite);
      delete genJetTreeOut_p;
    }    
  }
  
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
