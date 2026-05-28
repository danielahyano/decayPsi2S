#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "TGenPsi2S.h"

using namespace std;

//_____________________________________________________________________________
int main(int argc, char* argv[]) {

	std::string inFile, outFile;
	string decayFile = "DECAYMU.DEC";
	if(argc==1){
		inFile = "/afs/cern.ch/user/d/dyano/private/STARlight/starlightTrunk_v313/build/slightPsi2sDiE.out";
		// inFile = "/afs/cern.ch/user/s/shuaiy/public/starlight/decayPsi2S/testFiles/slight_CohPsi2S_4Feeddown_0001.out";
		outFile = "test";
	}
	else if(argc==3){
		inFile  = std::string(argv[1]);
		outFile = std::string(argv[2]);
	}
	else if(argc==4){
   		 // Custom decay card: ./fdgen inFile outFile decayCard.DEC 0
    		inFile    = std::string(argv[1]);
    		outFile   = std::string(argv[2]);
    		decayFile = std::string(argv[3]);
	}
	else{
		cout<<"argc should be 1, 3, or 4 (with custom decay card)!"<<endl;
		return -1;
	}

	TGenPsi2S *gen = new TGenPsi2S(inFile, outFile, decayFile, 0);

	//gen->SetEtaRange(-2.5, 2.5);

	gen->EventLoop();

	delete gen;

	return 0;

}//main

