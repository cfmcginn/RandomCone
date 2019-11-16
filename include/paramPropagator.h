#ifndef PARAMPROPAGATOR_H
#define PARAMPROPAGATOR_H

//cpp
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

//ROOT
#include "TFile.h"
#include "TNamed.h"

//Local
#include "include/checkMakeDir.h"
#include "include/returnRootFileContentsList.h"
#include "include/stringUtil.h"

class paramPropagator{
 public:
  paramPropagator();
  ~paramPropagator(){};

  void setupFromTXT(std::string inFileName);
  void setupFromROOT(std::string inFileName);
  void setupFromROOT(TFile* inFile_p);
  
  void setArbitraryParam(std::string inStr);
  void setEtaBinsLow(std::string inStr);
  void setEtaBinsHigh(std::string inStr);
  void setCentBinsLow(std::string inStr);
  void setCentBinsHigh(std::string inStr);
  void setNRC(std::string inStr);
  void setRCInput(std::string inStr);
  void setImbInput(std::string inStr);
  void setEvtInput(std::string inStr);
  void setExtendImb(std::string inStr);
  void setRCR(std::string inStr);
  void setJetR(std::string inStr);
  void setPthats(std::string inStr);
  void setPthatWeights(std::string inStr);
  void setDoNCollWeights(std::string inStr);
  void setPtCut(std::string inStr);
  void setIsMC(std::string inStr);
  void setIsPP(std::string inStr);
  void setCentInput(std::string inStr);
  void setNRhoMC(std::string inStr);
  void setRhoInput(std::string inStr);
  void setRhoFlowInput(std::string inStr);

  bool checkNCentBinsMax(int nCentBinsMax);
  bool checkNEtaBinsMax(int nEtaBinsMax);
  
  std::string getArbitraryParam(std::string inStr);
  std::string getEtaBinsLowStr(){return paramFound["ETABINSLOW"];}
  std::string getEtaBinsHighStr(){return paramFound["ETABINSHIGH"];}
  std::string getCentBinsLowStr(){return paramFound["CENTBINSLOW"];}
  std::string getCentBinsHighStr(){return paramFound["CENTBINSHIGH"];}
  std::string getNRCStr(){return paramFound["NRC"];}
  std::string getNImbStr(){return paramFound["NIMB"];}
  std::string getRCInputStr(){return paramFound["RCINPUT"];}
  std::string getRCRStr(){return paramFound["RCR"];}
  std::string getJetRStr(){return paramFound["JETR"];}
  std::string getPthatStr(){return paramFound["PTHATS"];}
  std::string getPthatWeightsStr(){return paramFound["PTHATWEIGHTS"];}
  std::string getDoNCollWeightsStr(){return paramFound["DONCOLLWEIGHTS"];}
  std::string getPtCutStr(){return paramFound["PTCUT"];}
  std::string getIsMCStr(){return paramFound["ISMC"];}
  std::string getIsPPStr(){return paramFound["ISPP"];}
  std::string getCentInputStr(){return paramFound["CENTINPUT"];}
  std::string getNRhoMCStr(){return paramFound["NRHOMC"];}
  std::string getRhoInputStr(){return paramFound["RHOINPUT"];}
  std::string getRhoFlowInputStr(){return paramFound["RHOFLOWINPUT"];}
  
  std::map<std::string, std::string> getParamFound(){return paramFound;}
  
  Int_t getNEtaBins(){return nEtaBins;}
  std::vector<double> getEtaBinsLow(){return etaBinsLow;}
  template<typename T>
  void getEtaBinsLow(T arr[]);
  std::vector<double> getEtaBinsHigh(){return etaBinsHigh;}
  template<typename T>
  void getEtaBinsHigh(T arr[]);

  Int_t getNCentBins(){return nCentBins;}
  std::vector<double> getCentBinsLow(){return centBinsLow;}
  template<typename T>
  void getCentBinsLow(T arr[]);
  std::vector<double> getCentBinsHigh(){return centBinsHigh;}
  template<typename T>
  void getCentBinsHigh(T arr[]);

