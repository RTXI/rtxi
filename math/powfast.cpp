/*------------------------------------------------------------------------------

   HXA7241 General library.
   Copyright (c) 2005-2007,  Harrison Ainsworth / HXA7241.

   http://www.hxa7241.org/

------------------------------------------------------------------------------*/

#include "powfast.hpp"
#include <math.h>

/// implementation -------------------------------------------------------------

namespace {

/**
 * Following the bit-twiddling idea in:
 *
 * 'A Fast, Compact Approximation of the Exponential Function'
 * Technical Report IDSIA-07-98
 * Nicol N. Schraudolph;
 * IDSIA,
 * 1998-06-24.
 *
 * [Rewritten for floats by HXA7241, 2007.]
 *
 * and the adjustable-lookup idea in:
 *
 * 'Revisiting a basic function on current CPUs: A fast logarithm implementation
 * with adjustable accuracy'
 * Technical Report ICSI TR-07-002;
 * Oriol Vinyals, Gerald Friedland, Nikki Mirghafori;
 * ICSI,
 * 2007-06-21.
 *
 * [Improved (doubled accuracy) and rewritten by HXA7241, 2007.]
 */

const float _2p23 = 8388608.0f;

/*
 * Initialize powFast lookup table.
 *
 * @pTable     length must be 2 ^ precision
 * @precision  number of mantissa bits used, >= 0 and <= 18
 */
void
powFastSetTable(unsigned int* const pTable, const unsigned int precision)
{
  // step along table elements and x-axis positions
  float zeroToOne = 1.0f / (static_cast<float>(1 << precision) * 2.0f);
  for (int i = 0; i < (1 << precision); ++i) {
    // make y-axis value for table element
    const float f = (::powf(2.0f, zeroToOne) - 1.0f) * _2p23;
    pTable[i] = static_cast<unsigned int>(f < _2p23 ? f : (_2p23 - 1.0f));
    zeroToOne += 1.0f / static_cast<float>(1 << precision);
  }
}

/*
 * Get pow (fast!).
 *
 * @val        power to raise radix to
 * @ilog2      one over log, to required radix, of two
 * @pTable     length must be 2 ^ precision
 * @precision  number of mantissa bits used, >= 0 and <= 18
 */
inline float
powFastLookup(const float val, const float ilog2, unsigned int* const pTable,
              const unsigned int precision)

{
  // build float bits
  const int i = static_cast<int>((val * (_2p23 * ilog2)) + (127.0f * _2p23));

  // replace mantissa with lookup
  const int it = (i & 0xFF800000) | pTable[(i & 0x7FFFFF) >> (23 - precision)];

  // convert bits to float
  return *reinterpret_cast<const float*>(&it);
}
}

/// wrapper class --------------------------------------------------------------

PowFast::PowFast(const unsigned int precision)
  : precision_m(precision <= 18u ? precision : 18u)
  , pTable_m(new unsigned int[1 << precision_m])
{
  powFastSetTable(pTable_m, precision_m);
}

PowFast::~PowFast()
{
  delete[] pTable_m;
}

float
PowFast::two(const float f) const
{
  return powFastLookup(f, 1.0f, pTable_m, precision_m);
}

float
PowFast::e(const float f) const
{
  return powFastLookup(f, 1.44269504088896f, pTable_m, precision_m);
}

float
PowFast::ten(const float f) const
{
  return powFastLookup(f, 3.32192809488736f, pTable_m, precision_m);
}

float
PowFast::r(const float logr, const float f) const
{
  return powFastLookup(f, (logr * 1.44269504088896f), pTable_m, precision_m);
}

unsigned int
PowFast::precision() const
{
  return precision_m;
}

/// default instance -----------------------------------------------------------
const PowFast&
POWFAST()
{
  static const PowFast k(11);
  return k;
}

/// test -----------------------------------------------------------------------
#ifdef TESTING

#include <float.h>
#include <iomanip>
#include <ostream>
#include <stdlib.h>

