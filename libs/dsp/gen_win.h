/*
 * Generic window for filter
 */

#ifndef _GEN_WIN_H_
#define _GEN_WIN_H_

#include <vector>

/*! Generic Window base class
 *
 * This class serves as an interface for generic windows. Windows are
 * used for DFT inputs to reduce signal leakage and improve power
 * spectrum estimation.
 */
class GenericWindow
{
public:
  /*!
   * Creates a Window
   */
  GenericWindow();

  /*!
   * Creates a window with specified length
   *
   * When dealing with Windowing functions, users can either use a "lag"
   * window or a "data" window. They are conceptually the same. This
   * constructor will create an array of length size and pupulate it using
   * the initialize function, which relies on the half of the lag window.
   *
   * \param length The size of the window
   */
  explicit GenericWindow(int length);

  /*!
   * Creates a full window using the internal data stored in half lag window.
   *
   * \param length The size of the window
   */
  void Initialize(int length);

  /*!
   * Return the value of internal buffer at provided index.
   *
   * \param samp_indx The index where to get the value
   *
   * \returns The value at the provided index inside the internal buffer
   */
  double GetDataWinCoeff(int samp_indx);

  /*!
   * Normalize the values inside internal buffer have maximum 1.0
   */
  void NormalizeWindow();

  /*!
   * Obtain a pointer to the data window internal buffer.
   *
   * \returns Pointer to the first element of the data window buffer
   */
  double* GetDataWindow();

  /*!
   * Obtain a pointer to the half of the lag window internal buffer.
   *
   * \returns Pointer to the first element of the lag window internal buffer
   */
  double* GetHalfLagWindow();

  /*!
   * Obtain the number of "taps" for this window
   *
   * Taps is a representation of the number of points needed in signal
   * processing and it is closely related to the data window.
   *
   * \returns Number of taps
   */
  int GetNumTaps() const;

  /*!
   * Get half the length of the window
   *
   * This is not just taking half of the length, as it is required to treat
   * even and odd windows differently.
   *
   * \returns Half the size of the window
   */
  int GetHalfLength() const;

  /*!
   * Get the length of the window.
   *
   * Both the data window and lag window contain the same length
   *
   * \returns The length of the internal data window/lag window buffer
   */
  int GetLength() const;

private:
  std::size_t Length=0;
  std::size_t Half_Length=0;
  std::vector<double> Half_Lag_Win;
  std::vector<double> Lag_Win;
  std::vector<double> Data_Win;
};

#endif
