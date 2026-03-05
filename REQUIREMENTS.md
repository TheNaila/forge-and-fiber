# Forge & Fiber Sensor System — Engineering Requirements

**Document Version:** 1.0
**Status:** Approved
**Last Updated:** 2026-03-04
**Owner:** Forge & Fiber Platform Engineering

---

## 1. Overview

### 1.1 Purpose

The Forge & Fiber Sensor System is a C++ application that polls a configurable set of hardware sensors, applies per-type calibration and processing, and exports the collected readings to CSV for downstream analysis. It is designed for deployment in environments where sensor data feeds safety-critical decisions; correctness and deterministic failure behavior are primary design constraints.

### 1.2 Scope

This document covers sensor management, data processing, calibration, error handling, CSV export, build requirements, and quality standards. Hardware driver integration, network telemetry, and real-time OS scheduling are out of scope.

### 1.3 Guiding Principle

Per the Forge & Fiber Style Guide: *"Failure isn't just a bug — it's a liability."* All system behavior under error conditions must be explicit, logged, and handled. Silent failures are not acceptable.

---

## 2. System Architecture

The system is organized into two primary components:

- **SensorManager** — registers sensors, maintains per-sensor state, polls readings, and routes raw values to the processor.
- **DataProcessor** — applies calibration, performs type-specific analysis and range enforcement, maintains a history of processed readings, and exports data to CSV.

A thin `main.cpp` entry point initializes both components, runs the main polling loop, and handles top-level shutdown.

The build system is CMake (minimum version 3.10), targeting the C++14 standard. The compiled executable is named `sensor_system`.

---

## 3. Error Handling

### FR-ERR-1: Result Type

All functions that can fail **must** return a `Result<T, ErrorCode>` type rather than returning sentinel values (e.g., `-1`, `false`, or `nullptr`) or throwing exceptions as a control-flow mechanism. Callers **must** explicitly handle both the success and failure cases.

### FR-ERR-2: No Silent Failures

No function **may** discard an error condition without either propagating it to the caller or logging a diagnostic message. Returning a default value (such as `0.0` or `false`) in place of an error is not permitted unless explicitly documented as a safe fallback.

### FR-ERR-3: Fail Fast

The system **must** halt or enter a defined safe state upon encountering an unrecoverable error. Catching an exception and continuing normal operation without corrective action is not permitted.

### FR-ERR-4: Error Logging

Every error condition **must** produce a log entry that includes: the component name, the function name, the sensor ID or name (if applicable), and a human-readable description of the fault. Log entries **must** be written to stderr.

---

## 4. SensorManager

### FR-SM-1: Sensor Registration

- The manager **must** support registration of up to 64 sensors.
- Each sensor **must** be assigned a unique integer ID at registration time.
- Each sensor **must** have a name (non-empty string) and a type (`temperature`, `pressure`, or `humidity`).
- Attempting to register a sensor when the maximum capacity is reached **must** return an error via the Result type; it **must not** silently overwrite an existing entry.

### FR-SM-2: Sensor State

Each registered sensor **must** track the following state:

| Field | Type | Description |
|---|---|---|
| `id` | `int` | Unique identifier |
| `name` | `std::string` | Human-readable label |
| `type` | `std::string` | Sensor category |
| `active` | `bool` | Whether the sensor is currently enabled |
| `error` | `bool` | Whether the sensor is in a fault state |
| `retries` | `int` | Count of consecutive failed reads |

### FR-SM-3: Read Buffer

Each sensor **must** maintain a circular buffer of its most recent raw readings. Buffer capacity **must** be defined as a named constant; it **must not** be a magic number in the source code.

### FR-SM-4: Polling

- `poll(id)` **must** simulate or read a sensor value, validate it, record it in the sensor's circular buffer, and return a `reading` struct.
- A reading of exactly zero **must** increment the sensor's retry counter.
- After three consecutive zero readings, the sensor **must** be transitioned to error state and the failure **must** be reported to the caller via the Result type.
- `poll()` **must not** throw an exception when called on a sensor in error state; it **must** return a failure Result.

### FR-SM-5: Automatic Error Recovery

- A sensor in error state **must** be eligible for automatic recovery after a configurable timeout (`TIMEOUT_MS`).
- The `TIMEOUT_MS` value **must** be defined as a named constant and **must** be enforced during polling; it **must not** be declared and left unused.
- Manual reset via `reset(id)` **must** also remain available.

### FR-SM-6: Batch Processing

`processAll()` **must** poll all active sensors and pass each reading to `DataProcessor::analyze()`. Sensors in error state **must** be skipped with a per-sensor log entry. The count of successfully processed sensors **must** be returned to the caller.

