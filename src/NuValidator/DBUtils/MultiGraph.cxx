//_____________________________________________________________________________
/*!

\class    genie::nuvld::MultiGraph

\brief

\author   Costas Andreopoulos (Rutherford Lab.)  <costas.andreopoulos \at stfc.ac.uk>

\created  January 2004                    
*/
//_____________________________________________________________________________

#include <iostream>

#include <TMath.h>

#include "DBUtils/MultiGraph.h"
#include "Messenger/Messenger.h"

using namespace genie::nuvld;

//______________________________________________________________________________
MultiGraph::MultiGraph()
{

}
//______________________________________________________________________________
MultiGraph::~MultiGraph()
{

}
//______________________________________________________________________________
void MultiGraph::AddGraph(string legend_entry, TGraphAsymmErrors * graph)
{
  _mgraph.push_back( graph );
  _legend_entries.push_back( legend_entry );  

  this->FormatGraph( _mgraph.size() - 1 );
}
//______________________________________________________________________________
unsigned int MultiGraph::NGraphs(void) const
{
  return _mgraph.size();
}
//______________________________________________________________________________
TGraphAsymmErrors * MultiGraph::GetGraph(unsigned int igraph) const
{
  if( igraph < this->NGraphs() ) return _mgraph[igraph];

  return 0;
}
//______________________________________________________________________________
string MultiGraph::GetLegendEntry(unsigned int igraph) const
{
  if( igraph < this->NGraphs() ) return _legend_entries[igraph];

  return "";
}
//______________________________________________________________________________
TLegend * MultiGraph::GetLegend(const char * option) const
{
  TLegend * legend = new TLegend(0.6, 0.4, 0.9, 0.9);

  legend->SetFillColor(0);

  this->FillLegend(option, legend);  

  return legend;
}
//______________________________________________________________________________
void MultiGraph::FillLegend(const char * option, TLegend * legend) const
{
  for(unsigned int igraph = 0; igraph < this->NGraphs(); igraph++)
     legend->AddEntry(_mgraph[igraph], _legend_entries[igraph].c_str(), option);
}
//______________________________________________________________________________
void MultiGraph::FormatGraph(unsigned int igraph)
{  
  const int n_colors  = 10;
  const int colors[]  = { 1, 2, 4, 6, 7, 8, 9, 50, 38, 40  };
  const int markers[] = { 3, 4, 5, 8, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30 };

  int imarker = igraph / n_colors;
  int icolor  = igraph % n_colors;

  LOG("NuVld", pDEBUG) << "Formatting graph = " << igraph
        << " - color = " << colors[icolor] << ", marker = " << markers[imarker];
  
  _mgraph[igraph] -> SetMarkerSize  (1);
  _mgraph[igraph] -> SetMarkerColor (colors[icolor]);
  _mgraph[igraph] -> SetMarkerStyle (markers[imarker]);
  _mgraph[igraph] -> SetLineColor   (colors[icolor]);
  _mgraph[igraph] -> SetLineWidth   (2);  
  _mgraph[igraph] -> SetLineStyle   (1);  
}
//______________________________________________________________________________


