#ifndef TGenPsi2S_h
#define TGenPsi2S_h

#include "EvtGen/EvtGen.hh"
#include "EvtGenBase/EvtPDL.hh"
#include "EvtGenBase/EvtSimpleRandomEngine.hh"

class TPythia8Decayer;
class TDatabasePDG;
class TLorentzVector;
class TClonesArray;
class TDecayPolarized;
class TParticle;
class TFile;
class TTree;

#include <vector>
#include <fstream>
#include "Rtypes.h"

class TGenPsi2S {

public:

  // seed: EvtGen random seed (use the job seed so every job gets different decays)
  // polAxis: 0 = transverse w.r.t. the beam (lab z)  [default, s-channel helicity conservation]
  //          1 = transverse w.r.t. the psi(2S) lab momentum (helicity frame)
  TGenPsi2S(const std::string& inp, const std::string& outp, const std::string& decayFile,
            int nev, unsigned int seed = 1, int polAxis = 0);
  ~TGenPsi2S();

  void SetEtaRange(double etamin, double etamax);
  void EventLoop();

private:
  
  EvtPDL pdl;          // <-- make it a member
  EvtGen* myGenerator; // <-- make it a member pointer if you use dynamic allocation
  bool PolarizedJpsi();

  void KeepFinalOnly();

  bool AcceptDecay();
  bool AcceptParticle(int idx);

  bool LoadInputEvent(TLorentzVector& vgen);
  void LoadParticle(TLorentzVector& pvec, const std::string& line);

  void WriteStarlight4dau(EvtParticle* muon1, EvtParticle* muon2, EvtParticle* h1, EvtParticle* h2);
  void WriteStarlight3dau(EvtParticle* muon1, EvtParticle* muon2, EvtParticle* h1);
  void PutTxTrack(std::ostringstream &tx, unsigned int ipart, EvtParticle* dau);

  std::ifstream fInp; // input file
  unsigned long fNevt; // number of events to process

  std::ofstream fTxOut; //output in Starlight .tx format
  std::string fEvtline; // event line
  std::string fVtxline; // vertex line
  unsigned long fNtx; //output events to Starlight .tx format

  TDatabasePDG *fPdgDat; // PDG database
  TPythia8Decayer *fDec; // Pythia8 decayer
  TClonesArray *fPart; // clones array for decay products
  TDecayPolarized *fPol = nullptr; // polarized J/psi decayer (unused, must be initialised: it is deleted in the destructor)
  EvtRandomEngine *fRandom = nullptr; // EvtGen random engine
  int fPolAxis = 0; // see constructor

  std::vector<const TParticle*> fVecPol; // all psi(2S) decay products with J/psi polarized decay

  bool fUseEta; // flag to use eta interval
  double fEtaMin; // minimal pseudorapidity
  double fEtaMax; // maximal pseudorapidity

  TFile *fRootOut; // output ROOT file
  TTree *jGenTree; // output tree with J/psi kinematics
  Double_t jGenPt, jGenPt2; // pT and pT^2
  Double_t jGenY; // rapidity
  Double_t jGenPhi; // azimuthal angle

  int PdgToGeant3(int pdg);

};//TGenPsi2S

#endif














