/*------------------------------------------------------------------------------

   HXA7241 General library.
   Copyright (c) 2005-2007,  Harrison Ainsworth / HXA7241.

   http://www.hxa7241.org/

------------------------------------------------------------------------------*/

#ifndef POWFAST_H
#define POWFAST_H

/*
 * Fast approximation to pow, with adjustable precision.<br/><br/>
 *
 * Precision can be 0 to 18.<br/>
 * Storage is (2 ^ precision) * 4 bytes -- 4B to 1MB<br/>
 * For precision 11: mean error < 0.01%, max error < 0.02%, storage 8KB.
 */
class PowFast
{
  /// standard object services
  /// ---------------------------------------------------
public:
  explicit PowFast(unsigned int precision = 11);
  ~PowFast();

private:
  PowFast(const PowFast&);
  PowFast& operator=(const PowFast&);

public:
  /// queries
  /// --------------------------------------------------------------------
  /* 2 ^ number. Number must be > -125 and < +128.*/
  float two(float) const;

  /* e ^ number. Number must be > -87.3ish and < +88.7ish. */
  float e(float) const;

  /* 10 ^ number. Number must be > -37.9ish and < +38.5ish. */
  float ten(float) const;

  /*
   * Get r ^ number.<br/><br/>
   *
   * @logr  logE of radix for power
   * @f     power to apply (beware under/over-flow)
   */
  float r(float logr, float f) const;

  unsigned int precision() const;

  /// fields
  /// ---------------------------------------------------------------------
private:
  unsigned int precision_m;
  unsigned int* pTable_m;
};

/// default instance
const PowFast& POWFAST();

#endif // POWFAST_H
