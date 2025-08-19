
//c++ headers
#include <string>
#include <iostream>
#include <boost/tokenizer.hpp>
#include <iomanip>
#include <sstream>

//ROOT headers
#include <TPythia8Decayer.h>
#include "TDatabasePDG.h"
#include "TLorentzVector.h"
#include "TClonesArray.h"
#include "TParticle.h"
#include "TMath.h"
#include "TFile.h"
#include "TTree.h"

//local headers
#include "TDecayPolarized.h"
#include "TGenPsi2S.h"

#include "EvtGen/EvtGen.hh"
#include "EvtGenBase/EvtRandomEngine.hh"
#include "EvtGenBase/EvtSimpleRandomEngine.hh"
#include "EvtGenBase/EvtParticleFactory.hh"
#include "EvtGenBase/EvtVector4R.hh"
#include "EvtGenBase/EvtParticle.hh"
#include "EvtGenBase/EvtPDL.hh"

using namespace std;
using namespace boost;

//_____________________________________________________________________________
TGenPsi2S::TGenPsi2S(const string& inp, const string& outp, int nev): fNevt(nev), fNtx(1),
  fUseEta(false), fEtaMin(0), fEtaMax(0), jGenPt(0), jGenPt2(0),
  jGenY(0), jGenPhi(0) {

  fInp.open(inp);

  // Initialize PDG table (EvtPDL)
  // Create an instance of EvtPDL
//  EvtPDL pdl;
  pdl.readPDT(std::string(getenv("HOME")) + "/Tools/evtGenBuild/evt.pdl");

  // Random number generator
  EvtSimpleRandomEngine* myRandom = new EvtSimpleRandomEngine();

  // Create the EvtGen instance
  EvtGen myGenerator("../evtgenDir/DECAY.DEC", "evt.pdl", myRandom);
  
  
  fDec = new TPythia8Decayer();
  fDec->Init();

  fPdgDat = TDatabasePDG::Instance();

  fPart = new TClonesArray("TParticle");

  fPol = new TDecayPolarized(13, 1.);

  fTxOut.open(Form("%s.tx", outp.c_str()));
  fEvtline = "EVENT: ";
  fVtxline = "VERTEX: 0 0 0 0 1 0 0 ";

  fRootOut = new TFile(Form("%s.root", outp.c_str()), "recreate");
  jGenTree = new TTree("jGenTree", "jGenTree");
  jGenTree ->Branch("jGenPt", &jGenPt, "jGenPt/D");
  jGenTree ->Branch("jGenPt2", &jGenPt2, "jGenPt2/D");
  jGenTree ->Branch("jGenY", &jGenY, "jGenY/D");
  jGenTree ->Branch("jGenPhi", &jGenPhi, "jGenPhi/D");

}//TGenPsi2S

//_____________________________________________________________________________
TGenPsi2S::~TGenPsi2S() {

  jGenTree->Write();
  fRootOut->Close();
  delete fRootOut;

  fTxOut.close();
  fInp.close();

  delete fDec;

  fPart->Clear();
  delete fPart;

  delete fPol;

};//~TGenPsi2S

//_____________________________________________________________________________
void TGenPsi2S::SetEtaRange(double etamin, double etamax) {

  //set pseudorapidity interval for e+ and e- from J/psi decay

  fUseEta = true;

  fEtaMin = etamin;
  fEtaMax = etamax;

}//SetEtaRange

