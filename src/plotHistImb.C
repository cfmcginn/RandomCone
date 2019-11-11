//Author: Chris McGinn (2019.11.05)

//cpp
#include <cmath>
#include <iostream>
#include <map>
#include <vector>

//ROOT
#include "TCanvas.h"
#include "TDatime.h"
#include "TFile.h"
#include "TH1D.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TNamed.h"
#include "TPad.h"
#include "TStyle.h"

//Local
#include "include/checkMakeDir.h"
#include "include/histDefUtility.h"
#include "include/plotUtilities.h"
#include "include/returnRootFileContentsList.h"
#include "include/stringUtil.h"

void doDivideWithZeros(TH1D* histNum_p, TH1D* histDenom_p, std::vector<int>* binTracker)
{
  std::vector<int> zeroDenomVals;

  for(Int_t hI = 0; hI < histDenom_p->GetXaxis()->GetNbins(); ++hI){
    if(TMath::Abs(histDenom_p->GetBinContent(hI+1)) < TMath::Power(10, -30)){
      zeroDenomVals.push_back(hI);
      histDenom_p->SetBinContent(hI+1, 1.0);
    }

    if(TMath::Abs(histDenom_p->GetBinContent(hI+1)) > TMath::Power(10, 30)){
      zeroDenomVals.push_back(hI);
    }
  }
  histNum_p->Divide(histDenom_p);

  for(unsigned int hI = 0; hI < zeroDenomVals.size(); ++hI){
    histNum_p->SetBinContent(zeroDenomVals[hI]+1, 10000);
    histNum_p->SetBinError(zeroDenomVals[hI]+1, 0.0);
  }

  if(binTracker != NULL){
    binTracker->clear();
    for(unsigned int hI = 0; hI < zeroDenomVals.size(); ++hI){
      binTracker->push_back(zeroDenomVals[hI]);
    }
  }

  return;
}


void doRevertDivideZeros(TH1D* hist_p, std::vector<int>* binTracker)
{
  for(unsigned int bI = 0; bI < binTracker->size(); ++bI){
    hist_p->SetBinContent(binTracker->at(bI)+1, 0.0);
  }
  
  return;
}


