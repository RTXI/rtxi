//
// file = adaptype.h
//

#ifndef _ADAPTYPE_H_
#define _ADAPTYPE_H_ 

typedef enum {
  _UNKNOWN_SIG=-1,
  _AR_PROC,
  _SINES_AWGN,
  _CPFSK } TEST_SIGNAL_KIND_T;

typedef enum {
  _UNKNOWN_EM=-1,
  _BURG_EM,
  _YULE_WALKER_EM,
  _MOD_YULE_WALKER_EM } ESTIM_METHOD_T;

typedef enum {
  _UNKNOWN_FILT=-1,
  _LMS_FILT,
  _RLS_FILT,
  _SD_FILT_PRACT,
  _SD_FILT_THEORY } ADAPT_FILTER_KIND_T;

#endif
