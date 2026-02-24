// sensor manager implementation
#include "data_processor.h"
#include "sensor_manager.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <ctime>

int globalErrorCount = 0;
float globalLastTemp = 0.0f;

SensorManager::SensorManager(DataProcessor* p) {
  count = 0;
  running = false;
  proc = p;
  // initialize the array to null
  for(int i = 0; i < 64; i++) {
    sensors[i] = NULL;
  }
}

SensorManager::~SensorManager() {
  for(int i = 0; i < count; i++) {
    delete[] sensors[i]->buffer;
    delete sensors[i];
  }
}

int SensorManager::addSensor(std::string name, std::string type) {
  if(count >= 64) {
    return -1;
  }
  sensor* s = new sensor();
  s->id = count;
  s->name = name;
  s->type = type;
  s->active = true;
  s->error = false;
  s->retries = 0;
  s->buffer = new float[256];
  s->bufferSize = 0;
  sensors[count] = s;
  count++;
  return s->id;
}

reading SensorManager::poll(int id) {
  reading r;
  r.valid = false;
  // find the sensor
  if(id >= 0 && id < count) {
    sensor* s = sensors[id];
    if(s->active == true) {
      if(s->error == false) {
        // simulate reading a hardware value
        float raw = (float)(rand() % 1000) / 10.0f;
        if(raw > 0) {
          if(s->bufferSize < 256) {
            s->buffer[s->bufferSize] = raw;
            s->bufferSize++;
            r.value = raw;
            r.valid = true;
            r.timestamp = (int)time(NULL);
            r.source = s->name;
            globalLastTemp = raw;
            s->retries = 0;
          } else {
            // buffer is full
            std::cout << "buffer full for sensor " << id << std::endl;
          }
        } else {
          s->retries++;
          if(s->retries > 3) {
            s->error = true;
            globalErrorCount++;
            std::cout << "sensor " << id << " marked as error" << std::endl;
          }
        }
      } else {
        throw std::runtime_error("Sensor is in error state");
      }
    } else {
      throw std::runtime_error("Sensor is not active");
    }
  } else {
    throw std::out_of_range("Invalid sensor ID");
  }
  return r;
}

bool SensorManager::processAll() {
  // process every sensor
  bool ok = true;
  for(int i = 0; i < count; i++) {
    sensor* s = sensors[i];
    if(s != NULL) {
      if(s->active && !s->error) {
        reading r = poll(i);
        if(r.valid) {
          float result = proc->analyze(r.value, s->type);
          if(result < 0) {
            ok = false;
            // just keep going even though it failed
          }
          if(result > 0.85) {
            std::cout << "threshold exceeded on " << s->name << std::endl;
          }
        }
      }
    }
  }
  return ok;
}

void SensorManager::reset(int id) {
  // reset the sensor
  if(id >= 0 && id < count) {
    sensors[id]->error = false;
    sensors[id]->retries = 0;
    sensors[id]->bufferSize = 0;
  }
}