### FR-SM-7: No Global State

Sensor state and error counts **must** be encapsulated within the `SensorManager` instance. Module-level global variables for error counts or cached sensor values are not permitted.

### FR-SM-8: Poll Rate

The main loop **must** use the `pollRate` value (milliseconds between iterations) to control actual timing between polls. Declaring `pollRate` without using it to introduce any delay or scheduling is not compliant.

---

## 5. DataProcessor

### FR-DP-1: Calibration

- `applyCalibration(offset, scale)` **must** set the offset and scale factors and mark the processor as calibrated.
- `analyze()` **must** return a failure Result if the processor has not been calibrated.
- Calibration parameters **must** be validated; a scale of exactly `0.0` **must** be rejected with an error.

### FR-DP-2: Analysis — Temperature

- Temperature readings **must** have calibration applied (`(value + offset) * scale`) before range enforcement.
- Values outside `[TEMP_MIN_C, TEMP_MAX_C]` **must** produce a warning log entry and **must** be returned as a failure Result. Silent clamping to the valid range is not permitted.
- `TEMP_MIN_C` and `TEMP_MAX_C` **must** be defined as named constants.
- Values that exceed the high-temperature warning threshold **must** log a warning. The threshold **must** be a named constant.
- The Celsius-to-Fahrenheit conversion factor **must** be a named constant; it **must not** be a magic number inline in the calculation.

### FR-DP-3: Analysis — Pressure

- Pressure readings **must** have calibration applied before unit conversion.
- The conversion factor from raw units to PSI **must** be defined as a named constant (e.g., `BAR_TO_PSI`).
- Pressure exceeding defined warning thresholds **must** produce log entries. All threshold values **must** be named constants.
- Negative pressure values **must** return a failure Result with a log entry.

### FR-DP-4: Analysis — Humidity

- Humidity readings outside `[0.0, 100.0]` **must** return a failure Result with a log entry. Silent discard is not permitted.

### FR-DP-5: Unknown Sensor Types

- `analyze()` receiving an unrecognized sensor type **must** return a failure Result with a log entry. Returning the raw value without processing is not permitted.

### FR-DP-6: History

- All successfully processed readings **must** be appended to the history vector.
- Failed readings (returned as error Results) **must not** be appended to history.

### FR-DP-7: Average

- `getAverage()` **must** return a `Result<float, ErrorCode>`. When history is empty, it **must** return a failure Result, not `0.0f`.

### FR-DP-8: CSV Export

- `exportCSV(path)` **must** write every entry in the history vector to the output file — one row per reading, with no rows skipped.
- The loop index variable **must** be incremented exactly once per iteration.
- If the file cannot be opened, the function **must** return a failure Result with a log entry describing the path and the system error.
- The CSV header row **must** be written before any data rows.

---

## 6. Memory Management

### FR-MEM-1: No Raw Owning Pointers

Dynamically allocated objects **must** be managed with `std::unique_ptr` or `std::shared_ptr`. Use of raw `new` and `delete` for ownership management is not permitted.

### FR-MEM-2: No Leaks at Exit

All allocated resources **must** be released before `main()` returns. Every object created with `new` **must** have a corresponding `delete` path reachable under all exit conditions, including exception paths.

---

## 7. Naming and Code Style

All code **must** conform to the Forge & Fiber Style Guide. The following rules are mandatory for this release:

### FR-STYLE-1: No Magic Numbers

Every numeric literal with domain meaning (thresholds, conversion factors, capacities, timeouts) **must** be defined as a named `constexpr` constant with a descriptive name in the appropriate header. Inline numeric literals in logic code are not permitted.

### FR-STYLE-2: Naming Conventions

| Context | Convention | Example |
|---|---|---|
| Global variables | `g_` prefix | `g_errorCount` |
| Member variables | `m_` prefix | `m_bufferSize` |
| Pointer parameters | `p_` prefix | `p_processor` |
| Boolean variables | `is` / `has` prefix | `isCalibrated`, `hasError` |
| Constants | `SCREAMING_SNAKE_CASE` | `TEMP_MAX_C` |
| Functions / methods | `camelCase` | `applyCalibration()` |

### FR-STYLE-3: Increment Style

Pre-increment (`++i`) **must** be used in all loop counters. Post-increment (`i++`) is not permitted in loop headers.

### FR-STYLE-4: Const Correctness

All variables and parameters that are not modified after initialization **must** be declared `const`. Member functions that do not modify object state **must** be declared `const`.

### FR-STYLE-5: No Exceptions for Control Flow

C++ exceptions **must not** be used as a primary error-signaling mechanism. The Result pattern (FR-ERR-1) is the required approach. Exceptions **may** only propagate from third-party library calls that cannot be changed.

