//_____________________________________________________________________________
/*!

\class    genie::nuvld::vDataSelectionDialog

\brief

\author   Costas Andreopoulos (Rutherford Lab.)  <C.V.Andreopoulos@rl.ac.uk>

\created  November 01, 2004
*/
//_____________________________________________________________________________

#ifndef _NEUTRINO_DATA_SELECTION_DIALOG_H_
#define _NEUTRINO_DATA_SELECTION_DIALOG_H_

#include <string>

#include <TApplication.h>
#include <TVirtualX.h>
#include <TGClient.h>
#include <TGFrame.h>
#include <TGListBox.h>
#include <TGButton.h>
#include <TGNumberEntry.h>
#include <TGLabel.h>
#include <RQ_OBJECT.h>

#include "NuVldGUI/DBConnection.h"
#include "NuVldGUI/DataSelectionDialog.h"

using std::string;

namespace genie {
namespace nuvld {

class vDataSelectionDialog : public DataSelectionDialog {

RQ_OBJECT("vDataSelectionDialog")

public:
   vDataSelectionDialog(const TGWindow *p, const TGWindow *main,
                           bool & attn, UInt_t w, UInt_t h,
                           UInt_t options = kVerticalFrame, DBConnection * db = 0);
   virtual ~vDataSelectionDialog();

   void CloseWindow (void) { delete this;               }
   void Close       (void) { _main->SendCloseMessage(); }

   //-- Events triggered on SelectionChanged() event so as to continuously
   //   restore consistency between selected entries amongst the various widgets

   void MakeConsistentWithExpListbox      (void);
   void MakeConsistentWithExpObsListboxes (void);
   
   void SelectAllExp     (void);
   void SelectAllXSec    (void);
   void SelectAllProbes  (void);
   void SelectAllTargets (void);

   //--- DataSelectionDialog interface

   string BundleKeyListInString (void);
   string BundleCutsInString    (void);
   string BundleDrawOptInString (void);
      
private:

   void BuildLeftFrameWidgets    (void);
   void BuildRightFrameWidgets   (void);

   void LoadExperimentsFromDB    (void);   
   void LoadXSecTypesFromDB      (void);
   void LoadProbesFromDB         (void);
   void LoadTargetsFromDB        (void);
   void LoadMeasurementsFromDB   (void);

   void SelectAllMeasurementsForExp    (string exp_name);
   void SelectAllObservablesForExp     (string exp_name);
   void SelectAllReactionsForExp       (string exp_name);
   void SelectAllTargetsForExp         (string exp_name);
   void SelectAllMeasurementsForExpObs (string exp_name, string observable);
   void SelectAllObservablesForExpObs  (string exp_name, string observable);
   void SelectAllReactionsForExpObs    (string exp_name, string observable);
   void SelectAllTargetsForExpObs      (string exp_name, string observable);
            
   void PositionRelativeToParent (const TGWindow * main);

   TGTransientFrame * _main;
   TGCompositeFrame * _main_left_frame;
   TGCompositeFrame * _main_right_frame;
   TGGroupFrame *     _xsec_err_group_frame;
   TGGroupFrame *     _exp_group_frame;
   TGGroupFrame *     _obs_group_frame;
   TGGroupFrame *     _energy_group_frame;
   TGGroupFrame *     _wcut_group_frame;
   TGGroupFrame *     _cuts_group_frame;
   TGGroupFrame *     _reaction_group_frame;
   TGGroupFrame *     _init_state_group_frame;
   TGGroupFrame *     _target_group_frame;
   TGButton *         _close_button;
   TGCheckButton *    _select_all_exp;
   TGCheckButton *    _select_all_xsec;
   TGCheckButton *    _select_all_nu;
   TGCheckButton *    _select_all_target;   
   TGCheckButton *    _scale_with_energy;
   TGMatrixLayout *   _energy_matrix_layout;
   TGLayoutHints *    _mleft_frame_layout;
   TGLayoutHints *    _mright_frame_layout;
   TGLayoutHints *    _listbox_layout;
   TGLayoutHints *    _button_layout;
   TGListBox *        _measurements_listbox;
   TGListBox *        _xsec_err_listbox;
   TGListBox *        _exp_listbox;
   TGListBox *        _xsec_listbox;
   TGListBox *        _nu_listbox;
   TGListBox *        _target_listbox;
   TGNumberEntry *    _W_cut;
   TGNumberEntry *    _E_min;
   TGNumberEntry *    _E_max;
   TGLabel *          _E_minLabel;
   TGLabel *          _E_maxLabel;
   
   DBConnection * _db;
   
   ClassDef(vDataSelectionDialog, 0)
};

} // nuvld namespace
} // genie namespace

#endif // _NEUTRINO_DATA_SELECTION_DIALOG_H_

