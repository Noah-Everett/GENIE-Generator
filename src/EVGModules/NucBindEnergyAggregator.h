//____________________________________________________________________________
/*!

\class    genie::NucBindEnergyAggregator

\brief    A nuclear binding energy 'collector' which visits the event record,
          finds nucleons originating from within a nuclei and subtracts the
          binding energy they had in the nucleus.
          To record this action in the event record a hypothetical BINDINO is
          added to the event record.
          Is a concerete implementation of the EventRecordVisitorI interface.

\author   Costas Andreopoulos <costas.andreopoulos \at stfc.ac.uk>
          STFC, Rutherford Appleton Laboratory

\created  November 19, 2004

\cpright  Copyright (c) 2003-2009, GENIE Neutrino MC Generator Collaboration
          For the full text of the license visit http://copyright.genie-mc.org
          or see $GENIE/LICENSE
*/
//____________________________________________________________________________

#ifndef _NUCLEAR_BINDING_ENERGY_AGGREGATOR_H_
#define _NUCLEAR_BINDING_ENERGY_AGGREGATOR_H_

#include "EVGCore/EventRecordVisitorI.h"

namespace genie {

class GHepParticle;

class NucBindEnergyAggregator : public EventRecordVisitorI {

public :
  NucBindEnergyAggregator();
  NucBindEnergyAggregator(string config);
 ~NucBindEnergyAggregator();

  //-- implement the EventRecordVisitorI interface
  void ProcessEventRecord(GHepRecord * event_rec) const;

  //-- overload the Algorithm::Configure() methods to load private data
  //   members from configuration options
  void Configure(const Registry & config);
  void Configure(string config);

private:
  void LoadConfig (void);
  //GHepParticle * FindMotherNucleus(int ipos, GHepRecord * event_rec) const;

  bool fAllowRecombination; 
};

}      // genie namespace

#endif // _NUCLEAR_BINDING_ENERGY_AGGREGATOR_H_
