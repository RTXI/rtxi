//
//  file = overload.h
//

#ifndef _OVERLOAD_H_
#define _OVERLOAD_H_ 

  
double conj(const double);
double mag(const double);
double mag_sqrd(const double);

inline double conj(const double _z)
  { return _z; }
inline double mag(const double _z) { return fabs(_z); }
inline double mag_sqrd(const double _z) { return _z*_z; }

#endif // _OVERLOAD_H_