---

## 8. Documentation

### FR-DOC-1: Doxygen Coverage

Every public class, public function, and public constant **must** have a Doxygen comment block. Coverage **must** be 100% as measured by Doxygen's undocumented-symbol report.

### FR-DOC-2: Function Documentation

Each Doxygen block for a function **must** include:
- `@brief` — one-sentence summary.
- `@param` — one entry per parameter, describing type constraints and valid ranges.
- `@return` — description of the return value and all possible Result states.

### FR-DOC-3: File Headers

Every `.cpp` and `.h` file **must** begin with an SPDX license identifier comment and a `@file` Doxygen tag, per the style guide.

### FR-DOC-4: TODO Format

Any deferred work noted in comments **must** follow the format:
```
// TODO(author): TICKET-ID — description
```
Bare `// TODO: clean this up later` comments are not compliant.

---

## 9. Testing

### FR-TEST-1: Test Framework

The project **must** include a unit test suite integrated into the CMake build. A suitable framework (e.g., Google Test, Catch2) **must** be declared as a CMake dependency.

### FR-TEST-2: Coverage Requirement

Line coverage for `sensor_manager.cpp` and `data_processor.cpp` **must** be at or above 90% as measured by a coverage tool (e.g., gcov/lcov).

### FR-TEST-3: Required Test Cases

Tests **must** cover at minimum:

- `analyze()` correctly applies calibration offset and scale before range checks.
- `analyze()` returns a failure Result (not a clamped value) for out-of-range temperature.
- `analyze()` returns a failure Result for humidity outside `[0, 100]`.
- `analyze()` returns a failure Result for an unrecognized sensor type.
- `exportCSV()` writes the same number of rows as entries in history — no rows skipped.
- `getAverage()` returns a failure Result when history is empty.
- `poll()` transitions a sensor to error state after three consecutive zero readings.
- `poll()` returns a failure Result (does not throw) when called on a sensor in error state.
- `addSensor()` returns a failure Result when the sensor array is full.
- `processAll()` skips sensors in error state and logs each one.

### FR-TEST-4: Deterministic Simulation

The sensor value simulation used during testing **must** be seeded or replaceable with an injected value source so that test outcomes are deterministic across runs.

---

## 10. Build Requirements

- CMake minimum version: 3.10.
- C++ standard: C++14.
- The build **must** compile cleanly with `-Wall -Wextra` and zero warnings.
- A `test` CMake target **must** be defined that builds and runs the unit test suite.

---

## 11. Data Structures

### `reading`

| Field | Type | Description |
|---|---|---|
| `value` | `float` | Processed sensor value |
| `timestamp` | `time_t` | Unix timestamp at time of read |
| `valid` | `bool` | Whether the reading passed validation |
| `source` | `std::string` | Sensor name |

Note: `timestamp` **must** be typed as `time_t`, not `int`, to avoid truncation on platforms where `time_t` exceeds 32 bits.

### `sensor`

See FR-SM-2.

### `calibration`

| Field | Type | Description |
|---|---|---|
| `offset` | `float` | Additive calibration factor |
| `scale` | `float` | Multiplicative calibration factor |
| `calibrated` | `bool` | Whether calibration has been applied |

---

## 12. Named Constants (Required)

The following constants **must** be defined before any release build. All are currently represented as magic numbers in the codebase and **must** be replaced.

| Constant | Domain | Replaces |
|---|---|---|
| `MAX_SENSORS` | Sensor array capacity | `64` |
| `SENSOR_BUFFER_SIZE` | Per-sensor circular buffer capacity | `256` |
| `MAX_RETRIES` | Retries before error state | `3` |
| `TIMEOUT_MS` | Sensor recovery timeout (milliseconds) | `5000` |
| `TEMP_MIN_C` | Minimum valid temperature (°C) | `-40` |
| `TEMP_MAX_C` | Maximum valid temperature (°C) | `125` |
| `TEMP_WARN_F` | High-temperature warning threshold (°F) | `200` |
| `C_TO_F_SCALE` | Celsius-to-Fahrenheit scale factor | `1.8` |
| `C_TO_F_OFFSET` | Celsius-to-Fahrenheit offset | `32` |
| `BAR_TO_PSI` | Pressure unit conversion factor | `0.145038` |
| `PRESSURE_WARN_1_PSI` | First pressure warning threshold | `150` |
| `PRESSURE_WARN_2_PSI` | Second pressure warning threshold | `200` |
| `PRESSURE_CRITICAL_PSI` | Critical pressure threshold | `300` |

---

*End of Requirements Document*