  Int_t getNRC(){return nRC;}
  Double_t getRCR(){return rcR;}
  Double_t getJetR(){return jetR;}
  std::vector<Double_t> getPthats(){return pthats;}
  std::vector<Double_t> getPthatWeights(){return pthatWeights;}
  Bool_t getDoNCollWeights(){return doNCollWeights;}
  Double_t getPtCut(){return ptCut;}
  Bool_t getIsMC(){return isMC;}
  Bool_t getIsPP(){return isPP;}

  std::string getRCInStr(){return rcInStr;}
  bool getIsVectorRC(){return isVectorRC;}
  bool getIsDoubleRC(){return isDoubleRC;}

  std::string getEvtInStr(){return evtInStr;}
  bool getIsVectorEvt(){return isVectorEvt;}
  std::string getEvtNVarStr(){return evtNVarStr;}
  std::string getEvtVarStr(){return evtVarStr;}
  std::string getEvtPosStr(){return evtPosStr;}
  int getEvtPos(){return evtPos;}

  std::string getImbInStr(){return imbInStr;}
  bool getIsVectorImb(){return isVectorImb;}
  bool getIsDoubleImb(){return isDoubleImb;}

  std::string getRCNStr(){return rcNStr;}
  std::string getRCPtStr(){return rcPtStr;}
  std::string getRCPhiStr(){return rcPhiStr;}
  std::string getRCEtaStr(){return rcEtaStr;}

  std::string getPthatValStr(){return imbPthatStr;}

  std::string getImbNStr(){return imbNStr;}
  std::string getImbPtStr(){return imbPtStr;}
  std::string getImbPhiStr(){return imbPhiStr;}
  std::string getImbEtaStr(){return imbEtaStr;}
  std::string getImbRefPtStr(){return imbRefPtStr;}
  std::string getImbWeightStr(){return imbWeightStr;}

  std::string getImbConstNStr(){return imbConstNStr;}
  std::string getImbConstPtStr(){return imbConstPtStr;}
  std::string getImbConstPhiStr(){return imbConstPhiStr;}
  std::string getImbConstEtaStr(){return imbConstEtaStr;}
  std::string getImbConstjtposStr(){return imbConstjtposStr;}

  std::string getImbPfCHFStr(){return imbPfCHFStr;}
  std::string getImbPfCEFStr(){return imbPfCEFStr;}
  std::string getImbPfMUFStr(){return imbPfMUFStr;}
  std::string getImbPfNEFStr(){return imbPfNEFStr;}
  std::string getImbPfNHFStr(){return imbPfNHFStr;}
  
  std::string getCentInStr(){return centInStr;}
  std::string getCentTableStr(){return centTableStr;}
  std::string getCentVarType(){return centVarType;}
  std::vector<std::string> getCentVarStr(){return centVarStr;}

  Int_t getNRhoMC(){return nRhoMC;}
  
  std::string getRhoInStr(){return rhoInStr;}
  bool getIsVectorRho(){return isVectorRho;}
  bool getIsDoubleRho(){return isDoubleRho;}
  std::string getNRhoStr(){return nRhoStr;}
  std::string getRhoStr(){return rhoStr;}
  std::string getRhoEtaMinStr(){return rhoEtaMinStr;}
  std::string getRhoEtaMaxStr(){return rhoEtaMaxStr;}

  std::string getRhoFlowInStr(){return rhoFlowInStr;}

