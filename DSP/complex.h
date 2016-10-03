//
//  file = complex.h
//

#ifndef _COMPLEX_H_
#define _COMPLEX_H_ 

#include <math.h>
#undef complex
#undef cabs

class complex
{
public:
  complex(double _r, double _i=0.0);
  complex(float _r, float _i);
  complex();
  void operator+= (const complex);
  void operator-= (const complex);
  void operator*= (const complex);
  void operator*= (double);
  void operator/= (double);
 // void operator=(const complex&);
//  void operator=(const complex_vector&);
  
  friend double real(const complex&);
  friend double imag(const complex&);
  friend complex conj(const complex);
  friend complex sqrt(const complex);
  friend double norm(const complex);
  friend double cabs(const complex);
  friend double mag(const complex);
  friend double arg(const complex);
  friend complex cexp(const complex);
  friend double mag_sqrd(const complex);
  friend complex operator- (const complex);
  friend complex operator+ (const complex, const complex);
  friend complex operator- (const complex, const complex);
  friend complex operator* (const complex, const complex);
  friend complex operator* (const complex, double);
  friend complex operator* (double, const complex);
  friend complex operator/ (const complex, const complex);
  friend complex operator/ (const complex, double);
  friend complex operator/ (double, const complex);
  
private:
  double re, im;
  
};

inline complex::complex( double _r, double _i) : re(_r), im(_i) { }
inline complex::complex( float _r, float _i) 
                      : re((double)_r), im((double)_i) { }
inline complex::complex() : re(0.0), im(0.0) { }
inline void complex::operator+= (const complex _z)
       { re += _z.re; im += _z.im; }
inline void complex::operator-= (const complex _z)
       { re -= _z.re; im -= _z.im; }
inline void complex::operator*= (double _v) {re *= _v; im *= _v; }
inline void complex::operator*= (const complex value)
  {
    double real_part;
    re = (real_part = re) * value.re - im * value.im;
    im = real_part * value.im + im * value.re;
  }
/*        
inline void complex::operator= (const complex &right)
{
  re = right.re;
  im = right.im;
}      
*/                                                                 
inline double real(const complex& _z) { return _z.re; }
inline double imag(const complex& _z) { return _z.im; } 
inline complex conj(const complex _z)
  { return complex(_z.re, -_z.im); }
inline double norm(const complex _z) { return sqrt(_z.re*_z.re + _z.im*_z.im); }
inline double cabs(const complex _z) { return sqrt(_z.re*_z.re + _z.im*_z.im); }
inline double mag(const complex _z) { return sqrt(_z.re*_z.re + _z.im*_z.im); }
inline double mag_sqrd(const complex _z) { return _z.re*_z.re + _z.im*_z.im; }
inline double arg(const complex _z) { return (double)atan2( (double)_z.im, 
                                                                   (double)_z.re); }

inline complex cexp( const complex _z)
  {
  double re_res, im_res;
  re_res = exp(_z.re)*cos(_z.im);
  im_res = exp(_z.im)*sin(_z.im);
  return(complex(re_res, im_res));
  }

inline complex sqrt( const complex _z)
  {
    double r, theta, re_res, im_res;
    r = sqrt(sqrt(_z.re*_z.re + _z.im*_z.im));
    theta = atan2( _z.im, _z.re)/2.0;
    re_res = r*cos(theta);
    im_res = r*sin(theta);
    return(complex(re_res, im_res));
  }
inline complex operator- (const complex _z)
  { return complex(-_z.re, -_z.im); }

inline complex operator+ ( const complex _z1,
                                 const complex _z2)
  { return complex( _z1.re + _z2.re, _z1.im + _z2.im);} 

inline complex operator- ( const complex _z1, 
                                 const complex _z2)
  { return complex( _z1.re - _z2.re, _z1.im - _z2.im); }

inline complex operator* (const complex _z1, double _v2)
  { return complex( _z1.re * _v2, _z1.im * _v2); }
  
inline complex operator* (double _v1, const complex _z2)
  { return complex(_z2.re * _v1, _z2.im * _v1); }
  
inline complex operator/ (const complex z1, double v2)
  { return complex(z1.re/v2, z1.im/v2); }
  
inline complex operator* ( const complex value1,
                                 const complex value2)
  {
    double real, imag;
    real = value1.re * value2.re - value1.im * value2.im;
    imag = value1.re * value2.im + value1.im * value2.re;
    return complex(real, imag);
  }
  
  
#include <iostream>

std::ostream& operator<< (std::ostream&, const complex&);
std::istream& operator>> (std::istream&, complex&);

static const complex complex_zero(0.0, 0.0);

#endif // _complex_H_
