#ifndef HISTDEFUTILITY_H
#define HISTDEFUTILITY_H

#include <vector>

#include "TH1.h"

void centerTitles(TH1* hist_p)
{
  hist_p->GetXaxis()->CenterTitle();
  hist_p->GetYaxis()->CenterTitle();
  
  return;
}

void centerTitles(std::vector<TH1*> hists_)
{
  for(unsigned int pI = 0; pI < hists_.size(); ++pI){
    centerTitles(hists_.at(pI));
  }
  
  return;
}

void setSumW2(TH1* hist_p){hist_p->Sumw2(); return;}
void setSumW2(std::vector<TH1*> hists_)
{
  for(unsigned int pI = 0; pI < hists_.size(); ++pI){
    setSumW2(hists_.at(pI));
  }
  
  return;
}


Double_t getMinGTZero(TH1* inHist_p)
{
  Double_t min = inHist_p->GetMaximum();

  for(Int_t bIX = 0; bIX < inHist_p->GetNbinsX(); ++bIX){
    if(inHist_p->GetBinContent(bIX+1) <= 0) continue;
    if(inHist_p->GetBinContent(bIX+1) < min) min = inHist_p->GetBinContent(bIX+1);
  }

  return min;
}

Double_t getMax(TH1* inHist_p)
{
  Double_t max = inHist_p->GetBinContent(1);

  for(Int_t bIX = 0; bIX < inHist_p->GetNbinsX(); ++bIX){
    if(inHist_p->GetBinContent(bIX+1) > max) max = inHist_p->GetBinContent(bIX+1);
  }

  return max;
}


#endif