//_____________________________________________________________________________
void TGenPsi2S::EventLoop() {

  unsigned long iev = 0;
  unsigned long nprint = 5e5;
  unsigned long nreject = 0;
  int pdgId = 100443; // PDG for ψ(2S)
//EvtId evtId = pdl.getId(pdgId);
  EvtId evtId = EvtPDL::getId("psi(2S)");
  cout << "fNevt: " << fNevt << endl;
  //input event loop
  while(true) {

    if(iev > fNevt and fNevt != 0) break;

    //original psi(2S) event
    TLorentzVector vgen;
    if( !LoadInputEvent(vgen) ) break;

    //reject broken input events
    if( vgen.M() < 0.1 ) {

      cout << "TGenPsi2S: rejecting input event: ";
      cout << iev+1 << " ";
      cout << vgen.Pt() << " " << vgen.Rapidity() << " ";
      cout << vgen.M() << endl;

      ++nreject;

      continue;
    }

    //decay the psi(2S)
    EvtVector4R psi2S(vgen.E(), vgen.Px(), vgen.Py(), vgen.Pz());
    EvtParticle* parent = EvtParticleFactory::particleFactory(evtId, psi2S);
    myGenerator->generateDecay(parent);

    //psi(2S) has three daughters, the one at indice 0 is a jpsi, and the ones at indices 1 and 2 are pions
    EvtParticle* jpsi = parent->getDaug(0);
    EvtParticle* pion1 = parent->getDaug(1);
    EvtParticle* pion2 = parent->getDaug(2);

    // need to decay the jpsi:
    myGenerator->generateDecay(jpsi);
    std::cout << "Daughters number: " << jpsi->getNDaug() << endl;

    // now we can get the two muon daughters:
    EvtParticle* muon1 = jpsi->getDaug(0);
    EvtParticle* muon2 = jpsi->getDaug(1);
    
    //write the output in .tx format
    WriteStarlight(muon1, muon2, pion1, pion2);

    //write J/psi kinematics in output tree
    //jGenTree->Fill();

    iev++;

    //if (iev != 0 and iev%nprint == 0) {
    //  cout << "processed " << iev << " events" << endl;
    //}

  }//input event loop

  cout << "Rejected input events: " << nreject << endl;
  cout << "Events written: " << iev << endl;

}//EventLoop

//_____________________________________________________________________________
bool TGenPsi2S::PolarizedJpsi() {

  //polarized J/psi -> e+e- decay

  int idx;
  for(int i=0; i<fPart->GetEntries(); i++) {
    TParticle *part = dynamic_cast<TParticle*>( fPart->At(i) );

    if( part->GetPdgCode() == 443 ) {
      idx = i;
      break;
    }
  }

  //original J/psi Lorentz vector and removal from decay clones array
  TParticle *pjpsi = dynamic_cast<TParticle*>( fPart->At(idx) );
  TLorentzVector vjpsi;
  pjpsi->Momentum(vjpsi);
  fPart->RemoveAt(idx);
  fPart->Compress();

  //J/psi kinematics in output tree
  jGenPt = vjpsi.Pt();
  jGenPt2 = jGenPt*jGenPt;
  jGenY = vjpsi.Rapidity();
  jGenPhi = vjpsi.Phi();

  //generate the decay
  fPol->Generate(vjpsi);

  //store all psi(2S) decay products including polarized J/psi
  fVecPol.clear();
  fVecPol.resize( fPart->GetEntries() + 2 );

  fVecPol[0] = fPol->GetDecay(0);
  fVecPol[1] = fPol->GetDecay(1);

  for(int i=0; i<fPart->GetEntries(); i++) {
    fVecPol[i+2] = dynamic_cast<TParticle*>( fPart->At(i) );
  }

  if( !fUseEta ) return true;

  //evaluate the requested pseudorapidity interval

//double eta0 = fPol->GetDecay(0)->Eta();
 // double eta1 = fPol->GetDecay(1)->Eta();

  //if( eta0 < fEtaMin or eta0 > fEtaMax ) return false;
  //if( eta1 < fEtaMin or eta1 > fEtaMax ) return false;

  //decay passed the pseudorapidity interval

  return true;

}//PolarizedJpsi

//_____________________________________________________________________________
void TGenPsi2S::KeepFinalOnly() {

  //keep only J/psi and final products of psi(2S) decay, also dileptons from J/psi
  //decay are removed since polarized decay will follow

  vector<int> to_remove;
  to_remove.reserve(fPart->GetEntries());

  for(int i=0; i<fPart->GetEntries(); i++) {
    TParticle *part = dynamic_cast<TParticle*>( fPart->At(i) );

    if( part->GetPdgCode() == 443 ) {
      to_remove.push_back( part->GetFirstDaughter() );
      to_remove.push_back( part->GetLastDaughter() );
      continue;
    }

    if( part->GetNDaughters() > 0 ) to_remove.push_back(i);

    if (part -> GetPdgCode() == 22 || part -> GetPdgCode() == 11 || part -> GetPdgCode() == -11) to_remove.push_back(i);
  }

  for(vector<int>::const_iterator it = to_remove.cbegin(); it != to_remove.cend(); ++it) {
    fPart->RemoveAt(*it);
  }

  fPart->Compress();

}//KeepFinalOnly