int plotHistImb(const std::string inFileName1, const std::string inFileName2, std::string globalStr = "", const std::string file1LegStr = "File 1", const std::string file2LegStr = "File 2")
{
  if(!checkFileExt(inFileName1, "root")) return 1;
  if(!checkFileExt(inFileName2, "root")) return 1;

  if(isStrSame(globalStr, "2")) globalStr = "0p2";
  if(isStrSame(globalStr, "3")) globalStr = "0p3";
  if(isStrSame(globalStr, "4")) globalStr = "0p4";
  if(isStrSame(globalStr, "6")) globalStr = "0p6";
  if(isStrSame(globalStr, "8")) globalStr = "0p8";
  if(isStrSame(globalStr, "10")) globalStr = "1p0";
  
  std::string globalStr2 = globalStr;
  if(globalStr.size() != 0){
    globalStr = "R" + globalStr;
    if(globalStr2.find("p") != std::string::npos) globalStr2.replace(globalStr2.find("p"), 1, ".");
    globalStr2 = "R=" + globalStr2;
  }
  
  TDatime* date = new TDatime();
  const std::string dateStr = std::to_string(date->GetDate());
  delete date;
  
  checkMakeDir("pdfDir");
  checkMakeDir("pdfDir/" + dateStr);
  
  TFile* inFile1_p = new TFile(inFileName1.c_str(), "READ");
  std::vector<std::string> tnameList = returnRootFileContentsList(inFile1_p, "TNamed");
  std::map<std::string, std::string> tnameMap1, tnameMap2;

  for(auto const & iter : tnameList){
    TNamed* tempName = (TNamed*)inFile1_p->Get(iter.c_str());
    std::string name = tempName->GetName();
    name = name.substr(name.rfind("/")+1, name.size());
    tnameMap1[name] = tempName->GetTitle();
  }

  TFile* inFile2_p = new TFile(inFileName2.c_str(), "READ");
  tnameList = returnRootFileContentsList(inFile2_p, "TNamed");
  for(auto const & iter : tnameList){
    TNamed* tempName = (TNamed*)inFile2_p->Get(iter.c_str());
    std::string name = tempName->GetName();
    name = name.substr(name.rfind("/")+1, name.size());
    tnameMap2[name] = tempName->GetTitle();
  }

  for(auto const& iter : tnameMap1){
    if(!isStrSame(tnameMap2[iter.first], iter.second)){
      std::cout << "Map of params in file 1 \'" << inFileName1 << "\' does not match those in inFileName2 \'" << inFileName2 << "\'. return 1" << std::endl;
      return 1;
    }
  }

  for(auto const& iter : tnameMap1){std::cout << iter.first << ", " << iter.second << std::endl;}

  std::cout << tnameMap1["nJtPtBins"] << ", " << tnameMap1["nJtAbsEtaBins"] << std::endl;
  const Int_t nMaxJtPtBins = 50;
  const Int_t nMaxJtAbsEtaBins = 50;
  const Int_t nJtPtBins = std::stoi(tnameMap1["nJtPtBins"]);
  const Int_t nJtAbsEtaBins = std::stoi(tnameMap1["nJtAbsEtaBins"]);

  if(nMaxJtPtBins < nJtPtBins){
    std::cout << "Given nJtPtBins \'" << nJtPtBins << "\' is greater than nMaxJtPtBins \'" << nMaxJtPtBins << "\'. return 1" << std::endl;
    return 1;
  }
  if(nMaxJtAbsEtaBins < nJtAbsEtaBins){
    std::cout << "Given nJtAbsEtaBins \'" << nJtAbsEtaBins << "\' is greater than nMaxJtAbsEtaBins \'" << nMaxJtAbsEtaBins << "\'. return 1" << std::endl;
    return 1;
  }
  
  std::vector<std::string> jtPtBinsStr = commaSepStringToVect(tnameMap1["jtPtBins"]);
  std::vector<std::string> jtAbsEtaBinsStr = commaSepStringToVect(tnameMap1["jtAbsEtaBins"]);
  Double_t jtPtBins[nMaxJtPtBins+1];
  Double_t jtAbsEtaBins[nMaxJtAbsEtaBins+1];

  
  for(int jI = 0; jI < nJtPtBins; ++jI){
    std::string ptLowStr = jtPtBinsStr[jI].substr(jtPtBinsStr[jI].find("JtPt")+4, jtPtBinsStr[jI].size());
    while(ptLowStr.find("p") != std::string::npos){
      ptLowStr.replace(ptLowStr.find("p"), 1, ".");
    }
    
    std::string ptHighStr = ptLowStr.substr(ptLowStr.find("to")+2, ptLowStr.size());
    ptLowStr.replace(ptLowStr.find("to"), ptLowStr.size(), "");

    double ptLow = std::stod(ptLowStr);    
    double ptHigh = std::stod(ptHighStr);

    if(jI == 0) jtPtBins[jI] = ptLow;
    jtPtBins[jI+1] = ptHigh;
  }
  
  
  for(int jI = 0; jI < nJtAbsEtaBins; ++jI){
    std::string absEtaLowStr = jtAbsEtaBinsStr[jI].substr(jtAbsEtaBinsStr[jI].find("JtAbsEta")+8, jtAbsEtaBinsStr[jI].size());
    while(absEtaLowStr.find("p") != std::string::npos){
      absEtaLowStr.replace(absEtaLowStr.find("p"), 1, ".");
    }
    
    std::string absEtaHighStr = absEtaLowStr.substr(absEtaLowStr.find("to")+2, absEtaLowStr.size());
    absEtaLowStr.replace(absEtaLowStr.find("to"), absEtaLowStr.size(), "");

    std::cout << absEtaLowStr << ", " << absEtaHighStr << std::endl;
    double absEtaLow = std::stod(absEtaLowStr);    
    double absEtaHigh = std::stod(absEtaHighStr);

    if(jI == 0) jtAbsEtaBins[jI] = absEtaLow;
    jtAbsEtaBins[jI+1] = absEtaHigh;
  }

  std::cout << "Bin check: " << std::endl;
  for(int jI = 0; jI < nJtAbsEtaBins+1; ++jI){
    std::cout << jtAbsEtaBins[jI] << std::endl;
  }

  const Int_t nFiles = 2;
  TH1D* xjMean_p[nFiles][nMaxJtPtBins];
  TH1D* xjSigma_p[nFiles][nMaxJtPtBins];
  TH1D* xjSigmaOverMean_p[nFiles][nMaxJtPtBins];

  for(Int_t fI = 0; fI < nFiles; ++fI){
    for(Int_t jI = 0; jI < nJtPtBins; ++jI){
      xjMean_p[fI][jI] = new TH1D(("xjMean_File" + std::to_string(fI) + "_" + jtPtBinsStr[jI] + "_h").c_str(), ";|#eta|;#LTx_{J}#GT", nJtAbsEtaBins, jtAbsEtaBins);
      xjSigma_p[fI][jI] = new TH1D(("xjSigma_File" + std::to_string(fI) + "_" + jtPtBinsStr[jI] + "_h").c_str(), ";|#eta|;#sigma(x_{J})", nJtAbsEtaBins, jtAbsEtaBins);
      xjSigmaOverMean_p[fI][jI] = new TH1D(("xjSigmaOverMean_File" + std::to_string(fI) + "_" + jtPtBinsStr[jI] + "_h").c_str(), ";|#eta|;#sigma(x_{J})/#LTx_{J}#GT", nJtAbsEtaBins, jtAbsEtaBins);

      centerTitles({xjMean_p[fI][jI], xjSigma_p[fI][jI], xjSigmaOverMean_p[fI][jI]});
    }
  }

  TLatex* label_p = new TLatex();
  label_p->SetTextFont(43);
  label_p->SetTextSize(18);
  label_p->SetNDC();

  TLegend* leg_p = new TLegend(0.6, 0.7, 0.9, 0.9);
  leg_p->SetTextFont(43);
  leg_p->SetTextSize(18);
  leg_p->SetFillColor(0);
  leg_p->SetBorderSize(0);
  leg_p->SetFillStyle(0);

  TH1D* dummyHist_p = new TH1D("dummyHist_h", "", 10, 0, 1);
  dummyHist_p->SetMarkerStyle(24);
  dummyHist_p->SetMarkerColor(kRed);
  dummyHist_p->SetLineColor(kRed);
  
  for(int jI = 0; jI < nJtPtBins; ++jI){
    for(int jI2 = 0; jI2 < nJtAbsEtaBins; ++jI2){
      std::string histName = "xj_" + jtPtBinsStr[jI] + "_" + jtAbsEtaBinsStr[jI2] + "_h";
      TCanvas* canv_p = new TCanvas("canv_p", "", 450, 450);
      canv_p->SetTopMargin(0.02);
      canv_p->SetRightMargin(0.02);
      canv_p->SetLeftMargin(0.14);
      canv_p->SetBottomMargin(0.14);
      
      TH1D* hist1_p = (TH1D*)inFile1_p->Get(histName.c_str());
      TH1D* hist2_p = (TH1D*)inFile2_p->Get(histName.c_str());

      canv_p->cd();
      
      hist1_p->SetMarkerSize(1);
      hist1_p->SetMarkerStyle(24);
      hist1_p->SetMarkerColor(1);
      hist1_p->SetLineColor(1);
      
      hist2_p->SetMarkerSize(1);
      hist2_p->SetMarkerStyle(25);
      hist2_p->SetMarkerColor(kRed);
      hist2_p->SetLineColor(kRed);

      centerTitles(hist1_p);

      hist1_p->Sumw2();
      hist2_p->Sumw2();

      hist1_p->Scale(1./hist1_p->Integral());
      hist2_p->Scale(1./hist2_p->Integral());

      hist1_p->SetMinimum(0.0);
      
      hist1_p->DrawCopy("HIST E1 P");
      hist2_p->DrawCopy("HIST E1 P SAME");

      if(globalStr.size() != 0) label_p->DrawLatex(0.2, 0.92, globalStr2.c_str());
      label_p->DrawLatex(0.2, 0.84, (prettyString(jtPtBins[jI], 1, false) + " < p_{T} < " + prettyString(jtPtBins[jI+1], 1, false)).c_str());
      label_p->DrawLatex(0.2, 0.76, (prettyString(jtAbsEtaBins[jI], 1, false) + " < |#eta| < " + prettyString(jtAbsEtaBins[jI+1], 1, false)).c_str());

      if(jI == 0 && jI2 == 0){
	leg_p->AddEntry(hist1_p, file1LegStr.c_str(), "P L");
	leg_p->AddEntry(hist2_p, file2LegStr.c_str(), "P L");
      }
      
      gStyle->SetOptStat(0);

      xjMean_p[0][jI]->SetBinContent(jI2+1, hist1_p->GetMean());
      xjMean_p[0][jI]->SetBinError(jI2+1, hist1_p->GetMeanError());
      xjSigma_p[0][jI]->SetBinContent(jI2+1, hist1_p->GetStdDev());
      xjSigma_p[0][jI]->SetBinError(jI2+1, hist1_p->GetStdDevError());

      if(hist1_p->GetMean() > TMath::Power(10, -20)){
	xjSigmaOverMean_p[0][jI]->SetBinContent(jI2+1, hist1_p->GetStdDev()/hist1_p->GetMean());
	xjSigmaOverMean_p[0][jI]->SetBinError(jI2+1, hist1_p->GetStdDevError()/hist1_p->GetMean());
      }
      else{
	xjSigmaOverMean_p[0][jI]->SetBinContent(jI2+1, 0.0);
	xjSigmaOverMean_p[0][jI]->SetBinError(jI2+1, 0.0);
      }

      xjMean_p[1][jI]->SetBinContent(jI2+1, hist2_p->GetMean());
      xjMean_p[1][jI]->SetBinError(jI2+1, hist2_p->GetMeanError());
      xjSigma_p[1][jI]->SetBinContent(jI2+1, hist2_p->GetStdDev());
      xjSigma_p[1][jI]->SetBinError(jI2+1, hist2_p->GetStdDevError());

      if(hist2_p->GetMean() > TMath::Power(10, -20)){
	xjSigmaOverMean_p[1][jI]->SetBinContent(jI2+1, hist2_p->GetStdDev()/hist2_p->GetMean());
	xjSigmaOverMean_p[1][jI]->SetBinError(jI2+1, hist2_p->GetStdDevError()/hist2_p->GetMean());
      }
      else{
	xjSigmaOverMean_p[1][jI]->SetBinContent(jI2+1, 0.0);
	xjSigmaOverMean_p[1][jI]->SetBinError(jI2+1, 0.0);
      }

      leg_p->Draw("SAME");
      
      std::string saveName = "pdfDir/" + dateStr + "/" + histName.substr(0, histName.rfind("_h")) + "_" + globalStr +  "_" + dateStr + ".pdf";
      quietSaveAs(canv_p, saveName);
      delete canv_p;
    }
  }

  const Double_t padSplit = 0.35;
    
  for(Int_t jI = 0; jI < nJtPtBins; ++jI){
    TCanvas* canv_p = new TCanvas("canv_p", "", 450, 450);
    canv_p->SetTopMargin(0.01);
    canv_p->SetRightMargin(0.01);
    canv_p->SetLeftMargin(0.01);
    canv_p->SetBottomMargin(0.01);

    TPad* pads_p[2];
    pads_p[0] = new TPad("pad0", "", 0.0, padSplit, 1.0, 1.0);
    pads_p[1] = new TPad("pad1", "", 0.0, 0.0, 1.0, padSplit);
    
    pads_p[0]->SetTopMargin(0.02);
    pads_p[0]->SetLeftMargin(0.14);
    pads_p[0]->SetBottomMargin(0.001);
    pads_p[0]->SetRightMargin(0.02);
    
    pads_p[1]->SetTopMargin(0.02);
    pads_p[1]->SetLeftMargin(0.14);
    pads_p[1]->SetBottomMargin(0.14/padSplit);
    pads_p[1]->SetRightMargin(0.02);
    
    canv_p->cd();
    pads_p[0]->Draw("SAME");
    
    canv_p->cd();
    pads_p[1]->Draw("SAME");
    
    canv_p->cd();
    pads_p[0]->cd();
    
    xjMean_p[0][jI]->SetMarkerSize(1);
    xjMean_p[0][jI]->SetMarkerStyle(24);
    xjMean_p[0][jI]->SetMarkerColor(1);
    xjMean_p[0][jI]->SetLineColor(1);
    
    xjMean_p[1][jI]->SetMarkerSize(1);
    xjMean_p[1][jI]->SetMarkerStyle(25);
    xjMean_p[1][jI]->SetMarkerColor(kRed);
    xjMean_p[1][jI]->SetLineColor(kRed); 

    xjMean_p[1][jI]->GetXaxis()->SetTitleFont(43);
    xjMean_p[1][jI]->GetYaxis()->SetTitleFont(43);
    xjMean_p[1][jI]->GetXaxis()->SetTitleSize(18);
    xjMean_p[1][jI]->GetYaxis()->SetTitleSize(18);

    xjMean_p[1][jI]->GetXaxis()->SetLabelFont(43);
    xjMean_p[1][jI]->GetYaxis()->SetLabelFont(43);
    xjMean_p[1][jI]->GetXaxis()->SetLabelSize(16);
    xjMean_p[1][jI]->GetYaxis()->SetLabelSize(16);

    xjMean_p[0][jI]->GetXaxis()->SetTitleFont(43);
    xjMean_p[0][jI]->GetYaxis()->SetTitleFont(43);
    xjMean_p[0][jI]->GetXaxis()->SetTitleSize(18);
    xjMean_p[0][jI]->GetYaxis()->SetTitleSize(18);

    xjMean_p[0][jI]->GetXaxis()->SetLabelFont(43);
    xjMean_p[0][jI]->GetYaxis()->SetLabelFont(43);
    xjMean_p[0][jI]->GetXaxis()->SetLabelSize(16);
    xjMean_p[0][jI]->GetYaxis()->SetLabelSize(16);


    Double_t max = TMath::Max(getMax(xjMean_p[0][jI]), getMax(xjMean_p[1][jI]));
    Double_t min = TMath::Min(getMinGTZero(xjMean_p[0][jI]), getMinGTZero(xjMean_p[1][jI]));
    
    double delta = max - min;
    max += 0.15*delta;
    min -= 0.15*delta;
    
    max = 1.45;
    min = 0.55;
    
    xjMean_p[0][jI]->SetMaximum(max);
    xjMean_p[0][jI]->SetMinimum(min);
    
    xjMean_p[0][jI]->DrawCopy("HIST E1 P");
    xjMean_p[1][jI]->DrawCopy("SAME HIST E1 P");    

    if(globalStr.size() != 0) label_p->DrawLatex(0.2, 0.92, globalStr2.c_str());
    label_p->DrawLatex(0.2, 0.84, (prettyString(jtPtBins[jI], 1, false) + " < p_{T} < " + prettyString(jtPtBins[jI+1], 1, false)).c_str());

    leg_p->Draw("SAME");

    canv_p->cd();
    pads_p[1]->cd();

    std::vector<int> binTracker;
    doDivideWithZeros(xjMean_p[1][jI], xjMean_p[0][jI], &binTracker);
    xjMean_p[1][jI]->GetYaxis()->SetTitle((file2LegStr + "/" + file1LegStr).c_str());
    xjMean_p[1][jI]->SetMaximum(1.15);
    xjMean_p[1][jI]->SetMinimum(0.85);

    xjMean_p[1][jI]->GetYaxis()->SetNdivisions(505);
    xjMean_p[1][jI]->GetYaxis()->SetTitleOffset(1.0);
    xjMean_p[1][jI]->GetXaxis()->SetTitleOffset(3.0);
      
    xjMean_p[1][jI]->DrawCopy("HIST E1 P");

    xjMean_p[1][jI]->Print("ALL");
    std::cout << "bad bins: ";
    for(unsigned int bI = 0; bI < binTracker.size(); ++bI){
      std::cout << binTracker[bI] << ", ";
    }
    std::cout << std::endl;
    
    doRevertDivideZeros(xjMean_p[1][jI], &binTracker);
    std::string saveName = "pdfDir/" + dateStr + "/meanXJ_" + jtPtBinsStr[jI] + "_" + globalStr + "_" + dateStr + ".pdf";
    quietSaveAs(canv_p, saveName);

    delete pads_p[1];
    delete pads_p[0];      
    delete canv_p;

    canv_p = new TCanvas("canv_p", "", 450, 450);
    canv_p->SetTopMargin(0.01);
    canv_p->SetRightMargin(0.01);
    canv_p->SetLeftMargin(0.01);
    canv_p->SetBottomMargin(0.01);

    pads_p[0] = new TPad("pad0", "", 0.0, padSplit, 1.0, 1.0);
    pads_p[1] = new TPad("pad1", "", 0.0, 0.0, 1.0, padSplit);
    
    pads_p[0]->SetTopMargin(0.02);
    pads_p[0]->SetLeftMargin(0.14);
    pads_p[0]->SetBottomMargin(0.001);
    pads_p[0]->SetRightMargin(0.02);
    
    pads_p[1]->SetTopMargin(0.02);
    pads_p[1]->SetLeftMargin(0.14);
    pads_p[1]->SetBottomMargin(0.14/padSplit);
    pads_p[1]->SetRightMargin(0.02);
    
    canv_p->cd();
    pads_p[0]->Draw("SAME");
    
    canv_p->cd();
    pads_p[1]->Draw("SAME");
    
    canv_p->cd();
    pads_p[0]->cd();
    
    xjSigma_p[0][jI]->SetMarkerSize(1);
    xjSigma_p[0][jI]->SetMarkerStyle(24);
    xjSigma_p[0][jI]->SetMarkerColor(1);
    xjSigma_p[0][jI]->SetLineColor(1);
    
    xjSigma_p[1][jI]->SetMarkerSize(1);
    xjSigma_p[1][jI]->SetMarkerStyle(25);
    xjSigma_p[1][jI]->SetMarkerColor(kRed);
    xjSigma_p[1][jI]->SetLineColor(kRed); 

    xjSigma_p[1][jI]->GetXaxis()->SetTitleFont(43);
    xjSigma_p[1][jI]->GetYaxis()->SetTitleFont(43);
    xjSigma_p[1][jI]->GetXaxis()->SetTitleSize(18);
    xjSigma_p[1][jI]->GetYaxis()->SetTitleSize(18);

    xjSigma_p[1][jI]->GetXaxis()->SetLabelFont(43);
    xjSigma_p[1][jI]->GetYaxis()->SetLabelFont(43);
    xjSigma_p[1][jI]->GetXaxis()->SetLabelSize(16);
    xjSigma_p[1][jI]->GetYaxis()->SetLabelSize(16);

    xjSigma_p[0][jI]->GetXaxis()->SetTitleFont(43);
    xjSigma_p[0][jI]->GetYaxis()->SetTitleFont(43);
    xjSigma_p[0][jI]->GetXaxis()->SetTitleSize(18);
    xjSigma_p[0][jI]->GetYaxis()->SetTitleSize(18);

    xjSigma_p[0][jI]->GetXaxis()->SetLabelFont(43);
    xjSigma_p[0][jI]->GetYaxis()->SetLabelFont(43);
    xjSigma_p[0][jI]->GetXaxis()->SetLabelSize(16);
    xjSigma_p[0][jI]->GetYaxis()->SetLabelSize(16);


    max = TMath::Max(getMax(xjSigma_p[0][jI]), getMax(xjSigma_p[1][jI]));
    min = TMath::Min(getMinGTZero(xjSigma_p[0][jI]), getMinGTZero(xjSigma_p[1][jI]));
    
    delta = max - min;
    max += 0.15*delta;
    min -= 0.15*delta;
    
    max = 0.4;
    min = 0.0;
    
    xjSigma_p[0][jI]->SetMaximum(max);
    xjSigma_p[0][jI]->SetMinimum(min);
    
    xjSigma_p[0][jI]->DrawCopy("HIST E1 P");
    xjSigma_p[1][jI]->DrawCopy("SAME HIST E1 P");    

    if(globalStr.size() != 0) label_p->DrawLatex(0.2, 0.92, globalStr2.c_str());
    label_p->DrawLatex(0.2, 0.84, (prettyString(jtPtBins[jI], 1, false) + " < p_{T} < " + prettyString(jtPtBins[jI+1], 1, false)).c_str());

    leg_p->Draw("SAME");

    canv_p->cd();
    pads_p[1]->cd();
    
    doDivideWithZeros(xjSigma_p[1][jI], xjSigma_p[0][jI], &binTracker);
    xjSigma_p[1][jI]->GetYaxis()->SetTitle((file2LegStr + "/" + file1LegStr).c_str());
    xjSigma_p[1][jI]->SetMaximum(1.15);
    xjSigma_p[1][jI]->SetMinimum(0.85);

    xjSigma_p[1][jI]->GetYaxis()->SetNdivisions(505);
    xjSigma_p[1][jI]->GetYaxis()->SetTitleOffset(1.0);
    xjSigma_p[1][jI]->GetXaxis()->SetTitleOffset(3.0);
      
    xjSigma_p[1][jI]->DrawCopy("HIST E1 P");
    xjSigma_p[1][jI]->Print("ALL");
    std::cout << "bad bins: ";
    for(unsigned int bI = 0; bI < binTracker.size(); ++bI){
      std::cout << binTracker[bI] << ", ";
    }
    std::cout << std::endl;

    doRevertDivideZeros(xjSigma_p[1][jI], &binTracker);
    saveName = "pdfDir/" + dateStr + "/sigmaXJ_" + jtPtBinsStr[jI] + "_" + globalStr + "_" + dateStr + ".pdf";
    quietSaveAs(canv_p, saveName);

    delete pads_p[1];
    delete pads_p[0];      
    delete canv_p;

    canv_p = new TCanvas("canv_p", "", 450, 450);
    canv_p->SetTopMargin(0.01);
    canv_p->SetRightMargin(0.01);
    canv_p->SetLeftMargin(0.01);
    canv_p->SetBottomMargin(0.01);

    pads_p[0] = new TPad("pad0", "", 0.0, padSplit, 1.0, 1.0);
    pads_p[1] = new TPad("pad1", "", 0.0, 0.0, 1.0, padSplit);
    
    pads_p[0]->SetTopMargin(0.02);
    pads_p[0]->SetLeftMargin(0.14);
    pads_p[0]->SetBottomMargin(0.001);
    pads_p[0]->SetRightMargin(0.02);
    
    pads_p[1]->SetTopMargin(0.02);
    pads_p[1]->SetLeftMargin(0.14);
    pads_p[1]->SetBottomMargin(0.14/padSplit);
    pads_p[1]->SetRightMargin(0.02);
    
    canv_p->cd();
    pads_p[0]->Draw("SAME");
    
    canv_p->cd();
    pads_p[1]->Draw("SAME");
    
    canv_p->cd();
    pads_p[0]->cd();
    
    xjSigmaOverMean_p[0][jI]->SetMarkerSize(1);
    xjSigmaOverMean_p[0][jI]->SetMarkerStyle(24);
    xjSigmaOverMean_p[0][jI]->SetMarkerColor(1);
    xjSigmaOverMean_p[0][jI]->SetLineColor(1);
    
    xjSigmaOverMean_p[1][jI]->SetMarkerSize(1);
    xjSigmaOverMean_p[1][jI]->SetMarkerStyle(25);
    xjSigmaOverMean_p[1][jI]->SetMarkerColor(kRed);
    xjSigmaOverMean_p[1][jI]->SetLineColor(kRed); 

    xjSigmaOverMean_p[1][jI]->GetXaxis()->SetTitleFont(43);
    xjSigmaOverMean_p[1][jI]->GetYaxis()->SetTitleFont(43);
    xjSigmaOverMean_p[1][jI]->GetXaxis()->SetTitleSize(18);
    xjSigmaOverMean_p[1][jI]->GetYaxis()->SetTitleSize(18);

    xjSigmaOverMean_p[1][jI]->GetXaxis()->SetLabelFont(43);
    xjSigmaOverMean_p[1][jI]->GetYaxis()->SetLabelFont(43);
    xjSigmaOverMean_p[1][jI]->GetXaxis()->SetLabelSize(16);
    xjSigmaOverMean_p[1][jI]->GetYaxis()->SetLabelSize(16);

    xjSigmaOverMean_p[0][jI]->GetXaxis()->SetTitleFont(43);
    xjSigmaOverMean_p[0][jI]->GetYaxis()->SetTitleFont(43);
    xjSigmaOverMean_p[0][jI]->GetXaxis()->SetTitleSize(18);
    xjSigmaOverMean_p[0][jI]->GetYaxis()->SetTitleSize(18);

    xjSigmaOverMean_p[0][jI]->GetXaxis()->SetLabelFont(43);
    xjSigmaOverMean_p[0][jI]->GetYaxis()->SetLabelFont(43);
    xjSigmaOverMean_p[0][jI]->GetXaxis()->SetLabelSize(16);
    xjSigmaOverMean_p[0][jI]->GetYaxis()->SetLabelSize(16);


    max = TMath::Max(getMax(xjSigmaOverMean_p[0][jI]), getMax(xjSigmaOverMean_p[1][jI]));
    min = TMath::Min(getMinGTZero(xjSigmaOverMean_p[0][jI]), getMinGTZero(xjSigmaOverMean_p[1][jI]));
    
    delta = max - min;
    max += 0.15*delta;
    min -= 0.15*delta;
    
    max = 0.4;
    min = 0.0;
    
    xjSigmaOverMean_p[0][jI]->SetMaximum(max);
    xjSigmaOverMean_p[0][jI]->SetMinimum(min);
    
    xjSigmaOverMean_p[0][jI]->DrawCopy("HIST E1 P");
    xjSigmaOverMean_p[1][jI]->DrawCopy("SAME HIST E1 P");    

    if(globalStr.size() != 0) label_p->DrawLatex(0.2, 0.92, globalStr2.c_str());
    label_p->DrawLatex(0.2, 0.84, (prettyString(jtPtBins[jI], 1, false) + " < p_{T} < " + prettyString(jtPtBins[jI+1], 1, false)).c_str());

    leg_p->Draw("SAME");

    canv_p->cd();
    pads_p[1]->cd();
    
    doDivideWithZeros(xjSigmaOverMean_p[1][jI], xjSigmaOverMean_p[0][jI], &binTracker);
    xjSigmaOverMean_p[1][jI]->GetYaxis()->SetTitle((file2LegStr + "/" + file1LegStr).c_str());
    xjSigmaOverMean_p[1][jI]->SetMaximum(1.15);
    xjSigmaOverMean_p[1][jI]->SetMinimum(0.85);

    xjSigmaOverMean_p[1][jI]->GetYaxis()->SetNdivisions(505);
    xjSigmaOverMean_p[1][jI]->GetYaxis()->SetTitleOffset(1.0);
    xjSigmaOverMean_p[1][jI]->GetXaxis()->SetTitleOffset(3.0);
      
    xjSigmaOverMean_p[1][jI]->DrawCopy("HIST E1 P");
    xjSigmaOverMean_p[1][jI]->Print("ALL");
    std::cout << "bad bins: ";
    for(unsigned int bI = 0; bI < binTracker.size(); ++bI){
      std::cout << binTracker[bI] << ", ";
    }
    std::cout << std::endl;

    doRevertDivideZeros(xjSigmaOverMean_p[1][jI], &binTracker);
    saveName = "pdfDir/" + dateStr + "/sigmaOverMeanXJ_" + jtPtBinsStr[jI] + "_" + globalStr + "_" + dateStr + ".pdf";
    quietSaveAs(canv_p, saveName);

    delete pads_p[1];
    delete pads_p[0];      
    delete canv_p;
  }

  delete label_p;
  delete leg_p;
  delete dummyHist_p;

  inFile2_p->Close();
  delete inFile2_p;
    
  inFile1_p->Close();
  delete inFile1_p;

  return 0;
}

int main(int argc, char* argv[])
{
  if(argc < 4 && argc > 6){
    std::cout << "Usage: ./bin/plotHistImb.exe <inFileName1> <inFileName2> <inFileName3> <globalStr> <leg1Str> <leg2Str> <leg3Str>. return 1" << std::endl;
    return 1;
  }
  
  int retVal = 0; 
  if(argc == 4) retVal += plotHistImb(argv[1], argv[2], argv[3]);
  else if(argc == 5) retVal += plotHistImb(argv[1], argv[2], argv[3], argv[4]);
  else if(argc == 6) retVal += plotHistImb(argv[1], argv[2], argv[3], argv[4], argv[5]);
  return retVal;
}
