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
  void setRCR(std::string inStr);
  void setCentInput(std::string inStr);
  void setNRhoMC(std::string inStr);
  void setRhoInput(std::string inStr);

  bool checkNCentBinsMax(int nCentBinsMax);
  bool checkNEtaBinsMax(int nEtaBinsMax);
  
  std::string getArbitraryParam(std::string inStr);
  std::string getEtaBinsLowStr(){return paramFound["ETABINSLOW"];}
  std::string getEtaBinsHighStr(){return paramFound["ETABINSHIGH"];}
  std::string getCentBinsLowStr(){return paramFound["CENTBINSLOW"];}
  std::string getCentBinsHighStr(){return paramFound["CENTBINSHIGH"];}
  std::string getNRCStr(){return paramFound["NRC"];}
  std::string getRCInputStr(){return paramFound["RCINPUT"];}
  std::string getRCRStr(){return paramFound["RCR"];}
  std::string getCentInputStr(){return paramFound["CENTINPUT"];}
  std::string getNRhoMCStr(){return paramFound["NRHOMC"];}
  std::string getRhoInputStr(){return paramFound["RHOINPUT"];}

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

  std::string getRCInStr(){return rcInStr;}
  bool getIsVectorRC(){return isVectorRC;}
  bool getIsDoubleRC(){return isDoubleRC;}

  std::string getRCNStr(){return rcNStr;}
  std::string getRCPtStr(){return rcPtStr;}
  std::string getRCPhiStr(){return rcPhiStr;}
  std::string getRCEtaStr(){return rcEtaStr;}

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

 private:
  static const Int_t nParams = 10;
  std::string params[nParams] = {"ETABINSLOW", "ETABINSHIGH", "CENTBINSLOW", "CENTBINSHIGH", "NRC", "RCINPUT", "RCR", "CENTINPUT", "NRHOMC", "RHOINPUT"};
  std::string paramDefaults[nParams] = {"0,1", "1,2", "0,30", "30,100", "1", "", "0.4", "", "100", ""};
  std::map<std::string, std::string> paramFound;
      
  Int_t nEtaBins;
  std::vector<double> etaBinsLow; 
  std::vector<double> etaBinsHigh;
 
  Int_t nCentBins;
  std::vector<double> centBinsLow;
  std::vector<double> centBinsHigh;
  
  Int_t nRC;
  Double_t rcR;

  std::string rcInStr;
  bool isVectorRC;
  bool isDoubleRC;
  std::string rcNStr;
  std::string rcPtStr;
  std::string rcPhiStr;
  std::string rcEtaStr;

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

  rcInStr = "";
  isVectorRC = false;
  isDoubleRC = false;
  rcNStr = "";
  rcPtStr = "";
  rcPhiStr = "";
  rcEtaStr = "";

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
  else if(isStrSame(tempVect[0], "CENTINPUT")) setCentInput(inStr);
  else if(isStrSame(tempVect[0], "NRHOMC")) setNRhoMC(inStr);
  else if(isStrSame(tempVect[0], "RHOINPUT")) setRhoInput(inStr);
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
  if(tempVect.size() != 6 && tempVect.size() != 7) std::cout << "WARNING PARAMPROPAGATOR: RCINPUT IS INVALID SIZE. return 1";
  else{
    rcInStr = tempVect[1];
    if(isStrSame("vector", tempVect[2])) isVectorRC = true;
    if(isStrSame("double", tempVect[3])) isDoubleRC = true;

    Int_t restPos = 4;
    if(!isVectorRC){
      rcNStr = tempVect[restPos];
      ++restPos;
    }
    rcPtStr = tempVect[restPos];
    rcPhiStr = tempVect[restPos+1];
    rcEtaStr = tempVect[restPos+2];    
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
