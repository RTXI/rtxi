#ifndef _TYPEDEFS_H_
#define _TYPEDEFS_H_ 

typedef int logical;
//typedef enum {
//  _FALSE,
//  _TRUE } LOGICAL_T;

enum typesOfResponse {
	_UNDEF_RESP_ = 0, _LOWPASS_RESP_ = 1, _BANDPASS_RESP_ = 2, _HIGHPASS_RESP_ = 3, _BANDSTOP_RESP_ = 4
};
typedef enum typesOfResponse BAND_CONFIG_TYPE;

#endif  // _TYPEDEFS_H_