typedef unsigned int udword;
typedef int dword;

namespace {

/*
 * rand [0,1) (may be coarsely quantized).
 */
float
randFloat()
{
  return static_cast<float>(static_cast<udword>(::rand())) /
         static_cast<float>(static_cast<udword>(RAND_MAX) + 1);
}
}

bool
test_PowFast(std::ostream* pOut, const bool isVerbose, const dword seed)
{
  bool isOk = true;

  if (pOut)
    *pOut << "[ test_PowFast ]\n\n";

  if (pOut) {
    pOut->setf(std::ios_base::scientific, std::ios_base::floatfield);
    pOut->precision(6);
  }

  ::srand(static_cast<unsigned>(seed));

  /// adjustable
  {
    const PowFast powFastAdj(11);

    const dword J_COUNT = 1000;

    float sumDifE = 0.0f;
    float maxDifE = static_cast<float>(FLT_MIN);

    const dword E_START = -86;
    const dword E_END = +88;
    for (dword i = E_START; i < E_END; ++i) {
      for (dword j = 0; j < J_COUNT; ++j) {
        const float number = static_cast<float>(i) + randFloat();
        const float pe = ::powf(2.71828182845905f, number);
        const float pef = powFastAdj.e(number);
        const float peDif = ::fabsf(pef - pe) / pe;
        sumDifE += peDif;
        maxDifE = (maxDifE >= peDif) ? maxDifE : peDif;

        if ((0 == j) && pOut && isVerbose) {
          *pOut << std::showpos << std::fixed << std::setw(10) << number
                << std::scientific << std::noshowpos;
          *pOut << "  E " << pef << " " << peDif << "\n";
        }
      }
    }

    if (pOut && isVerbose)
      *pOut << "\n";

    float sumDifT = 0.0f;
    float maxDifT = static_cast<float>(FLT_MIN);
    const dword T_START = -36;
    const dword T_END = +38;

    for (dword i = T_START; i < T_END; ++i) {
      for (dword j = 0; j < J_COUNT; ++j) {
        const float number = static_cast<float>(i) + randFloat();
        const float pt = ::powf(10.0f, number);
        const float ptf = powFastAdj.ten(number);
        const float ptDif = ::fabsf(ptf - pt) / pt;
        sumDifT += ptDif;
        maxDifT = (maxDifT >= ptDif) ? maxDifT : ptDif;

        if ((0 == j) && pOut && isVerbose) {
          *pOut << std::showpos << std::fixed << std::setw(10) << number
                << std::scientific << std::noshowpos;
          *pOut << "  T " << ptf << " " << ptDif << "\n";
        }
      }
    }

    if (pOut && isVerbose)
      *pOut << "\n";

    const float meanDifE = sumDifE / (static_cast<float>(E_END - E_START) *
                                      static_cast<float>(J_COUNT));
    const float meanDifT = sumDifT / (static_cast<float>(T_END - T_START) *
                                      static_cast<float>(J_COUNT));

    if (pOut && isVerbose)
      *pOut << "precision: " << powFastAdj.precision() << "\n";
    if (pOut && isVerbose)
      *pOut << "mean diff,  E: " << meanDifE << "  10: " << meanDifT << "\n";
    if (pOut && isVerbose)
      *pOut << "max  diff,  E: " << maxDifE << "  10: " << maxDifT << "\n";
    if (pOut && isVerbose)
      *pOut << "\n";

    bool isOk_ = (meanDifE < 0.0001f) & (meanDifT < 0.0001f) &
                 (maxDifE < 0.0002f) & (maxDifT < 0.0002f);

    if (pOut)
      *pOut << "adjustable : " << (isOk_ ? "--- succeeded" : "*** failed")
            << "\n\n";

    isOk &= isOk_;
  }

  if (pOut)
    *pOut << (isOk ? "--- successfully" : "*** failurefully") << " completed "
          << "\n\n";
  if (pOut)
    pOut->flush();

  return isOk;
}

#endif // TESTING