 private:
  static const Int_t nParams = 21;
  std::string params[nParams] = {"ETABINSLOW", "ETABINSHIGH", "CENTBINSLOW", "CENTBINSHIGH", "NRC", "RCINPUT", "RCR", "JETR", "PTHATS", "PTHATWEIGHTS", "DONCOLLWEIGHTS", "PTCUT", "ISMC", "ISPP", "CENTINPUT", "NRHOMC", "RHOINPUT", "RHOFLOWINPUT", "EVTINPUT", "IMBINPUT", "EXTENDIMB"};
  std::string paramDefaults[nParams] = {"0,1", "1,2", "0,30", "30,100", "1", "", "0.4", "0.4", "1", "0.0", "0", "1", "", "", "100", "", "", "", "", ""};
  std::map<std::string, std::string> paramFound;
      
  Int_t nEtaBins;
  std::vector<double> etaBinsLow; 
  std::vector<double> etaBinsHigh;
 
  Int_t nCentBins;
  std::vector<double> centBinsLow;
  std::vector<double> centBinsHigh;
  
  Int_t nRC;
  Double_t rcR;
  Double_t jetR;
  std::string pthatStr;
  std::vector<Double_t> pthats;
  std::vector<Double_t> pthatWeights;
  Bool_t doNCollWeights;
  Double_t ptCut;
  Bool_t isMC;
  Bool_t isPP;
  
  std::string rcInStr;
  bool isVectorRC;
  bool isDoubleRC;
  std::string rcNStr;
  std::string rcPtStr;
  std::string rcPhiStr;
  std::string rcEtaStr;

  std::string evtInStr;
  bool isVectorEvt;
  std::string evtNVarStr;
  std::string evtVarStr;
  std::string evtPosStr;
  int evtPos;
  
  std::string imbInStr;
  bool isVectorImb;
  bool isDoubleImb;
  std::string imbPthatStr;
  std::string imbNStr;
  std::string imbPtStr;
  std::string imbPhiStr;
  std::string imbEtaStr;
  std::string imbRefPtStr;
  std::string imbWeightStr;
  std::string imbConstNStr;
  std::string imbConstPtStr;
  std::string imbConstPhiStr;
  std::string imbConstEtaStr;
  std::string imbConstjtposStr;

  std::string imbPfCHFStr;
  std::string imbPfCEFStr;
  std::string imbPfMUFStr;
  std::string imbPfNEFStr;
  std::string imbPfNHFStr;
  
  std::string centInStr;
  std::string centTableStr;
  std::string centVarType;
  std::vector<std::string> centVarStr;

  Int_t nRhoMC;
  
  std::string rhoInStr;
  bool isVectorRho;
  bool isDoubleRho;
  std::string nRhoStr;
  std::string rhoStr;
  std::string rhoEtaMinStr;
  std::string rhoEtaMaxStr;

  std::string rhoFlowInStr;

  bool checkNBinsMax(std::string binStr, int nBins, int nBinsMax);
};

paramPropagator::paramPropagator()
{
  for(Int_t pI = 0; pI < nParams; ++pI){
    paramFound[params[pI]] = "";
  }

  nEtaBins = 0;
  nCentBins = 0;

  nRC = 0;
  rcR = -1;
  jetR = -1;
  pthats.clear();
  pthatWeights.clear();
  doNCollWeights = false;
  ptCut = 0.0;
  isMC = false;
  isPP = true;
  
  rcInStr = "";
  isVectorRC = false;
  isDoubleRC = false;
  rcNStr = "";
  rcPtStr = "";
  rcPhiStr = "";
  rcEtaStr = "";

  evtInStr = "";
  isVectorEvt = false;
  evtNVarStr = "";
  evtVarStr = "";
  evtPosStr = "";
  evtPos = -1;
    
  imbInStr = "";
  isVectorImb = false;
  isDoubleImb = false;
  imbPthatStr = "";
  imbNStr = "";
  imbPtStr = "";
  imbPhiStr = "";
  imbEtaStr = "";
  imbRefPtStr = "";
  imbWeightStr = "";
  imbConstNStr = "";
  imbConstPtStr = "";
  imbConstPhiStr = "";
  imbConstEtaStr = "";
  imbConstjtposStr = "";

  imbPfCHFStr = "";
  imbPfCEFStr = "";
  imbPfMUFStr = "";
  imbPfNEFStr = "";
  imbPfNHFStr = "";
  
  centInStr = "";
  centTableStr = "";
  centVarType = ""; 
  centVarStr = {};

  nRhoMC = 0;
  
  rhoInStr = "";
  isVectorRho = false;
  isDoubleRho = false;
  nRhoStr = "";
  rhoStr = "";
  rhoEtaMinStr = "";
  rhoEtaMaxStr = "";

  rhoFlowInStr = "";

  return;
}

