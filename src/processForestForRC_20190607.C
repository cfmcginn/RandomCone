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
#include "TRandom3.h"
#include "TTree.h"

//Local
#include "include/centralityFromInput.h"
#include "include/checkMakeDir.h"
#include "include/etaPhiFunc.h"
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
  const Int_t nParams = 8;
  const std::string params[nParams] = {"ETABINSLOW", "ETABINSHIGH", "CENTBINSLOW", "CENTBINSHIGH", "NRC", "RCINPUT", "RCR", "CENTINPUT"};
  const std::string paramDefaults[nParams] = {"0,1", "1,2", "0,30", "30,100", "1", "", "0.4", ""};
  std::map<std::string, std::string> paramFound;
  for(Int_t pI = 0; pI < nParams; ++pI){
    paramFound[params[pI]] = "";
  }

  const Int_t nEtaBinsMax = 10;
  const Int_t nCentBinsMax = 10;

  Int_t nEtaBins = 0;
  Double_t etaBinsLow[nEtaBinsMax];
  Double_t etaBinsHigh[nEtaBinsMax];
    
  Int_t nCentBins = 0;
  Double_t centBinsLow[nCentBinsMax];
  Double_t centBinsHigh[nCentBinsMax];

  Int_t nRC = 0;
  Double_t rcR = -1;
  
  std::string rcInStr = "";
  bool isVector = false;
  bool isDouble = false;
  std::string rcNStr = "";
  std::string rcPtStr = "";
  std::string rcPhiStr = "";
  std::string rcEtaStr = "";

  std::string centInStr = "";
  std::string centTableStr = "";
  std::string centVarType = "";
  std::vector<std::string> centVarStr;
  std::vector<double> centVarD;
  std::vector<float> centVarF;
  std::vector<int> centVarI;
  
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
  
  //PROCESS INPUT PARAMS FILE
  std::ifstream paramFile(paramFileName.c_str());
  std::string tempStr;

  //std::cout << "LINE: " << __LINE__ << std::endl;
  while(std::getline(paramFile, tempStr)){
    std::vector<std::string> tempVect = strToVect(tempStr);
    if(tempVect.size() == 0) continue;
    if(tempVect[0].substr(0,1).find("#") != std::string::npos) continue;
    if(paramFound.count(tempVect[0]) == 0) continue;
    else paramFound[tempVect[0]] = tempStr;
    
    std::cout << "START: " << tempVect[0] << std::endl;

    bool isGood = false;
    if(isStrSame(tempVect[0], "ETABINSLOW")){
      if(tempVect.size() == 1) std::cout << "ETABINSLOW HAS NO VALUES, \'" << tempStr << "\'. return 1" << std::endl;
      else if(nEtaBins != 0 && nEtaBins != (Int_t)tempVect.size()-1) std::cout << "ETABINSLOW HAS N VALUES MISMATCH, \'" << tempVect.size()-1 << "\', \'" << nEtaBins << "\'. return 1" << std::endl;
      else{
	if(nEtaBins == 0) nEtaBins = tempVect.size()-1;

	for(unsigned int tI = 1; tI < tempVect.size(); ++tI){
	  etaBinsLow[tI-1] = std::stod(tempVect[tI]);
	}	  
	
	isGood = true;
      }			 
    }
    else if(isStrSame(tempVect[0], "ETABINSHIGH")){
      if(tempVect.size() == 1) std::cout << "ETABINSHIGH HAS NO VALUES, \'" << tempStr << "\'. return 1" << std::endl;
      else if(nEtaBins != 0 && nEtaBins != (Int_t)tempVect.size()-1) std::cout << "ETABINSHIGH HAS N VALUES MISMATCH, \'" << tempVect.size()-1 << "\', \'" << nEtaBins << "\'. return 1" << std::endl;
      else{
	if(nEtaBins == 0) nEtaBins = tempVect.size()-1;

	for(unsigned int tI = 1; tI < tempVect.size(); ++tI){
	  etaBinsHigh[tI-1] = std::stod(tempVect[tI]);
	}	  
	
	isGood = true;
      }			 
    }
    else if(isStrSame(tempVect[0], "CENTBINSLOW")){
      if(tempVect.size() == 1) std::cout << "CENTBINSLOW HAS NO VALUES, \'" << tempStr << "\'. return 1" << std::endl;
      else if(nCentBins != 0 && nCentBins != (Int_t)tempVect.size()-1) std::cout << "CENTBINSLOW HAS N VALUES MISMATCH, \'" << tempVect.size()-1 << "\', \'" << nCentBins << "\'. return 1" << std::endl;
      else{
	if(nCentBins == 0) nCentBins = tempVect.size()-1;

	for(unsigned int tI = 1; tI < tempVect.size(); ++tI){
	  centBinsLow[tI-1] = std::stod(tempVect[tI]);
	}	  
	
	isGood = true;
      }			 
    }
    else if(isStrSame(tempVect[0], "CENTBINSHIGH")){
      if(tempVect.size() == 1) std::cout << "CENTBINSHIGH HAS NO VALUES, \'" << tempStr << "\'. return 1" << std::endl;
      else if(nCentBins != 0 && nCentBins != (Int_t)tempVect.size()-1) std::cout << "CENTBINSHIGH HAS N VALUES MISMATCH, \'" << tempVect.size()-1 << "\', \'" << nCentBins << "\'. return 1" << std::endl;
      else{
	if(nCentBins == 0) nCentBins = tempVect.size()-1;

	for(unsigned int tI = 1; tI < tempVect.size(); ++tI){
	  centBinsHigh[tI-1] = std::stod(tempVect[tI]);
	}	  
	
	isGood = true;
      }			 
    }
    else if(isStrSame(tempVect[0], "NRC")){ 
      if(tempVect.size() == 1) std::cout << "NRC HAS NO VALUES, \'" << tempStr << "\'. return 1" << std::endl;
      else{
	nRC = std::stoi(tempVect[1]);
	
	isGood = true;
      }			       
    }
    else if(isStrSame(tempVect[0], "RCINPUT")){
      if(tempVect.size() != 6 && tempVect.size() != 7) std::cout << "RCINPUT IS INVALID SIZE. return 1";
      else{
	rcInStr = tempVect[1];
	if(isStrSame("vector", tempVect[2])) isVector = true;
	if(isStrSame("double", tempVect[3])) isDouble = true;

	Int_t restPos = 4;
	if(!isVector){
	  rcNStr = tempVect[restPos];
	  ++restPos;
	}
	rcPtStr = tempVect[restPos];
	rcPhiStr = tempVect[restPos+1];
	rcEtaStr = tempVect[restPos+2];

	isGood = true;
      }
    }
    else if(isStrSame(tempVect[0], "RCR")){ 
      if(tempVect.size() == 1) std::cout << "RCR HAS NO VALUES, \'" << tempStr << "\'. return 1" << std::endl;
      else{
	rcR = std::stod(tempVect[1]);
	
	isGood = true;
      }			       
    }
    else if(isStrSame(tempVect[0], "CENTINPUT")){
      //std::cout << "LINE: " << __LINE__ << std::endl;
      if(tempVect.size() < 5) std::cout << "CENTINPUT IS INVALID SIZE. return 1";
      else{  	
	//std::cout << "LINE: " << __LINE__ << std::endl;
	centInStr = tempVect[1];
	centTableStr = tempVect[2];
	centVarType = tempVect[3];
	//std::cout << "LINE: " << __LINE__ << std::endl;
	for(unsigned int vI = 4; vI < tempVect.size(); ++vI){
	  centVarStr.push_back(tempVect[vI]);
	}
	//std::cout << "LINE: " << __LINE__ << std::endl;

	isGood = true;
      }
    }

    //std::cout << "LINE: " << __LINE__ << std::endl;

    if(!isGood) return 1;   
  }  
  paramFile.close();
  //std::cout << "LINE: " << __LINE__ << std::endl;

  std::cout << "RCR: " << rcR << std::endl;
  
  if(paramFound["RCINPUT"].size() == 0){
    std::cout << "Given config \'" << paramFileName << "\' must define RCINPUT. return 1" << std::endl;
    return 1;
  }
  if(paramFound["CENTINPUT"].size() == 0){
    std::cout << "Given config \'" << paramFileName << "\' must define CENTINPUT. return 1" << std::endl;
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
  outFileName = "output/" + dateStr + "/" + outFileName + "_RCForest_" + dateStr + ".root";  

  //Declare outFile, outTree, and tnamed
  TFile* outFile_p = new TFile(outFileName.c_str(), "RECREATE");
  TTree* rcTreeOut_p = new TTree("rcTree", "");
  TDirectoryFile* rcParamDir_p = (TDirectoryFile*)outFile_p->mkdir("rcParamDir");

  UInt_t outEntry;
  UInt_t etaPos = 0;
  UInt_t centPos = 0;
  Int_t centRC;
  Float_t etaRC;
  Float_t phiRC;
  Float_t ptRC; 
  
  rcTreeOut_p->Branch("outEntry", &outEntry, "outEntry/i");
  rcTreeOut_p->Branch("etaPos", &etaPos, "etaPos/i");
  rcTreeOut_p->Branch("centPos", &centPos, "centPos/i");
  rcTreeOut_p->Branch("centRC", &centRC, "centRC/I");
  rcTreeOut_p->Branch("etaRC", &etaRC, "etaRC/F");
  rcTreeOut_p->Branch("phiRC", &phiRC, "phiRC/F");
  rcTreeOut_p->Branch("ptRC", &ptRC, "ptRC/F");
  
  std::vector<std::string> rcBranchNames = {rcPtStr, rcPhiStr, rcEtaStr};
  if(!isVector) rcBranchNames.push_back(rcNStr);
  
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

  //std::cout << "LINE: " << __LINE__ << std::endl;

  centralityFromInput centTable(centTableStr);

  //std::cout << "LINE: " << __LINE__ << std::endl;

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

  //std::cout << "LINE: " << __LINE__ << std::endl;

  centTreeIn_p->SetBranchStatus("*", 0);
  for(auto const & iter : centVarStr){
    if(centListOfBranchesMap.count(iter) == 0){
      std::cout << "Requested branch \'" << iter << "\' not found in ttree \'" << centInStr << "\'. please check. return 1" << std::endl;
      return 1;
    }
    centTreeIn_p->SetBranchStatus(iter.c_str(), 1);
  }

  
  if(isVector){
    if(isDouble){
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

    if(isDouble){
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
  const Int_t nDiv = TMath::Max(1, nEntries/10);

  std::cout << "Processing " << nEntries << "..." << std::endl;
  for(Int_t entry = 0; entry < nEntries; ++entry){
    if(entry%nDiv == 0) std::cout << " Entry " << entry << "/" << nEntries << std::endl;
    rcTreeIn_p->GetEntry(entry);
    centTreeIn_p->GetEntry(entry);

    outEntry = entry;

    //Lets reassign values to float array for later simplicity
    if(isVector){
      nPart_ = 0;

      if(isDouble){
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
    else{
      for(Int_t pI = 0; pI < nPart_; ++pI){
	ptF_[pI] = (Float_t)ptD_[pI];
	phiF_[pI] = (Float_t)phiD_[pI];
	etaF_[pI] = (Float_t)etaD_[pI];
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
    for(Int_t cI = 0; cI < nCentBins; ++cI){
      if(centBinsLow[cI] <= centRC && centRC < centBinsHigh[cI]){
	centPos = cI;
	break;
      }
    }

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
      
      for(Int_t pI = 0; pI < nPart_; ++pI){
	if(getDR(etaRC, phiRC, etaF_[pI], phiF_[pI]) < rcR) ptRC += ptF_[pI];
      }
      
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
