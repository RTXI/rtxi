//
//  File = filt_imp.h
//

#ifndef _FILT_IMP_H_
#define _FILT_IMP_H_

#include <cstdint>

/*!
  * Filter Implementation Base Class
  *
  * This serves as an interface for filter implementations
  */
class FilterImplementation
{
public:
  FilterImplementation() = default;
  FilterImplementation(const FilterImplementation&) = default;
  FilterImplementation(FilterImplementation&&) = delete;
  FilterImplementation& operator=(const FilterImplementation&) = default;
  FilterImplementation& operator=(FilterImplementation&&) = delete;
  virtual ~FilterImplementation() = default;
  
  /*!
    * Obtain the number of taps for this filter
    *
    * \return The number of taps
    */
  virtual int GetNumTaps() = 0;

  /*!
    * Process the input value through filter
    *
    * \param input_val The input value
    *
    * \return Processed value
    */
  virtual double ProcessSample(double input_val) = 0;

  /*!
    * Process the input value through filter
    *
    * \param input_val The input value
    *
    * \return Processed value
    */
  virtual int64_t ProcessSample(int64_t input_val) = 0;
};

#endif
