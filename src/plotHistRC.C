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


int plotHistRC(const std::string inFileName)
{
  if(!checkFile(inFileName) || inFileName.find(".root") == std::string::npos){
    std::cout << "ERROR: Given inFileName \'" << inFileName << "\' is invalid. return 1" << std::endl;
    return 1;
  }

  const Int_t nStyles = 4;
  const Int_t styles[nStyles] = {24, 25, 28, 46};

  kirchnerPalette kPal;
  const Int_t nColors = 5;
  const Int_t colors[nColors] = {0, 2, 3, 5, 6}; // this are positions in the kirchnerpalette class
  
  TDatime* date = new TDatime();
  const std::string dateStr = std::to_string(date->GetDate());
  delete date;  
  
  TFile* inFile_p = new TFile(inFileName.c_str(), "READ");
  paramPropagator params;
  params.setupFromROOT(inFile_p);

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
  
  std::vector<std::string> listTH1D = returnRootFileContentsList(inFile_p, "TH1D", "histRCMin");
  std::vector<std::string> listTH2D = returnRootFileContentsList(inFile_p, "TH2D", "histRCMin");

  for(auto const & th1 : listTH1D){
    bool isFound = false;
    for(auto const & iter : nameCounterTH1){
      std::string name = iter.first;
      std::string eta = name.substr(name.find("_")+1, name.size());
      std::string cent = name.substr(0, name.find("_"));

      if(th1.find(eta) != std::string::npos){
	if(th1.find(cent) != std::string::npos){
	  nameCounterTH1[name]++;
	  comboNameToTH1NameMap[name] = th1;
	  isFound = true;
	  break;
	}
      }    
    }

    if(!isFound) std::cout << "Its not found!" << std::endl;    
  }

  for(auto const & iter : nameCounterTH1){
    if(iter.second != 1) std::cout << "WARNING: " << iter.first << " has counts != 1" << std::endl;
  }

  std::map<std::string, TH1D*> nameToHistMapTH1;

  TIter next(inFile_p->GetListOfKeys());
  TKey* key = NULL;
  while((key = (TKey*)next())){
    const std::string name = key->GetName();
    const std::string className = key->GetClassName();

    if(name.find("histRCMin") == std::string::npos) continue;
    
    if(className.find("TH1D") != std::string::npos) nameToHistMapTH1[name] = (TH1D*)key->ReadObj();
  }

  //Start plotting
  checkMakeDir("pdfDir");
  checkMakeDir("pdfDir/" + dateStr);

  TLine* line_p = new TLine();
  line_p->SetLineStyle(2);
  line_p->SetLineWidth(2);
  
  TLatex* label_p = new TLatex();  
  label_p->SetTextFont(43);
  label_p->SetTextSize(14);
  label_p->SetNDC();
  
  Double_t nSigmaLeft = 3.0;
  Double_t nSigmaRight = 1.0;
  Double_t fracLeft = calcQuickGaus(nSigmaLeft);
  Double_t fracRight = calcQuickGaus(nSigmaRight);

  for(Int_t eI = 0; eI < nEtaBins; ++eI){
    TCanvas* canv_p = new TCanvas("temp", "", 450, 450);
    canv_p->SetTopMargin(0.01);
    canv_p->SetLeftMargin(0.18);
    canv_p->SetBottomMargin(0.1);
    canv_p->SetRightMargin(0.01);

    //pre-processing step
    Double_t max = -1;
    Double_t min = 100000000000;
    
    for(Int_t cI = 0; cI < nCentBins; ++cI){
      std::string comboStr = centStr[cI] + "_" + etaStr[eI];
      std::string histName = comboNameToTH1NameMap[comboStr];
      TH1D* tempHist_p = nameToHistMapTH1[histName];

      Double_t scaleFactor = tempHist_p->Integral();

      for(Int_t bIX = 0; bIX < tempHist_p->GetXaxis()->GetNbins(); ++bIX){
	Double_t width = tempHist_p->GetBinWidth(bIX+1);
	Double_t binVal = tempHist_p->GetBinContent(bIX+1)/(width*scaleFactor);
	Double_t binErr = tempHist_p->GetBinError(bIX+1)/(width*scaleFactor);

	tempHist_p->SetBinContent(bIX+1, binVal);
	tempHist_p->SetBinError(bIX+1, binErr);
      }

      max = TMath::Max(max, getMax(tempHist_p));
      min = TMath::Min(min, getMinGTZero(tempHist_p));
    }

   
    TLegend* leg_p = new TLegend(0.2, 0.75, 0.4, 0.99);
    leg_p->SetFillColor(0);
    leg_p->SetFillStyle(0);
    leg_p->SetBorderSize(0);
    leg_p->SetTextFont(43);
    leg_p->SetTextSize(14);    
        
    std::cout << "MAX MIN: " << max << " " << min << std::endl;
    for(Int_t cI = 0; cI < nCentBins; ++cI){
      std::string comboStr = centStr[cI] + "_" + etaStr[eI];
      std::string histName = comboNameToTH1NameMap[comboStr];
      TH1D* tempHist_p = nameToHistMapTH1[histName];      
      
      tempHist_p->SetMaximum(max*2.);
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

      Double_t totalLeft = tempHist_p->Integral(1, maxBinPos);
      Double_t totalRight = tempHist_p->Integral(maxBinPos, tempHist_p->GetXaxis()->GetNbins());
      std::cout << "TOTAL LEFT RIGHT: " << totalLeft << " " << totalRight << std::endl;
      
      for(Int_t bIX = 0; bIX < maxBinPos/2 - 1; ++bIX){
	Double_t tempIntLeft = tempHist_p->Integral(maxBinPos - 1 - bIX, maxBinPos);
	Double_t tempIntRight = tempHist_p->Integral(maxBinPos, maxBinPos + 1 + bIX);

	if(tempIntRight/totalRight > fracRight && sigmaRightVal < -99) sigmaRightVal = tempHist_p->GetBinCenter(maxBinPos + 1 + bIX);

	if(tempIntLeft/totalLeft > fracLeft && sigmaLeftVal > 99) sigmaLeftVal = tempHist_p->GetBinCenter(maxBinPos - 1 - bIX);

	if(sigmaRightVal > -100 && sigmaLeftVal < 100) break;
      }
      
      TF1* fit_p = new TF1("fit_p", "gaus", sigmaLeftVal, sigmaRightVal);    
      tempHist_p->Fit("fit_p", "Q M N", "", sigmaLeftVal, sigmaRightVal);

      fit_p->SetLineStyle(2);
      fit_p->SetLineWidth(3);
      fit_p->SetLineColor(kPal.getColor(colors[cI%nColors]));
      fit_p->SetMarkerColor(kPal.getColor(colors[cI%nColors]));
      fit_p->SetMarkerSize(2);

      fit_p->DrawCopy("SAME");

      centLabel = centLabel + "; #mu=" + prettyString(fit_p->GetParameter(1), 2, false); 
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
    
    label_p->DrawLatex(0.7, 0.94, etaLabel.c_str());

    std::string sigLeftStr = "Fit N#times #sigma_{L} = " + prettyString(nSigmaLeft, 1, false);
    std::string sigRightStr = "Fit N#times #sigma_{R} = " + prettyString(nSigmaRight, 1, false);

    label_p->DrawLatex(0.7, 0.89, sigLeftStr.c_str());
    label_p->DrawLatex(0.7, 0.84, sigRightStr.c_str());

    gPad->SetLogy();
    gStyle->SetOptStat(0);
    
    std::string saveName = "pdfDir/" + dateStr + "/rCMinRho_" + etaStr[eI] + "_" + dateStr + ".pdf";
    quietSaveAs(canv_p, saveName);
    delete canv_p;

    delete leg_p;    
  }

  delete label_p;
  delete line_p;  
    
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
