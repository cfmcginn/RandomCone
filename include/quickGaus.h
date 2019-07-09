#ifndef QUICKGAUS_H
#define QUICKGAUS_H

#include "TMath.h"

double calcQuickGaus(Double_t nSigma)
{
  const Double_t sig = 1;
  const Double_t mean = 0.;

  Double_t maxVal = sig*nSigma;
  Double_t minVal = -maxVal;
  Int_t nIntervals = 100000;
  Double_t space = (maxVal - minVal)/((Double_t)nIntervals);

  Double_t totalA = 0;
  
  for(Int_t bIX = 0; bIX < nIntervals; ++bIX){
    Double_t x1 = minVal + bIX*space;
    Double_t x2 = minVal + (bIX+1)*space;
    
    Double_t y1 = TMath::Gaus(x1, mean, sig, kTRUE);
    Double_t y2 = TMath::Gaus(x2, mean, sig, kTRUE);

    Double_t maxY = TMath::Max(y1, y2);
    Double_t minY = TMath::Min(y1, y2);

    Double_t area = (x2 - x1)*(minY) + (x2 - x1)*(maxY - minY)/2.;

    totalA += area;
  }
  
  return totalA;
}

#endif
