#ifndef _TYPEDEFS_H_
#define _TYPEDEFS_H_

enum typesOfResponse
{
  UNDEF_RESP_ = 0,
  LOWPASS_RESP_,
  BANDPASS_RESP_,
  HIGHPASS_RESP_,
  BANDSTOP_RESP_
};
typedef enum typesOfResponse BAND_CONFIG_TYPE;

#endif // _TYPEDEFS_H_
