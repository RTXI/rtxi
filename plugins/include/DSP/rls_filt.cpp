//
//  File = rls_filt.cpp
//

#include <stdlib.h>
#include <fstream>
#include "rls_filt.h"
#include "adap_fir.h"
#include "matrix_T.h"
#ifdef _DEBUG
  #include <fstream>
  //#define _RLS_DEBUG 1
  extern std::ofstream DebugFile;
#endif

RlsFilter::RlsFilter( int num_taps,
                      double *coeff,
                      double delta,
                      double lambda,
                      logical quan_enab,
                      long coeff_quan_factor,
                      long input_quan_factor,
                      int tap_for_trans,
                      int secondary_tap,
                      int transient_len)
          :AdaptiveFir( num_taps,
                        coeff,
                        quan_enab,
                        coeff_quan_factor,
                        input_quan_factor,
                        tap_for_trans,
                        secondary_tap,
                        transient_len)
{
 //double delta=0.01;
 Delta = delta;
 //double lambda=0.95;
 Lambda = lambda;
 #ifdef _DEBUG
 DebugFile << "In RlsFilter constructor" << std::endl;
 #endif

 Cv_Work = new colvec<double>(0,num_taps);
 U_Vec = new colvec<double>(0,num_taps);
 K_Vec = new colvec<double>(0,num_taps);
 P_Mtx = new matrix<double>(0,num_taps,0,num_taps);
 Rv_Work = new rowvec<double>(0,Num_Taps);
 #ifdef _DEBUG
 DebugFile << "End of RLS ctor" << std::endl;
 #endif
 Trial_Count = 0;
 //ResetTaps();
 return;
}

void RlsFilter::ResetTaps( void )
{
  int tap_idx;

 for(int i=0; i <Num_Taps; i++)
   {
   for(int j=0; j<Num_Taps; j++)
     {
     (*P_Mtx)[i][j] = 0.0;
     }
   }
for(tap_idx=0; tap_idx<Num_Taps; tap_idx++)
    {
    Unquan_Coeff[tap_idx] = 0.0;
    Unquan_In_Buf[tap_idx] = 0.0;
    //(*P_Mtx)[tap_idx][tap_idx] = Lambda;
    (*P_Mtx)[tap_idx][tap_idx] = 1.0/Delta;
    }
  Update_Count = 0;
  Trial_Count++;
}
double RlsFilter::UpdateTaps( double true_samp,
                              double estim_samp,
                              logical trans_save_enab )
{
  double err_samp;
  int row_idx, tap_idx;
  double denom;
  colvec<double> *u_vec, *cv_work, *k_vec;
  rowvec<double> *rv_work;
  int read_idx, read_idx_init;
  double *test_ptr;

  #ifdef _RLS_DEBUG
    DebugFile << "in UpdateTaps" << std::endl;
  #endif

  /*
  cv_work = Cv_Work;
  k_vec = K_Vec;
  u_vec = U_Vec;
  rv_work = Rv_Work;
  */
 cv_work = new colvec<double>(0,Num_Taps);
 k_vec = new colvec<double>(0,Num_Taps);
 u_vec = new colvec<double>(0,Num_Taps);
 rv_work = new rowvec<double>(0,Num_Taps);
  err_samp = true_samp - estim_samp;
  read_idx_init = Write_Indx - 1;
  if(read_idx_init < 0) read_idx_init = Num_Taps - 1;
  //----------------------------------------------------
  //  copy circular input buffer into a regular vector
  read_idx = read_idx_init;
  for(row_idx=0; row_idx<Num_Taps; row_idx++)
    {
    (*u_vec)[row_idx] = Unquan_In_Buf[read_idx];
    read_idx--;
    if(read_idx < 0) read_idx = Num_Taps - 1;
    }
  //------------------------------------------------
  // perform update calculations
  #ifdef _RLS_DEBUG
    DebugFile << "\n------------------------------" << std::endl;
    DebugFile << "*cv_work = (*P_Mtx) * (*u_vec)" << std::endl;
  #endif
  *cv_work = (*P_Mtx) * (*u_vec);

  #ifdef _RLS_DEBUG
    DebugFile << "\n-----------------------------------------" << std::endl;
    DebugFile << "denom = Lambda + (!(*u_vec) * (*cv_work))" << std::endl;
  #endif
  denom = Lambda + (!(*u_vec) * (*cv_work));

  #ifdef _RLS_DEBUG
    DebugFile << "\n-------------------------" << std::endl;
    DebugFile << "*k_vec = (*cv_work)/denom" << std::endl;
  #endif
  *k_vec = (*cv_work)/denom;

  #ifdef _RLS_DEBUG
    DebugFile << "\n---------------------------------" << std::endl;
    DebugFile << "*rv_work = (!(*u_vec) * (*P_Mtx))" << std::endl;
  #endif
  *rv_work = (!(*u_vec) * (*P_Mtx));

  #ifdef _RLS_DEBUG
    DebugFile << "\n-----------------------------------" << std::endl;
    DebugFile << "(*P_Mtx) -= ((*k_vec) * (*rv_work))" << std::endl;
  #endif
  (*P_Mtx) -= ((*k_vec) * (*rv_work));

  double *k_array = k_vec->array(); 
  for(tap_idx=0; tap_idx<Num_Taps; tap_idx++)
    {
    Unquan_Coeff[tap_idx] += err_samp * (k_array[tap_idx]);
    if( (tap_idx == Tap_For_Trans) &&
        (Update_Count < Transient_Len) )
      {
      Tally_For_Avg[Update_Count] += Unquan_Coeff[tap_idx];

      if(trans_save_enab)
         Sample_Transient[Update_Count] = Unquan_Coeff[tap_idx];
      }
    if( (tap_idx == Secondary_Tap) &&
        (Update_Count < Transient_Len) )
      {
      Tally_For_Avg_2[Update_Count] += Unquan_Coeff[tap_idx];

      if(trans_save_enab)
         Sample_Trans_2[Update_Count] = Unquan_Coeff[tap_idx];
      }
    }
  //==============================
  // debug stuff follows
  #ifdef _NOT_DEFINED_
  if(Update_Count == 50)
  {
  int ii, jj;
  for(ii=0; ii<Num_Taps; ii++)
    {
    std::cout << "u_vec[" << ii << "] = " << ((*u_vec)[ii]) << std::endl;
    }
  for(ii=0; ii<Num_Taps; ii++)
    {
    for(jj=0; jj<Num_Taps; jj++)
      {
      std::cout << "P_Mtx[" << ii << "][" << jj << "] = " 
           << ((*P_Mtx)[ii][jj]) << std::endl;
      }
    }
  std::cout << "denom = " << denom << std::endl;
  for(ii=0; ii<Num_Taps; ii++)
    {
    std::cout << "cv_work[" << ii << "] = " << ((*cv_work)[ii]) 
         << "  k_vec = " << ((*k_vec)[ii])<< std::endl;
    }
  } // end of if(Update_Count == 5)
  #endif

  cv_work->PurgeData();
  rv_work->PurgeData();
  k_vec->PurgeData();
  u_vec->PurgeData();
  delete cv_work;
  delete k_vec;
  delete u_vec;
  delete rv_work;
  delete[] k_array;
  if(Update_Count < 10)
    {
    test_ptr = new double[21];
    #ifdef _DEBUG
    DebugFile << (void*)test_ptr << std::endl;
    #endif
    delete[] test_ptr;
    }

  Update_Count++;
  return(err_samp);
}


