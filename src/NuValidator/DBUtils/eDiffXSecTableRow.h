//_____________________________________________________________________________
/*!

\class    genie::nuvld::eDiffXSecTableRow

\brief

\author   Costas Andreopoulos (Rutherford Lab.)  <C.V.Andreopoulos@rl.ac.uk>

\created  January 2004          
*/
//_____________________________________________________________________________

#ifndef _D_DIFF_XSEC_TABLE_ROW_H_
#define _D_DIFF_XSEC_TABLE_ROW_H_

#include <string>
#include <ostream>

#include <TSQLRow.h>

#include "DBUtils/DBTableRow.h"

using std::string;
using std::ostream;

namespace genie {
namespace nuvld {

class eDiffXSecTableRow : public DBTableRow
{
public:

  friend class DBI;

  friend ostream & operator << (ostream & stream, const eDiffXSecTableRow & row);
  
  string Experiment     (void) const;
  string MeasurementTag (void) const;
  string SigmaUnits     (void) const;
  string EUnits         (void) const;
  string EPUnits        (void) const;
  string ThetaUnits     (void) const;
  string Q2Units        (void) const;
  string W2Units        (void) const;
  string NuUnits        (void) const;
  double Sigma          (void) const;
  double dSigma         (void) const;
  double E              (void) const;
  double EP             (void) const;
  double Theta          (void) const;
  double Q2             (void) const;
  double W2             (void) const;
  double Nu             (void) const;
  double Epsilon        (void) const;
  double Gamma          (void) const;
  double x              (void) const;

private:

  eDiffXSecTableRow();
  eDiffXSecTableRow(const DBTableRow * db_row);
  eDiffXSecTableRow(TSQLRow * row);
  virtual ~eDiffXSecTableRow();

ClassDef(eDiffXSecTableRow, 1)
};

} // nuvld namespace
} // genie namespace

#endif