void paramPropagator::setupFromTXT(std::string inFileName)
{
  if(!checkFile(inFileName) || inFileName.find(".txt") == std::string::npos){
    std::cout << "WARNING PARAMPROPAGATOR: Input txt file \'" << inFileName << "\' is not valid. return uninitialized" << std::endl;    
    return;
  }  
  
  std::ifstream inFile(inFileName.c_str());
  std::string tempStr;
  while(std::getline(inFile, tempStr)){
    std::cout << tempStr << std::endl;
    setArbitraryParam(tempStr);
  }
  inFile.close();  
  
  return;
}

void paramPropagator::setupFromROOT(std::string inFileName)
{
  if(!checkFile(inFileName) || inFileName.find(".root") == std::string::npos){
    std::cout << "WARNING PARAMPROPAGATOR: Input root file \'" << inFileName << "\' is not valid. return uninitialized" << std::endl;    
    return;
  }
  TFile* inFile_p = new TFile(inFileName.c_str(), "READ");
  setupFromROOT(inFile_p);
  inFile_p->Close();
  delete inFile_p;
						      
  return;
}

void paramPropagator::setupFromROOT(TFile* inFile_p)
{
  std::vector<std::string> names = returnRootFileContentsList(inFile_p, "TNamed", "");

  for(auto const & name : names){
    std::string name2 = name;
    while(name2.find("/") != std::string::npos){name2.replace(0, name2.find("/")+1, "");}
    if(paramFound.count(name2) != 0){
      TNamed* tempName_p = (TNamed*)inFile_p->Get(name.c_str());
      std::string fullStr = tempName_p->GetTitle();
      setArbitraryParam(fullStr);
    }
  }
  return;
}

void paramPropagator::setArbitraryParam(std::string inStr)
{
  std::vector<std::string> tempVect = strToVect(inStr);
  if(tempVect.size() == 0) return;
  if(tempVect[0].substr(0,1).find("#") != std::string::npos) return;
  if(paramFound.count(tempVect[0]) == 0) return;
  else paramFound[tempVect[0]] = inStr;

  if(isStrSame(tempVect[0], "ETABINSLOW")) setEtaBinsLow(inStr);
  else if(isStrSame(tempVect[0], "ETABINSHIGH")) setEtaBinsHigh(inStr);
  else if(isStrSame(tempVect[0], "CENTBINSLOW")) setCentBinsLow(inStr);
  else if(isStrSame(tempVect[0], "CENTBINSHIGH")) setCentBinsHigh(inStr);
  else if(isStrSame(tempVect[0], "NRC")) setNRC(inStr);
  else if(isStrSame(tempVect[0], "RCINPUT")) setRCInput(inStr);
  else if(isStrSame(tempVect[0], "RCR")) setRCR(inStr);
  else if(isStrSame(tempVect[0], "JETR")) setJetR(inStr);
  else if(isStrSame(tempVect[0], "PTHATS")) setPthats(inStr);
  else if(isStrSame(tempVect[0], "PTHATWEIGHTS")) setPthatWeights(inStr);
  else if(isStrSame(tempVect[0], "DONCOLLWEIGHTS")) setDoNCollWeights(inStr);
  else if(isStrSame(tempVect[0], "PTCUT")) setPtCut(inStr);
  else if(isStrSame(tempVect[0], "ISMC")) setIsMC(inStr);
  else if(isStrSame(tempVect[0], "ISPP")) setIsPP(inStr);
  else if(isStrSame(tempVect[0], "CENTINPUT")) setCentInput(inStr);
  else if(isStrSame(tempVect[0], "NRHOMC")) setNRhoMC(inStr);
  else if(isStrSame(tempVect[0], "RHOINPUT")) setRhoInput(inStr);
  else if(isStrSame(tempVect[0], "RHOFLOWINPUT")) setRhoFlowInput(inStr);
  else if(isStrSame(tempVect[0], "EVTINPUT")) setEvtInput(inStr);
  else if(isStrSame(tempVect[0], "IMBINPUT")) setImbInput(inStr);
  else if(isStrSame(tempVect[0], "EXTENDIMB")) setExtendImb(inStr);
  else{
    std::cout << "WARNING PARAMPROPAGATOR: Input param \'" << tempVect[0] << "\' is not valid. return" << std::endl;
    return;
  }
  
  return;
}

