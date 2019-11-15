//cpp
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

//ROOT
#include "TDatime.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TTree.h"

//Local
#include "include/checkMakeDir.h"
#include "include/histDefUtility.h"
#include "include/paramPropagator.h"
#include "include/plotUtilities.h"

int makeHistRC(const std::string inFileName)
{
  if(!checkFile(inFileName) || inFileName.find(".root") == std::string::npos){
    std::cout << "Given inFileName \'" << inFileName << "\' is not valid. return 1" << std::endl;
    return 1;
  }

  TDatime* date = new TDatime();
  const std::string dateStr = std::to_string(date->GetDate());
  delete date;

  std::cout << "LINE: " << __LINE__ << std::endl;

  Float_t ptRC_, ptRhoRC_, ptRhoFlowRC_, ptSubRC_, ptSubFlowRC_, ptJet_, ptRhoJet_, ptRhoFlowJet_, ptRef_, ptRhoRef_, ptRhoFlowRef_;
  UInt_t etaPos_, etaPosJet_, etaPosRef_;
  Int_t centRC_, centJet_, centRef_, centPos_, centPosJet_, centPosRef_;

  TFile* inFile_p = new TFile(inFileName.c_str(), "READ");
  paramPropagator params;
  params.setupFromROOT(inFile_p);
  const Int_t nCentBinsMax = 10;
  const Int_t nEtaBinsMax = 10;

  std::map<std::string, std::string> paramFound = params.getParamFound();
  const bool doJet = paramFound["IMBINPUT"].size() != 0;
  const bool doRef = params.getImbRefPtStr().size() != 0;
  
  if(!params.checkNEtaBinsMax(nEtaBinsMax)) return 1;
  if(!params.checkNCentBinsMax(nCentBinsMax)) return 1;

  Int_t nCentBins = params.getNCentBins();
  Int_t nEtaBins = params.getNEtaBins();

  const Int_t nPtBins = 3;
  const Double_t ptBinsLow[nPtBins] = {50., 100., 200.};
  const Double_t ptBinsHigh[nPtBins] = {100., 200., 500.};

  std::cout << "LINE: " << __LINE__ << std::endl;

  TTree* rcTree_p = (TTree*)inFile_p->Get("rcTree");
  Double_t rcPtMin = rcTree_p->GetMinimum("ptRC");
  Double_t rcPtMax = rcTree_p->GetMaximum("ptRC");
  Double_t rhoPtMin = rcTree_p->GetMinimum("ptRhoRC");
  Double_t rhoPtMax = rcTree_p->GetMaximum("ptRhoRC");
  Double_t rhoFlowPtMin = rcTree_p->GetMinimum("ptRhoFlowRC");
  Double_t rhoFlowPtMax = rcTree_p->GetMaximum("ptRhoFlowRC");
  Double_t rcMinRhoPtMin = rcTree_p->GetMinimum("ptSubRC");
  Double_t rcMinRhoPtMax = rcTree_p->GetMaximum("ptSubRC");
  Double_t rcMinRhoFlowPtMin = rcTree_p->GetMinimum("ptSubFlowRC");
  Double_t rcMinRhoFlowPtMax = rcTree_p->GetMaximum("ptSubFlowRC");

  Double_t jetRhoPtMin;
  Double_t jetRhoPtMax;
  Double_t jetRhoFlowPtMin;
  Double_t jetRhoFlowPtMax;

  Double_t refRhoPtMin;
  Double_t refRhoPtMax;
  Double_t refRhoFlowPtMin;
  Double_t refRhoFlowPtMax;

  std::cout << "LINE: " << __LINE__ << std::endl;

  TTree* jetTree_p = NULL;
  TTree* genJetTree_p = NULL;
  if(doJet){
    jetTree_p = (TTree*)inFile_p->Get("jetTree");

    jetRhoPtMin = jetTree_p->GetMinimum("ptRhoJet");
    jetRhoPtMax = jetTree_p->GetMaximum("ptRhoJet");
    jetRhoFlowPtMin = jetTree_p->GetMinimum("ptRhoFlowJet");
    jetRhoFlowPtMax = jetTree_p->GetMaximum("ptRhoFlowJet");
    
    if(doRef){
      genJetTree_p = (TTree*)inFile_p->Get("genJetTree");
      
      refRhoPtMin = genJetTree_p->GetMinimum("ptRhoRef");
      refRhoPtMax = genJetTree_p->GetMaximum("ptRhoRef");
      refRhoFlowPtMin = genJetTree_p->GetMinimum("ptRhoFlowRef");
      refRhoFlowPtMax = genJetTree_p->GetMaximum("ptRhoFlowRef");     
    }
  }

  std::cout << "LINE: " << __LINE__ << std::endl;
  
  std::vector<std::vector<Double_t> > rcPtVals, rhoPtVals, rhoFlowPtVals, rcMinRhoPtVals, rcMinRhoFlowPtVals, jetRhoPtVals, jetRhoFlowPtVals, refRhoPtVals, refRhoFlowPtVals;
  for(Int_t cI = 0; cI < nCentBins; ++cI){
    rcPtVals.push_back({});
    rhoPtVals.push_back({});
    rhoFlowPtVals.push_back({});
    rcMinRhoPtVals.push_back({});
    rcMinRhoFlowPtVals.push_back({});

    jetRhoPtVals.push_back({});
    jetRhoFlowPtVals.push_back({});

    refRhoPtVals.push_back({});
    refRhoFlowPtVals.push_back({});
  }

  std::cout << "LINE: " << __LINE__ << std::endl;

  rcTree_p->SetBranchStatus("*", 0);
  rcTree_p->SetBranchStatus("ptRC", 1);
  rcTree_p->SetBranchStatus("ptRhoRC", 1);
  rcTree_p->SetBranchStatus("ptRhoFlowRC", 1);
  rcTree_p->SetBranchStatus("ptSubRC", 1);
  rcTree_p->SetBranchStatus("ptSubFlowRC", 1);
  rcTree_p->SetBranchStatus("centPos", 1);

  rcTree_p->SetBranchAddress("ptRC", &ptRC_);
  rcTree_p->SetBranchAddress("ptRhoRC", &ptRhoRC_);
  rcTree_p->SetBranchAddress("ptRhoFlowRC", &ptRhoFlowRC_);
  rcTree_p->SetBranchAddress("ptSubRC", &ptSubRC_);
  rcTree_p->SetBranchAddress("ptSubFlowRC", &ptSubFlowRC_);
  rcTree_p->SetBranchAddress("centPos", &centPos_);

  if(doJet){
    jetTree_p->SetBranchStatus("*", 0);
    jetTree_p->SetBranchStatus("ptJet", 1);
    jetTree_p->SetBranchStatus("ptRhoJet", 1);
    jetTree_p->SetBranchStatus("ptRhoFlowJet", 1);
    jetTree_p->SetBranchStatus("centPos", 1);

    jetTree_p->SetBranchAddress("ptJet", &ptJet_);
    jetTree_p->SetBranchAddress("ptRhoJet", &ptRhoJet_);
    jetTree_p->SetBranchAddress("ptRhoFlowJet", &ptRhoFlowJet_);
    jetTree_p->SetBranchAddress("centPos", &centPosJet_);

    if(doRef){
      genJetTree_p->SetBranchStatus("*", 0);
      genJetTree_p->SetBranchStatus("ptRef", 1);
      genJetTree_p->SetBranchStatus("ptRhoRef", 1);
      genJetTree_p->SetBranchStatus("ptRhoFlowRef", 1);
      genJetTree_p->SetBranchStatus("centPos", 1);
      
      genJetTree_p->SetBranchAddress("ptRef", &ptRef_);
      genJetTree_p->SetBranchAddress("ptRhoRef", &ptRhoRef_);
      genJetTree_p->SetBranchAddress("ptRhoFlowRef", &ptRhoFlowRef_);
      genJetTree_p->SetBranchAddress("centPos", &centPosRef_);
    }      
  }

  std::cout << "LINE: " << __LINE__ << std::endl;

  const Int_t nEntries = rcTree_p->GetEntries();
  const Int_t nDiv = TMath::Max(1, nEntries/20);

  std::cout << "Pre-Processing " << nEntries << "..." << std::endl;
  for(Int_t entry = 0; entry < nEntries; ++entry){
    rcTree_p->GetEntry(entry);

    rcPtVals[centPos_].push_back(ptRC_);
    rhoPtVals[centPos_].push_back(ptRhoRC_);
    rhoFlowPtVals[centPos_].push_back(ptRhoFlowRC_);
    rcMinRhoPtVals[centPos_].push_back(ptSubRC_);
    rcMinRhoFlowPtVals[centPos_].push_back(ptSubFlowRC_);
  }
  
  if(doJet){
    for(Int_t entry = 0; entry < jetTree_p->GetEntries(); ++entry){  
      jetRhoPtVals[centPos_].push_back(ptRhoJet_);
      jetRhoFlowPtVals[centPos_].push_back(ptRhoFlowJet_);
    }
    
    if(doRef){
      for(Int_t entry = 0; entry < genJetTree_p->GetEntries(); ++entry){
	refRhoPtVals[centPos_].push_back(ptRhoRef_);
	refRhoFlowPtVals[centPos_].push_back(ptRhoFlowRef_);
      }
    }
  }

  std::cout << "FILE, LINE: " << __FILE__ << ", " << __LINE__ << std::endl;

  for(Int_t cI = 0; cI < nCentBins; ++cI){
    std::vector<Double_t> tempVect = rcPtVals[cI];
    std::sort(std::begin(tempVect), std::end(tempVect));
    rcPtVals[cI] = tempVect;
    
    tempVect = rhoPtVals[cI];
    std::sort(std::begin(tempVect), std::end(tempVect));
    rhoPtVals[cI] = tempVect;

    tempVect = rhoFlowPtVals[cI];
    std::sort(std::begin(tempVect), std::end(tempVect));
    rhoFlowPtVals[cI] = tempVect;
    
    tempVect = rcMinRhoPtVals[cI];
    std::sort(std::begin(tempVect), std::end(tempVect));
    rcMinRhoPtVals[cI] = tempVect;

    tempVect = rcMinRhoFlowPtVals[cI];
    std::sort(std::begin(tempVect), std::end(tempVect));
    rcMinRhoFlowPtVals[cI] = tempVect;

    if(doJet){
      tempVect = jetRhoPtVals[cI];
      std::sort(std::begin(tempVect), std::end(tempVect));
      jetRhoPtVals[cI] = tempVect;
      
      tempVect = jetRhoFlowPtVals[cI];
      std::sort(std::begin(tempVect), std::end(tempVect));
      jetRhoFlowPtVals[cI] = tempVect;

      if(doRef){
	tempVect = refRhoPtVals[cI];
	std::sort(std::begin(tempVect), std::end(tempVect));
	refRhoPtVals[cI] = tempVect;
	
	tempVect = refRhoFlowPtVals[cI];
	std::sort(std::begin(tempVect), std::end(tempVect));
	refRhoFlowPtVals[cI] = tempVect;
      }
    }
  }

  std::cout << "FILE, LINE: " << __FILE__ << ", " << __LINE__ << std::endl;

  
  Double_t threeSigmaMax = -1;
  for(Int_t cI = 0; cI < nCentBins; ++cI){
    Int_t threeSigmaPos = 997*rcMinRhoPtVals[cI].size()/1000;

    if(rcMinRhoPtVals[cI][threeSigmaPos] > threeSigmaMax) threeSigmaMax = rcMinRhoPtVals[cI][threeSigmaPos];
  }


  //  rcPtMax = rcPtVals[threeSigmaPos];
  //  rhoPtMax = rhoPtVals[threeSigmaPos];
  ++rcPtMax;
  ++rhoPtMax;
  ++rhoFlowPtMax;
  rcMinRhoPtMax = TMath::Max(threeSigmaMax, TMath::Abs(rcMinRhoPtMin-1));
  rcPtMin = TMath::Max(0., rcPtMin-1);
  rhoPtMin = TMath::Max(0., rhoPtMin-1);
  rhoFlowPtMin = TMath::Max(0., rhoFlowPtMin-1);
  rcMinRhoPtMin -= 1;

  threeSigmaMax = -1;
  for(Int_t cI = 0; cI < nCentBins; ++cI){
    Int_t threeSigmaPos = 997*rcMinRhoFlowPtVals[cI].size()/1000;

    if(rcMinRhoFlowPtVals[cI][threeSigmaPos] > threeSigmaMax) threeSigmaMax = rcMinRhoFlowPtVals[cI][threeSigmaPos];
  }

  rcMinRhoFlowPtMax = TMath::Max(threeSigmaMax, TMath::Abs(rcMinRhoFlowPtMin-1));
  rcMinRhoFlowPtMin -= 1;



  if(doJet){
    ++jetRhoPtMax;
    ++jetRhoFlowPtMax;

    jetRhoPtMin = TMath::Max(0., jetRhoPtMin-1);
    jetRhoFlowPtMin = TMath::Max(0., jetRhoFlowPtMin-1);

    if(doRef){
      ++refRhoPtMax;
      ++refRhoFlowPtMax;
      
      refRhoPtMin = TMath::Max(0., refRhoPtMin-1);
      refRhoFlowPtMin = TMath::Max(0., refRhoFlowPtMin-1);
    }
  }
  
  std::cout << "FILE, LINE: " << __FILE__ << ", " << __LINE__ << std::endl;

  rcPtVals.clear();
  rhoPtVals.clear();
  rcMinRhoPtVals.clear();
  
  inFile_p->Close();
  delete inFile_p;

  std::vector<double> centBinsLow = params.getCentBinsLow();
  std::vector<double> centBinsHigh = params.getCentBinsHigh();

  std::vector<double> etaBinsLow = params.getEtaBinsLow();
  std::vector<double> etaBinsHigh = params.getEtaBinsHigh();

  std::string outFileName = inFileName;
  while(outFileName.find("/") != std::string::npos){outFileName.replace(0, outFileName.find("/")+1, "");}
  if(outFileName.find(".") != std::string::npos) outFileName = outFileName.substr(0, outFileName.rfind("."));
  outFileName = "output/" + dateStr + "/" + outFileName + "_HISTRC_" + dateStr + ".root";
  checkMakeDir("output");
  checkMakeDir("output/" + dateStr);
  TFile* outFile_p = new TFile(outFileName.c_str(), "RECREATE");
  TH1D* histRC_p[nCentBinsMax][nEtaBinsMax];
  TH1D* histRho_p[nCentBinsMax][nEtaBinsMax];
  TH1D* histRhoFlow_p[nCentBinsMax][nEtaBinsMax];
  TH1D* histRCMinRho_p[nCentBinsMax][nEtaBinsMax];
  TH1D* histRCMinRhoFlow_p[nCentBinsMax][nEtaBinsMax];

  TH1D* histJetRho_p[nCentBinsMax][nEtaBinsMax][nPtBins];
  TH1D* histJetRhoFlow_p[nCentBinsMax][nEtaBinsMax][nPtBins];

  TH1D* histRefRho_p[nCentBinsMax][nEtaBinsMax][nPtBins];
  TH1D* histRefRhoFlow_p[nCentBinsMax][nEtaBinsMax][nPtBins];

  
  TH2D* histRC_VCent_p[nEtaBinsMax];
  TH2D* histRho_VCent_p[nEtaBinsMax];
  TH2D* histRhoFlow_VCent_p[nEtaBinsMax];
  TH2D* histRCMinRho_VCent_p[nEtaBinsMax];
  TH2D* histRCMinRhoFlow_VCent_p[nEtaBinsMax];

  for(Int_t eI = 0; eI < nEtaBins; ++eI){
    std::string etaStr = "Eta" + prettyString(etaBinsLow[eI], 1, true) + "to" + prettyString(etaBinsHigh[eI], 1, true);

    histRC_VCent_p[eI] = new TH2D(("histRC_VCent_" + etaStr + "_h").c_str(), ";Centrality (%);Random Cone #Sigma p_{T}", 100, -0.5, 99.5, 100, rcPtMin, rcPtMax);
    histRho_VCent_p[eI] = new TH2D(("histRho_VCent_" + etaStr + "_h").c_str(), ";Centrality (%);Random Cone #rho#times A", 100, -0.5, 99.5, 100, rhoPtMin, rhoPtMax);
    histRhoFlow_VCent_p[eI] = new TH2D(("histRhoFlow_VCent_" + etaStr + "_h").c_str(), ";Centrality (%);Random Cone #rho#times A", 100, -0.5, 99.5, 100, rhoFlowPtMin, rhoFlowPtMax);
    histRCMinRho_VCent_p[eI] = new TH2D(("histRCMinRho_VCent_" + etaStr + "_h").c_str(), ";Centrality (%);Random Cone #Sigma p_{T} - #rho#times A", 100, -0.5, 99.5, 100, rcMinRhoPtMin, rcMinRhoPtMax);
    histRCMinRhoFlow_VCent_p[eI] = new TH2D(("histRCMinRhoFlow_VCent_" + etaStr + "_h").c_str(), ";Centrality (%);Random Cone #Sigma p_{T} - #rho#times A", 100, -0.5, 99.5, 100, rcMinRhoFlowPtMin, rcMinRhoFlowPtMax);
    centerTitles({histRC_VCent_p[eI], histRho_VCent_p[eI], histRCMinRho_VCent_p[eI], histRhoFlow_VCent_p[eI], histRCMinRhoFlow_VCent_p[eI]});

    for(Int_t cI = 0; cI < nCentBins; ++cI){
      std::string centStr = "Cent" + std::to_string(((int)centBinsLow[cI])) + "to" + std::to_string(((int)centBinsHigh[cI]));
      
      histRC_p[cI][eI] = new TH1D(("histRC_" + centStr + "_" + etaStr + "_h").c_str(), ";Random Cone #Sigma p_{T};Counts", 100, rcPtMin, rcPtMax);
      histRho_p[cI][eI] = new TH1D(("histRho_" + centStr + "_" + etaStr + "_h").c_str(), ";Random Cone #rho#times A;Counts", 100, rhoPtMin, rhoPtMax);
      histRhoFlow_p[cI][eI] = new TH1D(("histRhoFlow_" + centStr + "_" + etaStr + "_h").c_str(), ";Random Cone #rho#times A;Counts", 100, rhoFlowPtMin, rhoFlowPtMax);
      histRCMinRho_p[cI][eI] = new TH1D(("histRCMinRho_" + centStr + "_" + etaStr + "_h").c_str(), ";Random Cone #Sigma p_{T} - #rho#times A;Counts", 100, rcMinRhoPtMin, rcMinRhoPtMax);
      histRCMinRhoFlow_p[cI][eI] = new TH1D(("histRCMinRhoFlow_" + centStr + "_" + etaStr + "_h").c_str(), ";Random Cone #Sigma p_{T} - #rhoFlow#times A;Counts", 100, rcMinRhoFlowPtMin, rcMinRhoFlowPtMax);

      std::vector<TH1*> histVect = {histRC_p[cI][eI], histRho_p[cI][eI], histRCMinRho_p[cI][eI], histRhoFlow_p[cI][eI], histRCMinRhoFlow_p[cI][eI]};

      if(doJet){
	for(Int_t pI = 0; pI < nPtBins; ++pI){	 	
	  std::string ptStr = "Pt" + prettyString(ptBinsLow[pI], 1, true) + "to" + prettyString(ptBinsHigh[pI], 1, true);
	  
	  histJetRho_p[cI][eI][pI] = new TH1D(("histJetRho_" + centStr + "_" + etaStr + "_" + ptStr + "_h").c_str(), ";Random Cone #rho#times A;Counts", 100, jetRhoPtMin, jetRhoPtMax);
	  histJetRhoFlow_p[cI][eI][pI] = new TH1D(("histJetRhoFlow_" + centStr + "_" + etaStr + "_" + ptStr + "_h").c_str(), ";Random Cone #rho#times A;Counts", 100, jetRhoFlowPtMin, jetRhoFlowPtMax);	

	  histVect.push_back(histJetRho_p[cI][eI][pI]);
	  histVect.push_back(histJetRhoFlow_p[cI][eI][pI]);
	}
	
	if(doRef){
	  for(Int_t pI = 0; pI < nPtBins; ++pI){	 	
	    std::string ptStr = "Pt" + prettyString(ptBinsLow[pI], 1, true) + "to" + prettyString(ptBinsHigh[pI], 1, true);

	    histRefRho_p[cI][eI][pI] = new TH1D(("histRefRho_" + centStr + "_" + etaStr + "_" + ptStr + "_h").c_str(), ";Random Cone #rho#times A;Counts", 100, refRhoPtMin, refRhoPtMax);
	    histRefRhoFlow_p[cI][eI][pI] = new TH1D(("histRefRhoFlow_" + centStr + "_" + etaStr + "_" + ptStr + "_h").c_str(), ";Random Cone #rho#times A;Counts", 100, refRhoFlowPtMin, refRhoFlowPtMax);	
	    
	    histVect.push_back(histRefRho_p[cI][eI][pI]);
	    histVect.push_back(histRefRhoFlow_p[cI][eI][pI]);
	  }
	}
      }
      
      centerTitles(histVect);
    }
  }

  inFile_p = new TFile(inFileName.c_str(), "READ");
  rcTree_p = (TTree*)inFile_p->Get("rcTree");
  
  rcTree_p->SetBranchStatus("*", 0);
  rcTree_p->SetBranchStatus("centPos", 1);
  rcTree_p->SetBranchStatus("etaPos", 1);
  rcTree_p->SetBranchStatus("cent", 1);
  rcTree_p->SetBranchStatus("ptRC", 1);
  rcTree_p->SetBranchStatus("ptRhoRC", 1);
  rcTree_p->SetBranchStatus("ptRhoFlowRC", 1);
  rcTree_p->SetBranchStatus("ptSubRC", 1);
  rcTree_p->SetBranchStatus("ptSubFlowRC", 1);

  rcTree_p->SetBranchAddress("centPos", &centPos_);
  rcTree_p->SetBranchAddress("etaPos", &etaPos_);
  rcTree_p->SetBranchAddress("cent", &centRC_);
  rcTree_p->SetBranchAddress("ptRC", &ptRC_);
  rcTree_p->SetBranchAddress("ptRhoRC", &ptRhoRC_);
  rcTree_p->SetBranchAddress("ptRhoFlowRC", &ptRhoFlowRC_);
  rcTree_p->SetBranchAddress("ptSubRC", &ptSubRC_);
  rcTree_p->SetBranchAddress("ptSubFlowRC", &ptSubFlowRC_);

  if(doJet){
    jetTree_p = (TTree*)inFile_p->Get("jetTree");
  
    jetTree_p->SetBranchStatus("*", 0);
    jetTree_p->SetBranchStatus("ptJet", 1);
    jetTree_p->SetBranchStatus("centPos", 1);
    jetTree_p->SetBranchStatus("etaPos", 1);
    jetTree_p->SetBranchStatus("cent", 1);
    jetTree_p->SetBranchStatus("ptRhoJet", 1);
    jetTree_p->SetBranchStatus("ptRhoFlowJet", 1);
    
    jetTree_p->SetBranchAddress("ptJet", &ptJet_);
    jetTree_p->SetBranchAddress("centPos", &centPosJet_);
    jetTree_p->SetBranchAddress("etaPos", &etaPosJet_);
    jetTree_p->SetBranchAddress("cent", &centJet_);
    jetTree_p->SetBranchAddress("ptRhoJet", &ptRhoJet_);
    jetTree_p->SetBranchAddress("ptRhoFlowJet", &ptRhoFlowJet_);

    if(doRef){
      genJetTree_p = (TTree*)inFile_p->Get("genJetTree");
      
      genJetTree_p->SetBranchStatus("*", 0);
      genJetTree_p->SetBranchStatus("ptRef", 1);
      genJetTree_p->SetBranchStatus("centPos", 1);
      genJetTree_p->SetBranchStatus("etaPos", 1);
      genJetTree_p->SetBranchStatus("cent", 1);
      genJetTree_p->SetBranchStatus("ptRhoRef", 1);
      genJetTree_p->SetBranchStatus("ptRhoFlowRef", 1);
      
      genJetTree_p->SetBranchAddress("ptRef", &ptRef_);
      genJetTree_p->SetBranchAddress("centPos", &centPosRef_);
      genJetTree_p->SetBranchAddress("etaPos", &etaPosRef_);
      genJetTree_p->SetBranchAddress("cent", &centRef_);
      genJetTree_p->SetBranchAddress("ptRhoRef", &ptRhoRef_);
      genJetTree_p->SetBranchAddress("ptRhoFlowRef", &ptRhoFlowRef_);
    }
  }
  
  std::cout << "Processing " << nEntries << "..." << std::endl;
  for(Int_t entry = 0; entry < nEntries; ++entry){
    if(entry%nDiv == 0) std::cout << " Entry " << entry << "/" << nEntries << std::endl;
    rcTree_p->GetEntry(entry);

    histRC_p[centPos_][etaPos_]->Fill(ptRC_);
    histRho_p[centPos_][etaPos_]->Fill(ptRhoRC_);
    histRhoFlow_p[centPos_][etaPos_]->Fill(ptRhoFlowRC_);
    histRCMinRho_p[centPos_][etaPos_]->Fill(ptSubRC_);
    histRCMinRhoFlow_p[centPos_][etaPos_]->Fill(ptSubFlowRC_);

    histRC_VCent_p[etaPos_]->Fill(centRC_, ptRC_);
    histRho_VCent_p[etaPos_]->Fill(centRC_, ptRhoRC_);
    histRhoFlow_VCent_p[etaPos_]->Fill(centRC_, ptRhoFlowRC_);
    histRCMinRho_VCent_p[etaPos_]->Fill(centRC_, ptSubRC_);
    histRCMinRhoFlow_VCent_p[etaPos_]->Fill(centRC_, ptSubFlowRC_);
  }
  
  if(doJet){
    for(Int_t entry = 0; entry < jetTree_p->GetEntries(); ++entry){
      jetTree_p->GetEntry(entry);

      Int_t ptPos = -1;
      for(Int_t pI = 0; pI < nPtBins; ++pI){
	if(ptJet_ >= ptBinsLow[pI] && ptJet_ < ptBinsHigh[pI]){
	  ptPos = pI;
	  break;
	}
      }

      if(ptPos < 0) continue;
      
      histJetRho_p[centPosJet_][etaPosJet_][ptPos]->Fill(ptRhoJet_);
      histJetRhoFlow_p[centPosJet_][etaPosJet_][ptPos]->Fill(ptRhoFlowJet_);
    }
    
    if(doRef){
      for(Int_t entry = 0; entry < genJetTree_p->GetEntries(); ++entry){
	genJetTree_p->GetEntry(entry);

	Int_t ptPos = -1;
	for(Int_t pI = 0; pI < nPtBins; ++pI){
	  if(ptRef_ >= ptBinsLow[pI] && ptRef_ < ptBinsHigh[pI]){
	    ptPos = pI;
	    break;
	  }
	}
	
	if(ptPos < 0) continue;

	histRefRho_p[centPosRef_][etaPosRef_][ptPos]->Fill(ptRhoRef_);
	histRefRhoFlow_p[centPosRef_][etaPosRef_][ptPos]->Fill(ptRhoFlowRef_);
      }
    }
  }
  
  inFile_p->Close();
  delete inFile_p;
  
  outFile_p->cd();

  for(Int_t eI = 0; eI < nEtaBins; ++eI){
    histRC_VCent_p[eI]->Write("", TObject::kOverwrite);
    delete histRC_VCent_p[eI];
    
    histRho_VCent_p[eI]->Write("", TObject::kOverwrite);
    delete histRho_VCent_p[eI];

    histRhoFlow_VCent_p[eI]->Write("", TObject::kOverwrite);
    delete histRhoFlow_VCent_p[eI];

    histRCMinRho_VCent_p[eI]->Write("", TObject::kOverwrite);
    delete histRCMinRho_VCent_p[eI];

    histRCMinRhoFlow_VCent_p[eI]->Write("", TObject::kOverwrite);
    delete histRCMinRhoFlow_VCent_p[eI];

    for(Int_t cI = 0; cI < nCentBins; ++cI){
      histRC_p[cI][eI]->Write("", TObject::kOverwrite);
      delete histRC_p[cI][eI];

      histRho_p[cI][eI]->Write("", TObject::kOverwrite);
      delete histRho_p[cI][eI];

      histRhoFlow_p[cI][eI]->Write("", TObject::kOverwrite);
      delete histRhoFlow_p[cI][eI];

      histRCMinRho_p[cI][eI]->Write("", TObject::kOverwrite);
      delete histRCMinRho_p[cI][eI];

      histRCMinRhoFlow_p[cI][eI]->Write("", TObject::kOverwrite);
      delete histRCMinRhoFlow_p[cI][eI];

      if(doJet){
	for(Int_t pI = 0; pI < nPtBins; ++pI){
	  histJetRho_p[cI][eI][pI]->Write("", TObject::kOverwrite);
	  delete histJetRho_p[cI][eI][pI];
	  
	  histJetRhoFlow_p[cI][eI][pI]->Write("", TObject::kOverwrite);
	  delete histJetRhoFlow_p[cI][eI][pI];
	  
	  if(doRef){
	    histRefRho_p[cI][eI][pI]->Write("", TObject::kOverwrite);
	    delete histRefRho_p[cI][eI][pI];
	    
	    histRefRhoFlow_p[cI][eI][pI]->Write("", TObject::kOverwrite);
	    delete histRefRhoFlow_p[cI][eI][pI];
	  }
	}
      }
    }
  }

  TDirectoryFile* rcParamDir_p = (TDirectoryFile*)outFile_p->mkdir("rcParamDir");
  rcParamDir_p->cd();
  for(auto const & iter : paramFound){
    TNamed tempName(iter.first.c_str(), iter.second.c_str());
    tempName.Write("", TObject::kOverwrite);
  }
  
  outFile_p->Close();
  delete outFile_p;
  
  return 0;
}

int main(int argc, char* argv[])
{
  if(argc != 2){
    std::cout << "Usage: ./bin/makeHistRC.exe <inFileName>" << std::endl;
    return 1;
  }
  
  int retVal = 0;
  retVal += makeHistRC(argv[1]);
  return retVal;
}