//_____________________________________________________________________________
bool TGenPsi2S::AcceptDecay() {

  //select J/psi dilepton decays

  int idx0=0, idx1=0;
  for(int i=0; i<fPart->GetEntries(); i++) {
    TParticle *part = dynamic_cast<TParticle*>( fPart->At(i) );

    if( part->GetPdgCode() == 443 ) {
      idx0 = part->GetFirstDaughter();
      idx1 = part->GetLastDaughter();
      break;
    }
  }

  if( idx0 <= 0 or idx1 <= 0 or TMath::Abs(idx1-idx0) != 1 ) return false;

  if( !AcceptParticle(idx0) or !AcceptParticle(idx1) ) return false;

  return true;

}//AcceptDecay

//_____________________________________________________________________________
bool TGenPsi2S::AcceptParticle(int idx) {

  TParticle *part = dynamic_cast<TParticle*>( fPart->At(idx) );

  int pdg = part->GetPdgCode();

  if(TMath::Abs(pdg) != 13 ) return false;

  return true;

}//AcceptParticle

//_____________________________________________________________________________
bool TGenPsi2S::LoadInputEvent(TLorentzVector& vgen) {

  string line;

  //event and vertex lines
  getline(fInp, line);
  if( !fInp.good() ) return false;
  getline(fInp, line);

  //particle lines
  TLorentzVector v0, v1;
  getline(fInp, line);
  LoadParticle(v0, line);

  getline(fInp, line);
  LoadParticle(v1, line);

  vgen = v0 + v1;

  return true;

}//LoadInputEvent

//_____________________________________________________________________________
void TGenPsi2S::LoadParticle(TLorentzVector& pvec, const std::string& line) {

  //cout << line << endl;

  char_separator<char> sep(" ");
  tokenizer<char_separator<char>> tok(line, sep);
  tokenizer<char_separator<char>>::iterator trk_it=tok.begin();

  //particle momentum
  ++trk_it; ++trk_it;
  Double_t pxyz[3];
  for(int i=0; i<3; i++) pxyz[i] = stod( *(trk_it++) );

  //particle pdg
  for(int i=0; i<3; i++) ++trk_it;
  int pdg = stoi( *trk_it );

  //set particle Lorentz vector
  pvec.SetXYZM(pxyz[0], pxyz[1], pxyz[2], fPdgDat->GetParticle(pdg)->Mass());

}//LoadParticle

//_____________________________________________________________________________
void TGenPsi2S::WriteStarlight(EvtParticle* muon1, EvtParticle* muon2, EvtParticle* pion1, EvtParticle* pion2) {

  //write output in Starlight .tx format

  std::ostringstream tx;
  // the first line is: EVENT: fNtx(the current index) NTracks(4) NVertices(1)
  tx << fEvtline << fNtx << " 4 1" << endl;
  tx << fVtxline << " 4 " << endl;
  
  // Now we need to write four lines for each of the products:
  PutTxTrack(tx, 0, muon1);
  PutTxTrack(tx, 1, muon2);
  PutTxTrack(tx, 2, pion1);
  PutTxTrack(tx, 3, pion2);
  
  fTxOut << tx.str();

  ++fNtx;

}//WriteStarlight

//_____________________________________________________________________________
void TGenPsi2S::PutTxTrack(ostringstream &tx, unsigned int ipart, EvtParticle* dau) {
  //utility function for Starlight .tx format

  EvtVector4R pDau = dau->getP4Lab();
  int dauPdg = EvtPDL::getStdHep(dau->getId());

  tx << "TRACK:  " << PdgToGeant3(dauPdg) << " ";
  tx << fixed << pDau.get(1) << " ";
  tx << fixed << pDau.get(2) << " ";
  tx << fixed << pDau.get(3) << " ";
  tx << fNtx << " " << ipart << " 0 ";
  tx << dauPdg << endl;

}//put_tx_track

int TGenPsi2S::PdgToGeant3(int pdg) {
    switch (pdg) {
        case 211: return 8;    // pi+
        case -211: return 9;   // pi-
        case 13: return 6;     // mu-
        case -13: return 5;    // mu+
        default: return 0;     // unknown
    }
}


















