//Author Chris McGinn (2019.11.11)

//cpp
#include <fstream>
#include <iostream>
#include <map>
#include <string>

//ROOT
#include "TMath.h"

//Local
#include "include/stringUtil.h"

class csnParamReader{
 public:
  csnParamReader(){};
  csnParamReader(std::string initFile);
  ~csnParamReader(){};

  double sigmaFromPt(double pt, Int_t centLow, Int_t centHigh, Int_t rParam, Double_t isSub);
  void setupCSNFromFile(std::string initFile);
  void Print();
  
 private:
  //maps are first rStr, then centStr
  std::map<std::string, std::map<std::string, double> > cParams;
  std::map<std::string, std::map<std::string, double> > sParams;
  std::map<std::string, std::map<std::string, double> > nParams;  
};


csnParamReader::csnParamReader(std::string initFile)
{
  setupCSNFromFile(initFile);
  return;
}

void csnParamReader::setupCSNFromFile(std::string initFile)
{
  std::ifstream inFile(initFile.c_str());
  std::string tempStr;
  while(std::getline(inFile, tempStr)){
    if(tempStr.find("ALGO") != std::string::npos) continue;
    
    std::vector<std::string> tempVect = commaSepStringToVect(tempStr);

    cParams[tempVect[0]][tempVect[1]] = std::stod(tempVect[2]);
    sParams[tempVect[0]][tempVect[1]] = std::stod(tempVect[3]);
    nParams[tempVect[0]][tempVect[1]] = std::stod(tempVect[4]);
  }
  inFile.close();
  
  return;
}

void csnParamReader::Print()
{
  for(auto const & iter : cParams){
    std::cout << iter.first << std::endl;

    for(auto const & iter2 : iter.second){    
      std::cout << " " << iter2.first << " (C,S,N): " << iter2.second << ", " << sParams[iter.first][iter2.first] << ", " << nParams[iter.first][iter2.first] << std::endl;
    }
  }

  return;
}

double csnParamReader::sigmaFromPt(double pt, Int_t centLow, Int_t centHigh, Int_t rParam, Double_t isSub)
{
  std::string centStr = "Cent" + std::to_string(centLow) + "to" + std::to_string(centHigh);
  std::string rStr = std::to_string(rParam);
  if(isSub) rStr = "Cs" + rStr;
  else rStr = "ak" + rStr;

  Double_t cParam = cParams[rStr][centStr];
  Double_t sParam = sParams[rStr][centStr];
  Double_t nParam = nParams[rStr][centStr];

  return TMath::Sqrt(cParam*cParam + sParam*sParam/pt + nParam*nParam/(pt*pt));
}
