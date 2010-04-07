#include "config.h"
#include "getStates.h"
#include "hpart.h"
//#include "CCXpair.h"
#include "output.h"
//#include "green.h"
#include "Hamiltonian.h"
#include "CCXpart.h"
#include "FieldOperators.h"
#include "BitClassification.h"

#include "iniconfig.h"


string input = "system.ini";

BitClassification Formula;
getStates S; 
output_handle OUT;
Hamiltonian H(Formula,S,OUT,input);


//(S,OUT,"system.ini",true,true);
//vector<getHpart> **Hpart;


//matrixs operatorsCiCXj(S,H,OUT);

IniConfig* pIni;

int main()
{	
	Formula.readin();
	cout << "=======================" << endl;
	cout << "System Info" << endl;
	cout << "=======================" << endl;
	Formula.printBitInfoList();
	cout << "=======================" << endl;
	cout << "Hopping Matrix" << endl;
	cout << "=======================" << endl;
	Formula.printHoppingMatrix();
	//determination of system
	
	pIni = new IniConfig(input);
	
	int N_bit = (*pIni)["system:N_bit"];
	int N_bit_m = (*pIni)["system:N_bit_m"];

	OUT = output_handle((*pIni)["output:path"]);

	// parameters of Green Function
	
	int i = (*pIni)["Green Function:i"];
	int j = (*pIni)["Green Function:j"];

	S.inigetStates(N_bit, N_bit_m);


	//end of determination	
	
	cout << "=======================" << endl;
	cout << "System is determined" << endl;
	cout << "=======================" << endl;
	cout << "=======================================" << endl;
	cout << "Process of creation and diagonalization" << endl;
	cout << "all parts of  Hamiltonian  has  started" << endl;
	cout << endl;

	//begining of creation all part of Hammiltonian

//	bool dump_ev_ef = Ini["output:dump_ev_ef"];

	H.enter(true,true);
	//finishing of creation
	
	cout << endl;
	cout << "All parts are created!" << endl;
	cout << endl;
	
/*	if (dump_ev_ef)
	{
		cout << "Eigenvectors are placed in " << OUT_EVec.fullpath() << endl;
		cout << "Eigenvalues are placed in " << OUT_EVal.fullpath() << endl;
	}
	cout << endl;*/

	//begining of creation matrixes C and CX
	
	cout << endl;
	cout << "==========================================" << endl;
	cout << "Beginning of rotation of matrices C and CX" << endl;
	CreationOperator CX(S,H,OUT,i);
	CX.prepare();
	CX.compute();
	CX.dump();
    
    // DEBUG
    RealType = (*pIni)["Green Function:beta"];
    DensityMatrix rho(H,beta);
    
//	CX.print_to_screen();


//	DestructionOperatorPart test(1,S,H.block(0,1,0),H.block(0,0,0),OUT);
//	test.enter();
	//test.print_to_screen();
	/*
	operatorsCiCXj.inimatrixs(i,j);
	
	operatorsCiCXj.putMatrXCX();				//creation matrix UXCXU

	if (i!=j)
	
{	
	operatorsCiCXj.putMatrXC();				//creation matrix UXCU 
	if ((bool) Ini["output:dump_c_cx"])
	{
		operatorsCiCXj.dump();
	
		cout << endl;
		cout << "Notrivial elements of rotated matrices" << endl;
		cout << "are placed in " << operatorsCiCXj.path() << endl;
	}
}	
	cout << endl;

	//finishing of creation

	//begining of creation Green Function G_ij(w)

	cout << endl;
	cout << "====================================" << endl;
	cout << "Start of Greens Function calculation" << endl;
	cout << endl;

	green GF_ij(i,j,S,H,operatorsCiCXj,OUT);
	
	if ((bool) Ini["output:dump_green"])
	{
		GF_ij.dump();
	
  		cout << endl;
		cout << "GF is placed in " << GF_ij.path() << endl;
		cout << endl;
	}

	//finishing of creation
	*/
	cout << endl << "All done." << endl;

	return 0;
}