void paramPropagator::setEtaBinsLow(std::string inStr)
{
  std::vector<std::string> tempVect = strToVect(inStr);
  if(tempVect.size() == 1) std::cout << "WARNING PARAMPROPAGATOR: ETABINSLOW HAS NO VALUES, \'" << inStr << "\'. return 1" << std::endl;
  else if(nEtaBins != 0 && nEtaBins != (Int_t)tempVect.size()-1) std::cout << "WARNING PARAMPROPAGATOR: ETABINSLOW HAS N VALUES MISMATCH, \'" << tempVect.size()-1 << "\', \'" << nEtaBins << "\'. return 1" << std::endl;
  else{
    if(nEtaBins == 0) nEtaBins = tempVect.size()-1;
    
    etaBinsLow.clear();
    for(unsigned int tI = 1; tI < tempVect.size(); ++tI){
      etaBinsLow.push_back(std::stod(tempVect[tI]));
    }
  }
  
  return;
}

void paramPropagator::setEtaBinsHigh(std::string inStr)
{
  std::vector<std::string> tempVect = strToVect(inStr);
  if(tempVect.size() == 1) std::cout << "WARNING PARAMPROPAGATOR: ETABINSHIGH HAS NO VALUES, \'" << inStr << "\'. return 1" << std::endl;
  else if(nEtaBins != 0 && nEtaBins != (Int_t)tempVect.size()-1) std::cout << "WARNING PARAMPROPAGATOR: ETABINSHIGH HAS N VALUES MISMATCH, \'" << tempVect.size()-1 << "\', \'" << nEtaBins << "\'. return 1" << std::endl;
  else{
    if(nEtaBins == 0) nEtaBins = tempVect.size()-1;

    etaBinsHigh.clear();
    for(unsigned int tI = 1; tI < tempVect.size(); ++tI){
      etaBinsHigh.push_back(std::stod(tempVect[tI]));
    }
  }
  
  return;
}

void paramPropagator::setCentBinsLow(std::string inStr)
{
  std::vector<std::string> tempVect = strToVect(inStr);
  if(tempVect.size() == 1) std::cout << "WARNING PARAMPROPAGATOR: CENTBINSLOW HAS NO VALUES, \'" << inStr << "\'. return 1" << std::endl;
  else if(nCentBins != 0 && nCentBins != (Int_t)tempVect.size()-1) std::cout << "WARNING PARAMPROPAGATOR: CENTBINSLOW HAS N VALUES MISMATCH, \'" << tempVect.size()-1 << "\', \'" << nCentBins << "\'. return 1" << std::endl;
  else{
    if(nCentBins == 0) nCentBins = tempVect.size()-1;

    centBinsLow.clear();
    for(unsigned int tI = 1; tI < tempVect.size(); ++tI){
      centBinsLow.push_back(std::stod(tempVect[tI]));
    }
  }
  
  return;
}


