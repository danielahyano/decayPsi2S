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
	unsigned int seed = 1;
	int polAxis = 0; // 0 = beam, 1 = helicity
	if(argc==1){
		inFile = "slight.out";
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
	else if(argc==5 || argc==6){
		// ./fdgen inFile outFile decayCard.DEC seed [polAxis]
		inFile    = std::string(argv[1]);
		outFile   = std::string(argv[2]);
		decayFile = std::string(argv[3]);
		seed      = std::stoul(argv[4]);
		if (argc==6) polAxis = std::stoi(argv[5]);
	}
	else{
		cout<<"usage: fdgen [inFile outFile [decayCard [seed [polAxis]]]]"<<endl;
		return -1;
	}

	TGenPsi2S *gen = new TGenPsi2S(inFile, outFile, decayFile, 0, seed, polAxis);

	//gen->SetEtaRange(-2.5, 2.5);

	gen->EventLoop();

	delete gen;

	return 0;

}//main

