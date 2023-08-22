

#include "daq.hpp"

namespace NIDAQ
{

const std::string_view DEFAULT_DRIVER_NAME = "National Instruments Driver";

class Device : public DAQ::Device
{
public:
  Device();

  size_t getChannelCount(DAQ::type_t type) const final;
  bool getChannelActive(DAQ::type_t type, DAQ::index_t index) const final;
  int setChannelActive(DAQ::type_t type, DAQ::index_t index, bool state) final;
  size_t getAnalogRangeCount(DAQ::type_t type, DAQ::index_t index) const final;
  size_t getAnalogReferenceCount(DAQ::type_t type, DAQ::index_t index) const final;
  size_t getAnalogUnitsCount(DAQ::type_t type, DAQ::index_t index) const final;
  size_t getAnalogDownsample(DAQ::type_t type, DAQ::index_t index) const final;
  std::string getAnalogRangeString(DAQ::type_t type,
                                           DAQ::index_t index,
                                           DAQ::index_t range) const final;
  std::string getAnalogReferenceString(DAQ::type_t type,
                                               DAQ::index_t index,
                                               DAQ::index_t reference) const final;
  std::string getAnalogUnitsString(DAQ::type_t type,
                                           DAQ::index_t index,
                                           DAQ::index_t units) const final;
  double getAnalogGain(DAQ::type_t type, DAQ::index_t index) const final;
  double getAnalogZeroOffset(DAQ::type_t type, DAQ::index_t index) const final;
  DAQ::index_t getAnalogRange(DAQ::type_t type, DAQ::index_t index) const final;
  DAQ::index_t getAnalogReference(DAQ::type_t type, DAQ::index_t index) const final;
  DAQ::index_t getAnalogUnits(DAQ::type_t type, DAQ::index_t index) const final;
  DAQ::index_t getAnalogOffsetUnits(DAQ::type_t type, DAQ::index_t index) const final;
  int setAnalogGain(DAQ::type_t type, DAQ::index_t index, double gain) final;
  int setAnalogRange(DAQ::type_t type, DAQ::index_t index, DAQ::index_t range) final;
  int setAnalogZeroOffset(DAQ::type_t type,
                                  DAQ::index_t index,
                                  double offset) final;
  int setAnalogReference(DAQ::type_t type,
                                 DAQ::index_t index,
                                 DAQ::index_t reference) final;
  int setAnalogUnits(DAQ::type_t type, DAQ::index_t index, DAQ::index_t units) final;
  int setAnalogOffsetUnits(DAQ::type_t type,
                                   DAQ::index_t index,
                                   DAQ::index_t units) final;
  int setAnalogDownsample(DAQ::type_t type,
                                  DAQ::index_t index,
                                  size_t downsample) final;
  int setAnalogCounter(DAQ::type_t type, DAQ::index_t index) final;
  int setAnalogCalibrationValue(DAQ::type_t type,
                                        DAQ::index_t index,
                                        double value) final;
  double getAnalogCalibrationValue(DAQ::type_t type,
                                           DAQ::index_t index) const final;
  int setAnalogCalibrationActive(DAQ::type_t type,
                                         DAQ::index_t index,
                                         bool state) final;
  bool getAnalogCalibrationActive(DAQ::type_t type, DAQ::index_t index) const final;
  bool getAnalogCalibrationState(DAQ::type_t type, DAQ::index_t index) const final;
  DAQ::direction_t getDigitalDirection(DAQ::index_t index) const final;
  int setDigitalDirection(DAQ::index_t index, DAQ::direction_t direction) final;

};

class Driver : public DAQ::Driver
{
public:
  static DAQ::Driver* getInstance();
  void loadDevices() final;
  void unloadDevices() final;
  std::vector<DAQ::Device*> getDevices() final;
private:
  Driver();
  std::vector<NIDAQ::Device> nidaq_devices;
};

} // namespace NIDAQ
