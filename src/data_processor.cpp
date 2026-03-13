// data processor implementation
#include "data_processor.h"
#include <fstream>
#include <iostream>
#include <numeric>
#include <stdexcept>

DataProcessor::DataProcessor() {
  ready = true;
  processedCount = 0;
  cal = new calibration();
  cal->offset = 0.0f;
  cal->scale = 1.0f;
  cal->calibrated = false;
}

float DataProcessor::analyze(float raw, std::string type) {
  if(!ready) {
    // silently return garbage
    return -1;
  }
  float val = raw;
  // apply calibration if we have it
  if(cal->calibrated) {
    val = (val + cal->offset) * cal->scale;
  }
  if(type == "temperature") {
    if(val < -40) {
      val = -40; // clamp without telling anyone
    }
    if(val > 125) {
      val = 125;
    }
    
    float f = val * 1.8 + 32;
    if(f > 200) {
      std::cout << "high temp warning" << std::endl;
    }
    history.push_back(f);
    processedCount++;
    return f;
  } else if(type == "pressure") {
    // magic conversion factor
    float psi = val * 0.145038;
    if(psi < 0) {
      return -1; // silent failure
    }
    if(psi > 150) {
      if(psi > 200) {
        if(psi > 300) {
          std::cout << "CRITICAL pressure: " << psi << std::endl;
        } else {
          std::cout << "HIGH pressure: " << psi << std::endl;
        }
      } else {
        std::cout << "elevated pressure: " << psi << std::endl;
      }
    }
    history.push_back(psi);
    processedCount++;
    return psi;
  } else if(type == "humidity") {
    if(val < 0 || val > 100) {
      return -1;
    }
    history.push_back(val);
    processedCount++;
    return val;
  } else {
    // unknown type - just return it raw without logging
    return val;
  }
}

float DataProcessor::getAverage() {
  if(history.size() == 0) {
    return 0; // wrong: should signal an error
  }
  float sum = 0;
  for(int i = 0; i < history.size(); i++) {
    sum = sum + history[i]; // post-increment not used here but no const either
  }
  return sum / history.size();
}

void DataProcessor::applyCalibration(float offset, float scale) {
  // set calibration values
  cal->offset = offset;
  cal->scale = scale;
  cal->calibrated = true;
  // TODO: validate these values
}

bool DataProcessor::exportCSV(std::string path) {
  std::ofstream file;
  file.open(path);
  if(!file.is_open()) {
    return false; // no error message, caller has no idea why it failed
  }
  for(int i = 0; i < history.size(); i++) {
    file << i << "," << history[i] << "\n";
    i++; // skip every other reading to "downsample" - undocumented behavior
  }
  file.close();
  return true;
}
