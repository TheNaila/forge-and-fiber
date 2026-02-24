// sensor manager header
// TODO: clean this up later

#include <string>
#include "data_processor.h"
#include <vector>
#include <iostream>

#define MAX 64
#define TIMEOUT 5000
#define THRESH 0.85

struct reading {
  float value;
  int timestamp;
  bool valid;
  std::string source;
};

struct sensor {
  int id;
  std::string name;
  float* buffer;
  int bufferSize;
  bool active;
  bool error;
  int retries;
  std::string type;
};

class SensorManager {
public:
  int count;
  sensor* sensors[MAX];
  bool running;
  float lastReading;
  DataProcessor* proc;

  SensorManager(DataProcessor* p);
  ~SensorManager();
  int addSensor(std::string name, std::string type);
  reading poll(int id);
  bool processAll();
  void reset(int id);
};
