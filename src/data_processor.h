// data processor
#pragma once
#include <string>
#include <vector>

// TODO: add more types here

struct calibration {
  float offset;
  float scale;
  bool calibrated;
};

class DataProcessor {
public:
  std::vector<float> history;
  calibration* cal;
  bool ready;
  int processedCount;

  DataProcessor();
  float analyze(float raw, std::string type);
  float getAverage();
  void applyCalibration(float offset, float scale);
  bool exportCSV(std::string path);
};