void paramPropagator::setCentBinsHigh(std::string inStr)
{
  std::vector<std::string> tempVect = strToVect(inStr);
  if(tempVect.size() == 1) std::cout << "WARNING PARAMPROPAGATOR: CENTBINSHIGH HAS NO VALUES, \'" << inStr << "\'. return 1" << std::endl;
  else if(nCentBins != 0 && nCentBins != (Int_t)tempVect.size()-1) std::cout << "WARNING PARAMPROPAGATOR: CENTBINSHIGH HAS N VALUES MISMATCH, \'" << tempVect.size()-1 << "\', \'" << nCentBins << "\'. return 1" << std::endl;
  else{
    if(nCentBins == 0) nCentBins = tempVect.size()-1;

    centBinsHigh.clear();
    for(unsigned int tI = 1; tI < tempVect.size(); ++tI){
      centBinsHigh.push_back(std::stod(tempVect[tI]));
    }
  }
    
  return;
}

void paramPropagator::setNRC(std::string inStr)
{
  std::vector<std::string> tempVect = strToVect(inStr);
  if(tempVect.size() == 1) std::cout << "WARNING PARAMPROPAGATOR: NRC HAS NO VALUES, \'" << inStr << "\'. return 1" << std::endl;
  else nRC = std::stoi(tempVect[1]);
  
  return;
}

void paramPropagator::setRCInput(std::string inStr)
{
  std::vector<std::string> tempVect = strToVect(inStr);
  if(tempVect.size() != 7 && tempVect.size() != 8) std::cout << "WARNING PARAMPROPAGATOR: RCINPUT IS INVALID SIZE. return 1";
  else{
    rcInStr = tempVect[1];
    if(isStrSame("vector", tempVect[2])) isVectorRC = true;
    if(isStrSame("double", tempVect[3])) isDoubleRC = true;

    Int_t restPos = 4;
    if(!isVectorRC){
      rcNStr = tempVect[restPos];
    }
    rcPtStr = tempVect[restPos+1];
    rcPhiStr = tempVect[restPos+2];
    rcEtaStr = tempVect[restPos+3];    
  }
	
  return;
}

void paramPropagator::setEvtInput(std::string inStr)
{

  std::vector<std::string> tempVect = strToVect(inStr);
  if(tempVect.size() != 6) std::cout << "WARNING PARAMPROPAGATOR: EVTINPUT IS INVALID SIZE. return 1";
  else{
    evtInStr = tempVect[1];
    if(isStrSame(tempVect[2], "vector")) isVectorEvt = true;
    else isVectorEvt = false;

    evtNVarStr = tempVect[3];
    evtVarStr = tempVect[4];
    evtPosStr = tempVect[5];
    evtPos = std::stoi(evtPosStr);
  }

  return;
}

void paramPropagator::setImbInput(std::string inStr)
{
  std::vector<std::string> tempVect = strToVect(inStr);
  if(tempVect.size() != 16) std::cout << "WARNING PARAMPROPAGATOR: IMBINPUT IS INVALID SIZE. return 1";
  else{
    imbInStr = tempVect[1];
    if(isStrSame("vector", tempVect[2])) isVectorImb = true;
    if(isStrSame("double", tempVect[3])) isDoubleImb = true;
      
    imbPthatStr = tempVect[4];

    imbNStr = tempVect[5];
    imbPtStr = tempVect[6];
    imbPhiStr = tempVect[7];
    imbEtaStr = tempVect[8];    
    imbRefPtStr = tempVect[9];
    imbWeightStr = tempVect[10];    

    imbConstNStr = tempVect[11];

    imbConstPtStr = tempVect[12];
    imbConstPhiStr = tempVect[13];
    imbConstEtaStr = tempVect[14];    
    imbConstjtposStr = tempVect[15];    
  }

  return;
}

