//____________________________________________________________________________
/*!

\class    genie::HadronizationModelI

\brief    Pure abstract base class.
          Defines the HadronizationModelI interface to be implemented by any
          algorithmic class performing hadronization.

\author   Costas Andreopoulos <C.V.Andreopoulos@rl.ac.uk>
          CCLRC, Rutherford Appleton Laboratory

\created  August 17, 2004

*/
//____________________________________________________________________________

#include "Fragmentation/HadronizationModelI.h"

using namespace genie;

//____________________________________________________________________________
HadronizationModelI::HadronizationModelI() :
Algorithm()
{

}
//____________________________________________________________________________
HadronizationModelI::HadronizationModelI(const char * param_set) :
Algorithm(param_set)
{

}
//____________________________________________________________________________
HadronizationModelI::~HadronizationModelI() 
{ 

}
//____________________________________________________________________________



