#include <iostream>

void MyRootLogon() {
    gSystem->AddIncludePath("-I$HOME/Tools/pythia8303/include");
    gSystem->AddIncludePath("-I$HOME/Tools/root_source/montecarlo/pythia8/inc/TPythia8Decayer.h");
    std::cout << "Executing rootlogon.C" << std::endl;
}

// Call your setup function when ROOT starts
void rootlogon() {
    MyRootLogon();
}