void paramPropagator::setExtendImb(std::string inStr)
{
  std::vector<std::string> tempVect = strToVect(inStr);
  if(tempVect.size() != 6) std::cout << "WARNING PARAMPROPAGATOR: EXTENDIMB IS INVALID SIZE. return 1";
  else{
    imbPfCHFStr = tempVect[1];
    imbPfCEFStr = tempVect[2];
    imbPfMUFStr = tempVect[3];
    imbPfNEFStr = tempVect[4];
    imbPfNHFStr = tempVect[5];
  }

  return;
}

void paramPropagator::setRCR(std::string inStr)
{
  std::vector<std::string> tempVect = strToVect(inStr);
  if(tempVect.size() == 1) std::cout << "WARNING PARAMPROPAGATOR: RCR HAS NO VALUES, \'" << inStr << "\'. return 1" << std::endl;
  else rcR = std::stod(tempVect[1]);
  
  return;
}

void paramPropagator::setJetR(std::string inStr)
{
  std::vector<std::string> tempVect = strToVect(inStr);
  if(tempVect.size() == 1) std::cout << "WARNING PARAMPROPAGATOR: JETR HAS NO VALUES, \'" << inStr << "\'. return 1" << std::endl;
  else jetR = std::stod(tempVect[1]);
  
  return;
}

void paramPropagator::setPthats(std::string inStr)
{
  std::vector<std::string> tempVect = strToVect(inStr);
  if(tempVect.size() == 1) std::cout << "WARNING PARAMPROPAGATOR: PTHATS HAS NO VALUES, \'" << inStr << "\'. return 1" << std::endl;
  for(unsigned int pI = 1; pI < tempVect.size(); ++pI){
    pthats.push_back(std::stod(tempVect[pI]));
  }
  
  return;
}

void paramPropagator::setPthatWeights(std::string inStr)
{
  std::vector<std::string> tempVect = strToVect(inStr);
  if(tempVect.size() == 1) std::cout << "WARNING PARAMPROPAGATOR: PTHATWEIGHTS HAS NO VALUES, \'" << inStr << "\'. return 1" << std::endl;
  for(unsigned int pI = 1; pI < tempVect.size(); ++pI){
    pthatWeights.push_back(std::stod(tempVect[pI]));
  }
  
  return;
}

void paramPropagator::setDoNCollWeights(std::string inStr)
{
  std::vector<std::string> tempVect = strToVect(inStr);
  if(tempVect.size() == 1) std::cout << "WARNING PARAMPROPAGATOR: DONCOLLWEIGHTS HAS NO VALUES, \'" << inStr << "\'. return 1" << std::endl;
  else doNCollWeights = std::stoi(tempVect[1]);
  
  return;
}

void paramPropagator::setPtCut(std::string inStr)
{
  std::vector<std::string> tempVect = strToVect(inStr);
  if(tempVect.size() == 1) std::cout << "WARNING PARAMPROPAGATOR: PTCUT HAS NO VALUES, \'" << inStr << "\'. return 1" << std::endl;
  else ptCut = std::stod(tempVect[1]);
  
  return;
}

void paramPropagator::setIsMC(std::string inStr)
{
  std::vector<std::string> tempVect = strToVect(inStr);
  if(tempVect.size() == 1) std::cout << "WARNING PARAMPROPAGATOR: ISMC HAS NO VALUES, \'" << inStr << "\'. return 1" << std::endl;
  else isMC = (bool)(std::stoi(tempVect[1]));
  
  return;
}

void paramPropagator::setIsPP(std::string inStr)
{
  std::vector<std::string> tempVect = strToVect(inStr);
  if(tempVect.size() == 1) std::cout << "WARNING PARAMPROPAGATOR: ISPP HAS NO VALUES, \'" << inStr << "\'. return 1" << std::endl;
  else isPP = (bool)(std::stoi(tempVect[1]));
  
  return;
}


bool paramPropagator::checkNBinsMax(std::string binStr, int nBins, int nBinsMax)
{
  if(nBins > nBinsMax){
    std::cout << "PARAMPROPAGATOR: n" << binStr << "Bins > n" << binStr << "BinsMax. return false" << std::endl;
    return false;
  }
  return true;
}

