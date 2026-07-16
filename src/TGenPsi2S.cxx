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
#include "EvtGenBase/EvtVectorParticle.hh"
#include "EvtGen/EvtGen.hh"
#include "EvtGenBase/EvtRandomEngine.hh"
#include "EvtGenBase/EvtSimpleRandomEngine.hh"
#include "EvtGenBase/EvtParticleFactory.hh"
#include "EvtGenBase/EvtVector4R.hh"
#include "EvtGenBase/EvtParticle.hh"
#include "EvtGenBase/EvtPDL.hh"
#include "EvtGenBase/EvtSpinDensity.hh"
#include "EvtGenBase/EvtSpinType.hh"
#include "EvtGenBase/EvtVector4C.hh"
using namespace std;
using namespace boost;

#include <cmath>

EvtParticle* makeMomentumAlignedVectorParticle(
    EvtId id,
    const EvtVector4R& p4,
    const EvtSpinDensity& rho)
{
    double px = p4.get(1), py = p4.get(2), pz = p4.get(3);
    double pmag = std::sqrt(px*px + py*py + pz*pz);

    double zx, zy, zz;
    if (pmag > 1e-12) {
        zx = px / pmag;
        zy = py / pmag;
        zz = pz / pmag;
    } else {
        zx = 0.0; zy = 0.0; zz = 1.0;
    }

    double ax, ay, az;
    if (std::fabs(zx) < 0.9) { ax = 1.0; ay = 0.0; az = 0.0; }
    else                     { ax = 0.0; ay = 1.0; az = 0.0; }

    double adotz = ax*zx + ay*zy + az*zz;
    double xx = ax - adotz*zx;
    double xy = ay - adotz*zy;
    double xz = az - adotz*zz;
    double xmag = std::sqrt(xx*xx + xy*xy + xz*xz);
    xx /= xmag; xy /= xmag; xz /= xmag;

    double yx = zy*xz - zz*xy;
    double yy = zz*xx - zx*xz;
    double yz = zx*xy - zy*xx;

    EvtVector4C epsX(0.0, xx, xy, xz);
    EvtVector4C epsY(0.0, yx, yy, yz);
    EvtVector4C epsZ(0.0, zx, zy, zz);

    EvtVectorParticle* myPart = new EvtVectorParticle;
    myPart->init(id, p4, epsX, epsY, epsZ);
    myPart->setSpinDensityForward(rho);

    return myPart;
}

//_____________________________________________________________________________
TGenPsi2S::TGenPsi2S(const string& inp, const string& outp,const string& decayFile, int nev): fNevt(nev), fNtx(1),
  fUseEta(false), fEtaMin(0), fEtaMax(0), jGenPt(0), jGenPt2(0),
  jGenY(0), jGenPhi(0) {

  fInp.open(inp);

  // Initialize PDG table (EvtPDL)
  // Create an instance of EvtPDL
//  EvtPDL pdl;
 // pdl.readPDT(std::string(getenv("HOME")) + "/Tools/evtGenBuild/evt.pdl");
  pdl.readPDT(std::string("evt.pdl"));
  // Random number generator
  EvtSimpleRandomEngine* myRandom = new EvtSimpleRandomEngine();

  // Create the EvtGen instance
  myGenerator = new EvtGen(decayFile.c_str(), "", myRandom); 

  fPdgDat = TDatabasePDG::Instance();

  fPart = new TClonesArray("TParticle");

//fPol = new TDecayPolarized(13, 1.);

  fTxOut.open(Form("%s.tx", outp.c_str()));
  fEvtline = "EVENT: ";
  fVtxline = "VERTEX: 0 0 0 0 1 0 0 ";

  fRootOut = new TFile(Form("%s.root", outp.c_str()), "recreate");

}//TGenPsi2S

