//cpp
#include <iostream>
#include <map>
#include <string>
#include <vector>

//ROOT
#include "TCanvas.h"
#include "TDatime.h"
#include "TF1.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TKey.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TLine.h"
#include "TStyle.h"

//Local
#include "include/checkMakeDir.h"
#include "include/histDefUtility.h"
#include "include/kirchnerPalette.h"
#include "include/paramPropagator.h"
#include "include/plotUtilities.h"
#include "include/quickGaus.h"
#include "include/returnRootFileContentsList.h"

void runPlotter(std::map<std::string, std::string>* comboNameToTH1NameMap, std::map<std::string, TH1D*>* nameToHist, paramPropagator* params_p, std::string dateStr, std::string moreCombo, std::string postCombo = "")
{
  const Int_t nStyles = 4;
  const Int_t styles[nStyles] = {24, 25, 28, 46};

  const Int_t nCentBins = params_p->getNCentBins();
  const Int_t nEtaBins = params_p->getNEtaBins();

  const Double_t ptCut = params_p->getPtCut();
  const Double_t rcR = params_p->getRCR();

  

  /*
  const Int_t nPtBins = 4;
  const Double_t ptBinsLow[nPtBins] = {50., 100., 200., 300.};
  const Double_t ptBinsHigh[nPtBins] = {100., 200., 300., 500.};

  std::vector<std::string> ptBinsStr;
  for(Int_t pI = 0; pI < nPtBins; ++pI){
    ptBinsStr.push_back("Pt"+prettyString(ptBinsLow[pI],1,true)+"to"+prettyString(ptBinsHigh[pI],1,true));
  }
  */  
  kirchnerPalette kPal;
  const Int_t nColors = 5;
  const Int_t colors[nColors] = {0, 2, 3, 5, 6}; // this are positions in the kirchnerpalette class
  
  Double_t nSigmaLeft = 3.0;
  Double_t nSigmaRight = 0.5;
  Double_t fracLeft = calcQuickGaus(nSigmaLeft);
  Double_t fracRight = calcQuickGaus(nSigmaRight);

  
  
  std::vector<std::string> centStr, etaStr;
  std::vector<double> centBinsLow = params_p->getCentBinsLow();
  std::vector<double> centBinsHigh = params_p->getCentBinsHigh();

  std::vector<double> etaBinsLow = params_p->getEtaBinsLow();
  std::vector<double> etaBinsHigh = params_p->getEtaBinsHigh();

  for(Int_t cI = 0; cI < nCentBins; ++cI){
    std::string temp = "Cent" + std::to_string(((Int_t)centBinsLow[cI])) + "to" + std::to_string(((Int_t)centBinsHigh[cI]));
    centStr.push_back(temp);
  }

  for(Int_t eI = 0; eI < nEtaBins; ++eI){
    std::string temp = "Eta" + prettyString(etaBinsLow[eI],1,true) + "to" + prettyString(etaBinsHigh[eI],1,true);
    etaStr.push_back(temp);
  }

  std::string histStr = "";
  
  std::string tagStr = "RCR" + prettyString(rcR, 1, true) + "_PtCut" + prettyString(ptCut, 2, true) + "_IsMC" + std::to_string(params_p->getIsMC());
  
  std::cout << "FILE, LINE: " << __FILE__ << ", " << __LINE__ << std::endl;
    
  for(Int_t eI = 0; eI < nEtaBins; ++eI){
    TCanvas* canv_p = new TCanvas("temp", "", 450, 450);
    canv_p->SetTopMargin(0.01);
    canv_p->SetLeftMargin(0.18);
    canv_p->SetBottomMargin(0.1);
    canv_p->SetRightMargin(0.01);
    
    //pre-processing step
    Double_t max = -1;
    Double_t min = 100000000000;

    std::cout << "FILE, LINE: " << __FILE__ << ", " << __LINE__ << std::endl;

    for(Int_t cI = 0; cI < nCentBins; ++cI){
    std::cout << "FILE, LINE: " << __FILE__ << ", " << __LINE__ << std::endl;
      std::string comboStr = moreCombo + centStr[cI] + "_" + etaStr[eI];
      if(postCombo.size() != 0) comboStr = comboStr + "_" + postCombo;
      
      std::cout << "COUNT: " << comboNameToTH1NameMap->count(comboStr) << std::endl;
      std::cout << "COMB: " << comboStr << std::endl;

      for(auto const & iter : (*comboNameToTH1NameMap)){
	std::cout << " " << iter.first << std::endl;
      }

      std::cout << "MORE, cent, eta post: " << moreCombo << ", " << centStr[cI] << ", " << etaStr[eI] << ", " << postCombo << std::endl;
      std::cout << " COMB2: " << comboStr << std::endl;
      std::string histName = comboNameToTH1NameMap->at(comboStr);
      std::cout << comboStr << ", " << histName << std::endl;
      std::cout << "FILE, LINE: " << __FILE__ << ", " << __LINE__ << std::endl;

      std::cout << "NAMETOHIST: " << std::endl;
      for(auto const & iter : (*nameToHist)){
	std::cout << " " << iter.first << std::endl;
      }
      
      std::cout << nameToHist->count(histName) << std::endl;
      TH1D* tempHist_p = nameToHist->at(histName);

          std::cout << "FILE, LINE: " << __FILE__ << ", " << __LINE__ << std::endl;

      histStr = histName;

      std::cout << "FILE, LINE: " << __FILE__ << ", " << __LINE__ << std::endl;

      Double_t scaleFactor = tempHist_p->Integral();
      
      for(Int_t bIX = 0; bIX < tempHist_p->GetXaxis()->GetNbins(); ++bIX){
	Double_t width = tempHist_p->GetBinWidth(bIX+1);
	Double_t binVal = tempHist_p->GetBinContent(bIX+1)/(width*scaleFactor);
	Double_t binErr = tempHist_p->GetBinError(bIX+1)/(width*scaleFactor);
	
	tempHist_p->SetBinContent(bIX+1, binVal);
	tempHist_p->SetBinError(bIX+1, binErr);
      }

          std::cout << "FILE, LINE: " << __FILE__ << ", " << __LINE__ << std::endl;

      max = TMath::Max(max, getMax(tempHist_p));
      min = TMath::Min(min, getMinGTZero(tempHist_p));
    }

        std::cout << "FILE, LINE: " << __FILE__ << ", " << __LINE__ << std::endl;

    TLegend* leg_p = new TLegend(0.19, 0.99 - .035*nCentBins, 0.4, 0.99);
    leg_p->SetFillColor(0);
    leg_p->SetFillStyle(0);
    leg_p->SetBorderSize(0);
    leg_p->SetTextFont(43);
    leg_p->SetTextSize(13);    

      std::cout << "FILE, LINE: " << __FILE__ << ", " << __LINE__ << std::endl;

    std::cout << "MAX MIN: " << max << " " << min << std::endl;
    for(Int_t cI = 0; cI < nCentBins; ++cI){
      std::string comboStr = moreCombo + centStr[cI] + "_" + etaStr[eI];
      if(postCombo.size() != 0) comboStr = comboStr + "_" + postCombo;
      std::string histName = comboNameToTH1NameMap->at(comboStr);
      TH1D* tempHist_p = nameToHist->at(histName);
      
      tempHist_p->SetMaximum(max*4.);
      tempHist_p->SetMinimum(min/2.);
      
      std::string titleStr = "#frac{1}{N} #frac{dN}{d(#Sigmap_{T}-#rho#times A)}";
      tempHist_p->GetYaxis()->SetTitle(titleStr.c_str());
      tempHist_p->GetYaxis()->SetTitleOffset(2.2);
      
      tempHist_p->GetXaxis()->SetTitleOffset(1.2);
      
      tempHist_p->SetMarkerStyle(styles[cI%nStyles]);
      tempHist_p->SetMarkerSize(1);
      tempHist_p->SetMarkerColor(kPal.getColor(colors[cI%nColors]));
      tempHist_p->SetLineColor(kPal.getColor(colors[cI%nColors]));
      
      std::string centLabel = centStr[cI];
      
      centLabel.replace(centLabel.find("Cent"), 4, "");
      centLabel.replace(centLabel.find("to"), 2, "-");
      centLabel = centLabel + "%";
      
      if(cI == 0) tempHist_p->DrawCopy("HIST E1 P");      
      else tempHist_p->DrawCopy("HIST E1 P SAME");      
      
      Int_t maxBinPos = -1;
      Double_t max = -1;
      Double_t sigmaLeftVal = 100;
      Double_t sigmaRightVal = -100;
      
      std::cout << "FRACLEFT: " << fracLeft << std::endl;
      std::cout << "FRACRIGHT: " << fracRight << std::endl;
      
      for(Int_t bIX = 0; bIX < tempHist_p->GetXaxis()->GetNbins(); ++bIX){
	if(tempHist_p->GetBinContent(bIX+1) > max){
	  max = tempHist_p->GetBinContent(bIX+1);
	  maxBinPos= bIX;
	}
      }
  std::cout << "FILE, LINE: " << __FILE__ << ", " << __LINE__ << std::endl;
      
      Double_t totalLeft = tempHist_p->Integral(1, maxBinPos);
      Double_t totalRight = tempHist_p->Integral(maxBinPos, tempHist_p->GetXaxis()->GetNbins());
      std::cout << "TOTAL LEFT RIGHT: " << totalLeft << " " << totalRight << std::endl;
      
      for(Int_t bIX = 0; bIX < maxBinPos; ++bIX){
	Double_t tempIntLeft = tempHist_p->Integral(bIX+1, maxBinPos);

	if(tempIntLeft/totalLeft < fracLeft){
	  sigmaLeftVal = tempHist_p->GetBinCenter(bIX+1);
	  break;
	}
      }
      
      for(Int_t bIX = maxBinPos; bIX < tempHist_p->GetXaxis()->GetNbins(); ++bIX){
	Double_t tempIntRight = tempHist_p->Integral(maxBinPos, bIX+1);

	if(tempIntRight/totalRight >= fracRight){
	  sigmaRightVal = tempHist_p->GetBinCenter(bIX+1);
	  break;
	}
      }
      /*
      for(Int_t bIX = 0; bIX < maxBinPos/2 - 1; ++bIX){
	Double_t tempIntLeft = tempHist_p->Integral(maxBinPos - 1 - bIX, maxBinPos);
	Double_t tempIntRight = tempHist_p->Integral(maxBinPos, maxBinPos + 1 + bIX);
	  
	if(tempIntRight/totalRight > fracRight && sigmaRightVal < -99) sigmaRightVal = tempHist_p->GetBinCenter(maxBinPos + 1 + bIX);
	
	if(tempIntLeft/totalLeft > fracLeft && sigmaLeftVal > 99) sigmaLeftVal = tempHist_p->GetBinCenter(maxBinPos - 1 - bIX);
	
	if(sigmaRightVal > -100 && sigmaLeftVal < 100) break;
      }
      */
      TF1* fit_p = new TF1("fit_p", "gaus", sigmaLeftVal, sigmaRightVal);    
      tempHist_p->Fit("fit_p", "Q M N", "", sigmaLeftVal, sigmaRightVal);
      std::cout << centLabel << ", " << sigmaLeftVal << ", " << sigmaRightVal << std::endl;
      
      fit_p->SetLineStyle(2);
      fit_p->SetLineWidth(3);
      fit_p->SetLineColor(kPal.getColor(colors[cI%nColors]));
      fit_p->SetMarkerColor(kPal.getColor(colors[cI%nColors]));
      fit_p->SetMarkerSize(2);
      
      fit_p->DrawCopy("SAME");
      
      centLabel = centLabel + "; #mu=" + prettyString(fit_p->GetParameter(1), 2, false) + "; #sigma=" + prettyString(fit_p->GetParameter(2), 2, false); 
      leg_p->AddEntry(tempHist_p, centLabel.c_str(), "P L");      
      
      delete fit_p;
      std::cout << "FITTING: " << centStr[cI] << ", " << sigmaLeftVal << ", " << sigmaRightVal << std::endl;
    }
    
    leg_p->Draw("SAME");    
    
    std::string etaLabel = etaStr[eI];
    etaLabel.replace(etaLabel.find("Eta"), 3, "");
    etaLabel.replace(etaLabel.find("to"), 2, "<|#eta_{RC}|<");
    while(etaLabel.find("p") != std::string::npos){etaLabel.replace(etaLabel.find("p"), 1, ".");}
    if(etaLabel.find("0.0<") != std::string::npos) etaLabel.replace(etaLabel.find("0.0<"), 4, "");

      std::cout << "FILE, LINE: " << __FILE__ << ", " << __LINE__ << std::endl;

    TLatex* label_p = new TLatex();  
    label_p->SetTextFont(43);
    label_p->SetTextSize(14);
    label_p->SetNDC();
    
    label_p->DrawLatex(0.7, 0.96, etaLabel.c_str());
    
    std::string sigLeftStr = "Fit N#times #sigma_{L} = " + prettyString(nSigmaLeft, 1, false);
    std::string sigRightStr = "Fit N#times #sigma_{R} = " + prettyString(nSigmaRight, 1, false);
    
    label_p->DrawLatex(0.7, 0.915, sigLeftStr.c_str());
    label_p->DrawLatex(0.7, 0.87, sigRightStr.c_str());

    std::string mcStr = "Data";
    if(params_p->getIsMC()) mcStr = "MC";
    label_p->DrawLatex(0.7, 0.825, ("R=" + prettyString(rcR, 1, false) + ", #bf{" + mcStr + "}").c_str());

    gPad->SetLogy();
    gStyle->SetOptStat(0);


    std::string finalTag = moreCombo;
    if(postCombo.size() != 0) finalTag = finalTag + postCombo + "_";    
	
      
    std::string saveName = "pdfDir/" + dateStr + "/" + finalTag + etaStr[eI] + "_" + tagStr + "_" + dateStr + ".pdf";
    quietSaveAs(canv_p, saveName);
    delete canv_p;

    delete label_p;
    delete leg_p;    
  }  

  return;
}


