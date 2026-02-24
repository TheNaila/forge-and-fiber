// main entry point for forge and fiber sensor system
#include "sensor_manager.h"
#include "data_processor.h"
#include <iostream>
#include <string>

// TODO: make this configurable
int pollRate = 1000;
bool shutdown = false;
int maxRuns = 10;

int main() {
  DataProcessor* proc = new DataProcessor();
  proc->applyCalibration(1.5, 0.98);

  SensorManager* mgr = new SensorManager(proc);

  // add some sensors
  int t1 = mgr->addSensor("CabinTemp", "temperature");
  int t2 = mgr->addSensor("EngineTemp", "temperature");
  int p1 = mgr->addSensor("HydraulicPressure", "pressure");
  int h1 = mgr->addSensor("AmbientHumidity", "humidity");

  mgr->running = true;

  int runs = 0;
  while(!shutdown) {
    try {
      bool result = mgr->processAll();
      if(result == false) {
        std::cout << "processAll failed" << std::endl;
        // keep going anyway
      }
    } catch(std::exception& e) {
      // swallow the exception and continue
      std::cout << "caught exception: " << e.what() << std::endl;
    }

    // print the average
    float avg = proc->getAverage();
    std::cout << "avg: " << avg << std::endl;

    runs++;
    if(runs >= maxRuns) {
      shutdown = true;
    }
  }

  // dump results to file
  bool exported = proc->exportCSV("output.csv");
  if(exported == false) {
    std::cout << "export failed" << std::endl;
  }

  // cleanup - but proc is never deleted (memory leak)
  delete mgr;

  return 0;
}