//_____________________________________________________________________________
TGenPsi2S::~TGenPsi2S() {

  fRootOut->Close();
  delete fRootOut;

  fTxOut.close();
  fInp.close();

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
  EvtId evtId = EvtPDL::getId("psi(2S)");
  std::string dummy;
  for(int i = 0; i < 3; ++i) {
      if(!std::getline(fInp, dummy)) {
          cout << "Error: Could not read STARlight header lines!" << endl;
          return;
      }
      cout << "Skipping header line " << i+1 << ": " << dummy << endl;
  }
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
    
    // Transverse polarization. NOTE on the basis: setSpinDensityForward()
    // interprets rho in the particle's own eps basis. With
    // makeMomentumAlignedVectorParticle the eps slots are CARTESIAN
    // directions (0 = x', 1 = y', 2 = z' = p-hat), NOT helicity states.
    // Transverse = m = +/-1 about z', and summing those projectors gives
    // 0.5|x'><x'| + 0.5|y'><y'| (imaginary cross terms cancel), with the
    // longitudinal z' slot EMPTY. The old diag(0.5, 0, 0.5) instead put
    // half the weight on z' (longitudinal!) -> a 50/50 transverse-linear +
    // longitudinal mix, giving lambda ~ -1/3 in the muon cosTheta* fit.
    EvtSpinDensity rho;
    rho.setDim(3);
    rho.set(0,0, 0.5);  // x' (transverse plane)
    rho.set(1,1, 0.5);  // y' (transverse plane)
    rho.set(2,2, 0.0);  // z' = p-hat (longitudinal, empty)
    EvtParticle* parent = makeMomentumAlignedVectorParticle(evtId, psi2S, rho);
    myGenerator->generateDecay(parent);
    EvtSpinDensity check = parent->getSpinDensityForward();
    //std::cout << "rho after set: " << check << std::endl;
    //psi(2S) has either three daughters, the one at indice 0 is a jpsi, and the ones at indices 1 and 2 are pions
    //psi(2S) might have two daughters, in case of psi2s to jpsi+eta
    int nd = parent->getNDaug();
    if (nd == 3) {
       // psi(2S) -> J/psi + h1 + h2 (e.g. pi+ pi-, pi0 pi0)
       EvtParticle* h1   = parent->getDaug(1);
       EvtParticle* h2   = parent->getDaug(2);
       EvtParticle* jpsi = parent->getDaug(0);

       // generateDecay(parent) decays recursively (the .dec defines
       // J/psi -> mu mu), so the J/psi normally arrives here already
       // decayed, with muons drawn against its inherited spin density.
       // Only decay it ourselves if the recursion did not.
       if (jpsi->getNDaug() == 0) myGenerator->generateDecay(jpsi);
       // now we can get the two muon daughters:
       EvtParticle* muon1 = jpsi->getDaug(0);
       EvtParticle* muon2 = jpsi->getDaug(1);
       WriteStarlight4dau(muon1, muon2, h1, h2);
    } else if (nd == 2) {
       // psi(2S) -> J/psi + X   (e.g. eta, pi0)
       EvtParticle* X    = parent->getDaug(1);
       EvtParticle* jpsi = parent->getDaug(0);
       if (jpsi->getNDaug() == 0) myGenerator->generateDecay(jpsi);
       // now we can get the two muon daughters:
       EvtParticle* muon1 = jpsi->getDaug(0);
       EvtParticle* muon2 = jpsi->getDaug(1);
       WriteStarlight3dau(muon1, muon2, X);
    } else {
       std::cerr << "Unexpected number of daughters for psi(2S): "
              << nd << std::endl;
    }

    iev++;

  }//input event loop

  cout << "Rejected input events: " << nreject << endl;
  cout << "Events written: " << iev << endl;

}//EventLoop

//_____________________________________________________________________________

//_____________________________________________________________________________

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
void TGenPsi2S::WriteStarlight4dau(EvtParticle* muon1, EvtParticle* muon2, EvtParticle* h1, EvtParticle* h2) {

  //write output in Starlight .tx format

  std::ostringstream tx;
  // the first line is: EVENT: fNtx(the current index) NTracks(4) NVertices(1)
  tx << fEvtline << fNtx << " 4 1" << endl;
  tx << fVtxline << " 4 " << endl;
  
  // Now we need to write four lines for each of the products:
  PutTxTrack(tx, 0, muon1);
  PutTxTrack(tx, 1, muon2);
  PutTxTrack(tx, 2, h1);
  PutTxTrack(tx, 3, h2);
  
  fTxOut << tx.str();

  ++fNtx;

}//WriteStarlight

//_____________________________________________________________________________
void TGenPsi2S::WriteStarlight3dau(EvtParticle* muon1, EvtParticle* muon2, EvtParticle* h1) {

  //write output in Starlight .tx format

  std::ostringstream tx;
  // the first line is: EVENT: fNtx(the current index) NTracks(4) NVertices(1)
  tx << fEvtline << fNtx << " 3 1" << endl;
  tx << fVtxline << " 3 " << endl;

  // Now we need to write four lines for each of the products:
  PutTxTrack(tx, 0, muon1);
  PutTxTrack(tx, 1, muon2);
  PutTxTrack(tx, 2, h1);

  fTxOut << tx.str();

  ++fNtx;

}//WriteStarlightETA

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
        case 11: return 3;     // e-
	case -11: return 2;    // e+
        default: return 0;     // unknown
    }
}