int plotHistRC(const std::string inFileName)
{
  if(!checkFile(inFileName) || inFileName.find(".root") == std::string::npos){
    std::cout << "ERROR: Given inFileName \'" << inFileName << "\' is invalid. return 1" << std::endl;
    return 1;
  }
  
  const Int_t nPtBins = 3;
  const Double_t ptBinsLow[nPtBins] = {50., 100., 200.};
  const Double_t ptBinsHigh[nPtBins] = {100., 200., 500.};

  std::vector<std::string> ptBinsStr;
  for(Int_t pI = 0; pI < nPtBins; ++pI){
    ptBinsStr.push_back("Pt"+prettyString(ptBinsLow[pI],1,true)+"to"+prettyString(ptBinsHigh[pI],1,true));
  }

  TDatime* date = new TDatime();
  const std::string dateStr = std::to_string(date->GetDate());
  delete date;  
  
  TFile* inFile_p = new TFile(inFileName.c_str(), "READ");
  paramPropagator params;
  params.setupFromROOT(inFile_p);

  std::map<std::string, std::string> paramFound = params.getParamFound();
  const bool doEvt = paramFound["EVTINPUT"].size() != 0;
  const bool doJet = paramFound["IMBINPUT"].size() != 0;
  const bool doRef = params.getImbRefPtStr().size() != 0;

  std::vector<std::string> epStr = {"FullPlane"};
  if(doEvt){
    epStr.push_back("InPlane");
    epStr.push_back("OutPlane");
  }

  const Double_t ptCut = params.getPtCut();

  const Int_t nCentBins = params.getNCentBins();
  const Int_t nEtaBins = params.getNEtaBins();

  std::vector<double> centBinsLow = params.getCentBinsLow();
  std::vector<double> centBinsHigh = params.getCentBinsHigh();

  std::vector<double> etaBinsLow = params.getEtaBinsLow();
  std::vector<double> etaBinsHigh = params.getEtaBinsHigh();

  std::vector<std::string> centStr, etaStr;
  std::map<std::string, int> nameCounterTH1;
  std::map<std::string, std::string> comboNameToTH1NameMap;
  std::map<std::string, int> nameCounterTH2;  

  for(Int_t cI = 0; cI < nCentBins; ++cI){
    std::string temp = "Cent" + std::to_string(((Int_t)centBinsLow[cI])) + "to" + std::to_string(((Int_t)centBinsHigh[cI]));
    centStr.push_back(temp);
  }

  for(Int_t eI = 0; eI < nEtaBins; ++eI){
    std::string temp = "Eta" + prettyString(etaBinsLow[eI],1,true) + "to" + prettyString(etaBinsHigh[eI],1,true);
    etaStr.push_back(temp);
  }

  
  for(auto const & eta : etaStr){
    for(auto const & cent : centStr){
      std::string name = cent + "_" + eta;
      nameCounterTH1[name] = 0;
    }

    nameCounterTH2[eta] = 0;    
  }
  
  std::vector<std::string> listTH1D = returnRootFileContentsList(inFile_p, "TH1D", "hist");
  std::vector<std::string> listTH2D = returnRootFileContentsList(inFile_p, "TH2D", "histRCMin");

  for(auto const & th1 : listTH1D){
    bool isFound = false;
    for(auto const & iter : nameCounterTH1){

      std::string preNom = "histRCMinRhoFlow_";
      std::string ptStr = "";

      if(th1.find("histRCMinRho_") != std::string::npos) preNom = "histRCMinRho_";
      else if(th1.find("histRCMinRhoFlow_") != std::string::npos) preNom = "histRCMinRhoFlow_";
      else if(th1.find("histRho_") != std::string::npos) preNom = "histRho_";
      else if(th1.find("histRhoFlow_") != std::string::npos) preNom = "histRhoFlow_";
      else if(th1.find("histJetRho_") != std::string::npos){
	preNom = "histJetRho_";
	for(unsigned int pI = 0; pI < ptBinsStr.size(); ++pI){
	  if(th1.find(ptBinsStr[pI]) != std::string::npos) ptStr = ptBinsStr[pI];	 
	}	
      }      
      else if(th1.find("histJetRhoFlow_") != std::string::npos){
	preNom = "histJetRhoFlow_";
	for(unsigned int pI = 0; pI < ptBinsStr.size(); ++pI){
	  if(th1.find(ptBinsStr[pI]) != std::string::npos) ptStr = ptBinsStr[pI];	  
	}	
      }
      else if(th1.find("histRefRho_") != std::string::npos){
	preNom = "histRefRho_";
	for(unsigned int pI = 0; pI < ptBinsStr.size(); ++pI){
	  if(th1.find(ptBinsStr[pI]) != std::string::npos) ptStr = ptBinsStr[pI];	  
	}	
      }      
      else if(th1.find("histRefRhoFlow_") != std::string::npos){
	preNom = "histRefRhoFlow_";
	for(unsigned int pI = 0; pI < ptBinsStr.size(); ++pI){
	  if(th1.find(ptBinsStr[pI]) != std::string::npos) ptStr = ptBinsStr[pI];	 
	}	
      }
      else continue;
      
      std::string evtStr = "";
      for(unsigned int epI = 0; epI < epStr.size(); ++epI){
	if(th1.find(epStr[epI]) != std::string::npos){
	  evtStr = epStr[epI];
	  break;
	}
      }

      std::cout << "PREBUILDIN " << th1 << std::endl;
      std::cout << "NAME: " << iter.first << std::endl;
      std::string name = iter.first;
      std::string eta = name.substr(name.find("_")+1, name.size());
      std::string cent = name.substr(0, name.find("_"));

      name = name + "_" + evtStr;
      name = preNom + name;
      if(ptStr.size() != 0) name = name + "_" + ptStr;
      
      if(th1.find(eta) != std::string::npos){
	if(th1.find(cent) != std::string::npos){

	  std::cout << "BUILDING NAME: " << name << std::endl;
	  nameCounterTH1[name]++;
	  comboNameToTH1NameMap[name] = th1;
	  isFound = true;
	  break;
	}
      }    
    }

    if(!isFound) std::cout << "\'" << th1 << "\' not found!" << std::endl;    
  }

  for(auto const & iter : nameCounterTH1){
    if(iter.second != 1) std::cout << "WARNING: " << iter.first << " has counts != 1" << std::endl;
  }

  std::map<std::string, TH1D*> nameToHistRCMinMapTH1, nameToHistRCFlowMinMapTH1, nameToHistRhoMapTH1, nameToHistRhoFlowMapTH1, nameToHistJetRhoMapTH1, nameToHistJetRhoFlowMapTH1, nameToHistRefRhoMapTH1, nameToHistRefRhoFlowMapTH1, nameToHistRCMapTH1;

  TIter next(inFile_p->GetListOfKeys());
  TKey* key = NULL;
  while((key = (TKey*)next())){
    const std::string name = key->GetName();
    const std::string className = key->GetClassName();

    if(name.find("histRCMinRho_") != std::string::npos){    
      if(className.find("TH1D") != std::string::npos) nameToHistRCMinMapTH1[name] = (TH1D*)key->ReadObj();
    }
    else if(name.find("histRCMinRhoFlow_") != std::string::npos){    
      if(className.find("TH1D") != std::string::npos) nameToHistRCFlowMinMapTH1[name] = (TH1D*)key->ReadObj();
    }
    else if(name.find("histRho_") != std::string::npos){    
      if(className.find("TH1D") != std::string::npos) nameToHistRhoMapTH1[name] = (TH1D*)key->ReadObj();
    }
    else if(name.find("histRhoFlow_") != std::string::npos){    
      if(className.find("TH1D") != std::string::npos) nameToHistRhoFlowMapTH1[name] = (TH1D*)key->ReadObj();
    }
    else if(name.find("histJetRho_") != std::string::npos){    
      if(className.find("TH1D") != std::string::npos) nameToHistJetRhoMapTH1[name] = (TH1D*)key->ReadObj();
    }
    else if(name.find("histJetRhoFlow_") != std::string::npos){    
      if(className.find("TH1D") != std::string::npos) nameToHistJetRhoFlowMapTH1[name] = (TH1D*)key->ReadObj();
    }
    else if(name.find("histRefRho_") != std::string::npos){    
      if(className.find("TH1D") != std::string::npos) nameToHistRefRhoMapTH1[name] = (TH1D*)key->ReadObj();
    }
    else if(name.find("histRefRhoFlow_") != std::string::npos){    
      if(className.find("TH1D") != std::string::npos) nameToHistRefRhoFlowMapTH1[name] = (TH1D*)key->ReadObj();
    }
    else if(name.find("histRC_") != std::string::npos){    
      if(className.find("TH1D") != std::string::npos) nameToHistRCMapTH1[name] = (TH1D*)key->ReadObj();
    }
  }

  //Start plotting
  checkMakeDir("pdfDir");
  checkMakeDir("pdfDir/" + dateStr);
  
  if(ptCut < 0.001){

    for(unsigned int epI = 0; epI < epStr.size(); ++epI){
      runPlotter(&comboNameToTH1NameMap, &nameToHistRCMinMapTH1, &params, dateStr, "histRCMinRho_", epStr[epI]);
      
      runPlotter(&comboNameToTH1NameMap, &nameToHistRCFlowMinMapTH1, &params, dateStr, "histRCMinRhoFlow_", epStr[epI]);

      runPlotter(&comboNameToTH1NameMap, &nameToHistRhoMapTH1, &params, dateStr, "histRho_", epStr[epI]);
      
      runPlotter(&comboNameToTH1NameMap, &nameToHistRhoFlowMapTH1, &params, dateStr, "histRhoFlow_", epStr[epI]);
      
      
      if(doJet){
	for(unsigned int pI = 0; pI < ptBinsStr.size(); ++pI){	
	  runPlotter(&comboNameToTH1NameMap, &nameToHistJetRhoMapTH1, &params, dateStr, "histJetRho_", epStr[epI] + "_" + ptBinsStr[pI]);    
	  runPlotter(&comboNameToTH1NameMap, &nameToHistJetRhoFlowMapTH1, &params, dateStr, "histJetRhoFlow_", epStr[epI] + "_" + ptBinsStr[pI]);
	  
	  if(doRef){
	    runPlotter(&comboNameToTH1NameMap, &nameToHistRefRhoMapTH1, &params, dateStr, "histRefRho_", epStr[epI] + "_" + ptBinsStr[pI]);    
	    runPlotter(&comboNameToTH1NameMap, &nameToHistRefRhoFlowMapTH1, &params, dateStr, "histRefRhoFlow_", epStr[epI] + "_" + ptBinsStr[pI]);
	  }
	}
      }
    }
  }

  inFile_p->Close();
  delete inFile_p;

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc != 2){
    std::cout << "Usage: ./bin/plotHistRC.exe <inFileName>. return 1" << std::endl;
    return 1;
  }

  int retVal = 0;
  retVal += plotHistRC(argv[1]);
  return retVal;
}