bool paramPropagator::checkNCentBinsMax(int nCentBinsMax){return checkNBinsMax("Cent", nCentBins, nCentBinsMax);}

bool paramPropagator::checkNEtaBinsMax(int nEtaBinsMax){return checkNBinsMax("Eta", nEtaBins, nEtaBinsMax);}

void paramPropagator::setCentInput(std::string inStr)
{
  std::vector<std::string> tempVect = strToVect(inStr);
  if(tempVect.size() < 5) std::cout << "WARNING PARAMPROPAGATOR: CENTINPUT IS INVALID SIZE. return 1" << std::endl;
  else{
    centInStr = tempVect[1];
    centTableStr = tempVect[2];
    centVarType = tempVect[3];

    centVarStr.clear();
    for(unsigned int vI = 4; vI < tempVect.size(); ++vI){
      centVarStr.push_back(tempVect[vI]);
    }
  }
  
  return;
}

void paramPropagator::setNRhoMC(std::string inStr)
{
  std::vector<std::string> tempVect = strToVect(inStr);
  if(tempVect.size() != 2) std::cout << "WARNING PARAMPROPAGATOR: NRHOMC IS INVALID SIZE. return 1" << std::endl;
  else nRhoMC = std::stoi(tempVect[1]);
  
  return;
}


void paramPropagator::setRhoInput(std::string inStr)
{
  std::vector<std::string> tempVect = strToVect(inStr);
  if(tempVect.size() < 7 || tempVect.size() > 8) std::cout << "WARNING PARAMPROPAGATOR: RHOINPUT IS INVALID SIZE. return 1" << std::endl;
  else{
    rhoInStr = tempVect[1];
    if(isStrSame("vector", tempVect[2])) isVectorRho = true;
    else isVectorRho = false;

    if(isStrSame("double", tempVect[3])) isDoubleRho = true;
    else isDoubleRho = false;

    nRhoStr = tempVect[4];
    rhoStr = tempVect[5];

    rhoEtaMinStr = tempVect[6];
    if(tempVect.size() == 8) rhoEtaMaxStr = tempVect[7];
  }

  return;
}


void paramPropagator::setRhoFlowInput(std::string inStr)
{
  std::vector<std::string> tempVect = strToVect(inStr);
  if(tempVect.size() != 2) std::cout << "WARNING PARAMPROPAGATOR: RHOFLOWINPUT IS INVALID SIZE. return 1" << std::endl;
  else rhoFlowInStr = tempVect[1];
  
  return;
}

std::string paramPropagator::getArbitraryParam(std::string inStr)
{
  if(paramFound.count(inStr) == 0){
    std::cout << "WARNING WARNING PARAMPROPAGATOR: Given \'" << inStr << "\' to getArbitraryParam is invalid. return empty string" << std::endl;
    return "";
  }
  else return paramFound[inStr];
}

template<typename T>
void paramPropagator::getEtaBinsLow(T arr[])
{
  for(unsigned int eI = 0; eI < etaBinsLow.size(); ++eI){
    arr[eI] = (T)etaBinsLow[eI];
  }  
  return;
}

template<typename T>
void paramPropagator::getEtaBinsHigh(T arr[])
{
  for(unsigned int eI = 0; eI < etaBinsHigh.size(); ++eI){
    arr[eI] = (T)etaBinsHigh[eI];
  }  
  return;
}

template<typename T>
void paramPropagator::getCentBinsLow(T arr[])
{
  for(unsigned int eI = 0; eI < centBinsLow.size(); ++eI){
    arr[eI] = (T)centBinsLow[eI];
  }  
  return;
}

template<typename T>
void paramPropagator::getCentBinsHigh(T arr[])
{
  for(unsigned int eI = 0; eI < centBinsHigh.size(); ++eI){
    arr[eI] = (T)centBinsHigh[eI];
  }  
  return;
}

#endif
