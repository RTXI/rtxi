//
// file = adaptype.h
//

#ifndef _ADAPTYPE_H_
#define _ADAPTYPE_H_

typedef enum {
  UNKNOWN_SIG = -1,
  AR_PROC,
  SINES_AWGN,
  CPFSK
} TEST_SIGNAL_KIND_T;

typedef enum {
  UNKNOWN_EM = -1,
  BURG_EM,
  YULE_WALKER_EM,
  MOD_YULE_WALKER_EM
} ESTIM_METHOD_T;

typedef enum {
  UNKNOWN_FILT = -1,
  LMS_FILT,
  RLS_FILT,
  SD_FILT_PRACT,
  SD_FILT_THEORY
} ADAPT_FILTER_KIND_T;

#endif
