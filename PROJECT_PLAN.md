# Forge & Fiber Sensor System — Project Timeline & Budget

**Document Version:** 1.0
**Status:** Closed — Delivered Late, Within Budget
**Target Delivery:** 2026-04-10
**Actual Delivery:** 2026-05-01
**Slip:** 3 weeks
**Owner:** Forge & Fiber Platform Engineering

---

## 1. Project Summary

| Item | Planned | Actual |
|---|---|---|
| Start Date | 2026-03-09 | 2026-03-09 |
| Delivery Date | 2026-04-10 | 2026-05-01 |
| Duration | 5 weeks | 8 weeks |
| **Schedule Variance** | — | **+3 weeks late** |
| Labor Budget | $27,000 | $21,100 |
| Hardware / Infrastructure | $0 (client-supplied) | $0 |
| **Total Budget** | **$27,000** | **$21,100** |
| **Budget Variance** | — | **−$5,900 (22% under)** |

Delivered late. Under budget. The slip was caused by two external blockers — hardware procurement and a Tech Lead reassignment — during which the developer had little to bill. Lower hours is the mechanical reason the project came in under budget despite running three weeks past its target date.

---

## 2. Team & Roles

| Role | Allocation | Rate | Notes |
|---|---|---|---|
| Senior C++ Engineer | Full-time (40 h/week planned) | $115/hr | All implementation — `SensorManager`, `DataProcessor`, `main.cpp`, CMake build |
| Safety Tech Lead | Part-time (~4.8 h/week planned) | $170/hr | Architecture review, style guide compliance, safety sign-off |

No QA engineer was staffed. Unit and integration test authorship was the responsibility of the Senior C++ Engineer per the requirements.

---

## 3. Phase Plan

### Phase 0 — Kickoff & Architecture Design (Week 1, Days 1–2)
**March 9 – March 10**

Requirements walkthrough with Safety Tech Lead. Reviewed the sensor domain model (`reading`, `sensor`, `calibration` structs), the two-class architecture (`SensorManager` / `DataProcessor`), the C++14 standard, and the CMake build target. Style guide reviewed — engineer acknowledged requirements for the Result pattern, Doxygen coverage, named constants, and the `>90%` unit test mandate.

Hardware clarification logged at kickoff: physical sensor test equipment (industrial pressure and temperature transducers) is the client's responsibility to provide for integration validation. Client confirmed delivery expected by end of Week 2.

---

### Phase 1 — Build System & Data Structures (Week 1)
**March 9 – March 13 | 5 days**

**Objective:** Compilable CMake project with all struct definitions and class skeletons in place.

| Task | Planned Hours | Actual Hours | Notes |
|---|---|---|---|
| `CMakeLists.txt` — project definition, C++14 standard, source list, `sensor_system` target | 2 | 2 | On estimate |
| `models` — `Rarity`, `reading`, `sensor`, `calibration` struct definitions in headers | 3 | 3 | On estimate |
| `sensor_manager.h` — class declaration, `MAX`, `TIMEOUT`, `THRESH` macro definitions, constructor signature | 3 | 4 | One extra hour: engineer flagged that `TIMEOUT` and `THRESH` should be `constexpr` per style guide; revised to macros as a temporary measure pending full compliance pass in Phase 5 |
| `data_processor.h` — class declaration, `calibration` member, history vector declaration | 2 | 2 | On estimate |
| Stub implementations — all `.cpp` files compile and link cleanly, all methods return placeholder values | 4 | 4 | On estimate |
| Safety Tech Lead review — architecture and header design | 4 | 2 | TL available only 2h this week due to competing commitments |

**Phase 1 Total — Planned: 18h / Actual: 17h**
**Status: On track.**

---

### Phase 2 — SensorManager Implementation (Week 2)
**March 16 – March 20 | 5 days**

**Objective:** Fully functional sensor registration, circular buffer, polling logic, and retry/error state.

| Task | Planned Hours | Actual Hours | Notes |
|---|---|---|---|
| `addSensor()` — sensor slot allocation, name/type assignment, 256-float buffer allocation (`new float[256]`) | 4 | 5 | One extra hour: engineer initially used `std::vector` for the buffer; revised to raw pointer array to match header declaration |
| `poll(id)` — `rand()` simulation, circular buffer write, zero-reading retry counter, error state transition after 3 retries | 8 | 9 | Exception-throw path on error state added here; noted internally as non-compliant with Result pattern requirement but deferred to Phase 5 compliance pass |
| `processAll()` — iterate active sensors, call `poll()`, forward reading to `DataProcessor::analyze()` | 4 | 4 | On estimate |
| `reset(id)` — clear error state, retries, buffer | 2 | 2 | On estimate |
| Global state — `globalErrorCount`, `globalLastTemp` declared at module level | 1 | 1 | On estimate |
| Safety Tech Lead review — SensorManager | 4 | 4 | On estimate. TL noted that `globalErrorCount` and `globalLastTemp` should be instance members; engineer acknowledged; deferred to Phase 5 |

**Phase 2 Total — Planned: 23h / Actual: 25h**
**Status: 2 hours over. Absorbed within weekly allocation. No milestone impact.**

---

### Phase 3 — DataProcessor Implementation (Weeks 3–4)
**March 23 – April 3 | 10 days planned**

**Objective:** Calibration application, per-type analysis with range checking, history accumulation, CSV export.

#### Week 3 — Hardware Delay Begins
**March 23 – March 27**

Client sensor hardware did not arrive as committed at end of Week 2. Without the hardware, the engineer could not validate temperature clamping behavior, pressure conversion accuracy against a known reference, or the sensor retry logic under real zero-read conditions. Engineer proceeded with `DataProcessor` implementation based on specification alone, but productivity was meaningfully reduced — time that would have been spent in iterative hardware validation loops was instead spent in code review and documentation drafting.

| Task | Planned Hours | Actual Hours | Notes |
|---|---|---|---|
| `applyCalibration()` — set offset, scale, mark calibrated | 2 | 2 | On estimate |
| `analyze()` — calibration gate, temperature path (clamp to [-40,125], convert to °F, warn >200°F) | 6 | 5 | Implemented; clamping is silent (no error returned on out-of-range); Result pattern not yet applied |
| `analyze()` — pressure path (× 0.145038 conversion, multi-level PSI warnings) | 4 | 3 | Under estimate; straightforward |
| `analyze()` — humidity path (0–100 bounds check, return -1 on violation) | 2 | 2 | On estimate |
| Begin Doxygen comment stubs on public interface | 3 | 0 | **Deferred.** Engineer spent planned Doxygen time attempting to build test fixture against simulated sensor without hardware reference. Work was non-billable. |
| Total billed this week | — | **14h** | 26h below the planned 40h due to hardware-blocked productivity loss |

#### Week 4 — Hardware Still Outstanding
**March 30 – April 3**

Hardware delivery slipped again. Client confirmed new ETA of April 7. Engineer completed remaining `DataProcessor` implementation tasks but continued operating without integration validation.

| Task | Planned Hours | Actual Hours | Notes |
|---|---|---|---|
| `getAverage()` — accumulate history, divide by count, return 0.0 on empty | 2 | 2 | On estimate. Returns `0.0f` for empty history rather than a failure Result; noted as non-compliant |
| `exportCSV()` — open file, write header, write history loop | 4 | 3 | Implemented with double-increment loop bug (loop counter incremented in both the `for` declaration and the loop body); not caught — no test coverage at this point |
| `main.cpp` — `DataProcessor` and `SensorManager` initialization, 10-iteration polling loop, exception catch-and-continue, final CSV export | 5 | 4 | Exception swallowing pattern used here; engineer noted it as a style guide violation and flagged for Phase 5 |
| Total billed this week | — | **12h** | Continued reduced billing. Engineer used remaining weekly time on other internal tasks not chargeable to this project. |

**Phase 3 Total — Planned: 28h / Actual: 26h across 2 weeks**
**Calendar impact: No additional slip yet. Hardware validation deferred to Week 5, compressing the compliance and testing phases.**

---

### Phase 4 — Hardware Integration Validation (Week 5)
**April 6 – April 10 | 5 days — original delivery week**

Hardware arrived April 7 (3 weeks behind commitment). Engineer spent the week validating sensor polling behavior and calibration math against real readings.

> This was the original delivery date. The project was not delivered on April 10.

| Task | Planned Hours | Actual Hours | Notes |
|---|---|---|---|
| Hardware environment setup and toolchain verification | 2 | 4 | One extra day: target platform compiler version differed from development environment; minor source compatibility fixes required |
| Validate temperature and pressure conversion accuracy | 4 | 6 | Pressure conversion factor (0.145038) confirmed accurate. Temperature clamping behavior confirmed functional but the silent-clamp issue was re-noted by engineer — no error surfaced when a sensor read 130°C; it was silently returned as 125°C. Flagged for Phase 5. |
| Validate retry/error-state behavior with intermittent sensor | 4 | 5 | Confirmed: 3 consecutive zero-reads correctly transitions sensor to error state. Also confirmed: calling `poll()` on a sensor in error state throws rather than returning a Result — non-compliant but functional for the current single-threaded use case. |
| Validate CSV export output | 2 | 3 | Engineer manually inspected CSV output. Noted that row count was approximately half of expected — did not identify the double-increment loop bug at this point; attributed discrepancy to the simulated sensor occasionally returning zeros (which do not enter history). **Bug not caught.** |

**Phase 4 Total — Planned: 12h / Actual: 24h**
**Calendar impact: Original delivery missed. Compliance, tests, and Doxygen still outstanding. Cumulative slip: 1 week.**

---

### Phase 5 — Style Guide Compliance, Tests & Doxygen (Week 6, Planned)
**April 13 – April 17**

**Objective:** Bring codebase into compliance with the Forge & Fiber Style Guide — named constants, Result pattern, Doxygen coverage, unit tests at ≥90%.

Safety Tech Lead conducted a full compliance review on Monday April 14. Findings were more extensive than anticipated.

**Safety Tech Lead review findings:**

| Finding | Requirement | Severity | Resolution |
|---|---|---|---|
| Exception thrown on `poll()` of error-state sensor | FR-ERR-1: Result pattern required | High | Partial — engineer revised to avoid throw in `processAll()`; direct `poll()` call still throws. Full Result pattern deferred to v1.1. |
| Exception caught and swallowed in `main.cpp` | FR-ERR-3: Fail Fast | High | Accepted risk; product owner approved for initial release. Comment added. |
| All threshold values are inline magic numbers | FR-STYLE-1: Named constants | Medium | Partial — engineer added `constexpr` constants for pressure thresholds (`PRESSURE_WARN_1_PSI` etc.) and temperature bounds. Conversion factors (`0.145038`, `1.8`, `32`) remain inline. |
| No Doxygen on any public symbol | FR-DOC-1: 100% coverage | Medium | Partial — engineer added `@brief` stubs to class declarations. Parameter and return documentation not completed. |
| `globalErrorCount`, `globalLastTemp` at module scope | FR-SM-7: No global state | Medium | Deferred to v1.1. |
| `DataProcessor` allocated with raw `new`, never deleted | FR-MEM-1/2: Smart pointers, no leaks | High | **Not caught during this review.** Would have been caught by Valgrind or test suite. |
| `TIMEOUT` macro declared, never enforced | FR-SM-5: Timeout must be implemented | Medium | Deferred to v1.1. |
| `pollRate` declared, never used | FR-SM-8: Poll rate must control timing | Medium | Deferred to v1.1. |
| `reading.timestamp` typed as `int`, not `time_t` | Section 11: Data Structures | Low | Deferred to v1.1. |
| `i++` in loop headers (should be `++i`) | FR-STYLE-3 | Low | Partially fixed. |

Following remediation of the High and partial Medium items, Safety Tech Lead issued a conditional sign-off: deliver with documented known deficiencies; full compliance required before v1.1.

**Phase 5 also included an attempt to integrate Google Test:**

The engineer allocated 12 hours to integrating `gtest` via CMake `FetchContent` and writing the required unit tests. After two days, the CMake dependency resolution was not building cleanly against the project's target toolchain. With the project already 1 week behind and the product owner unwilling to accept further slippage, the test suite was formally descoped on April 17.

**Descope decision — April 17:**
- Test suite deferred to v1.1.
- Google Test CMake integration to be resolved offline as a separate spike.
- No automated test coverage ships with v1.0.

| Task | Planned Hours | Actual Hours | Notes |
|---|---|---|---|
| Safety Tech Lead full review and findings documentation | 0 (rolling reviews planned) | 7 | Concentrated review was not originally planned as a discrete phase; budget impact |
| Engineer remediation of High and Medium findings | 8 | 14 | More items than estimated; partial compliance only |
| Google Test CMake integration and test authoring | 12 | 8 | Abandoned after 2 days; GTest `FetchContent` not resolving on target toolchain |
| Descope decision meeting | 0 | 1 | TL + engineer + product owner |

**Phase 5 Total — Planned: 20h dev + 0h TL / Actual: 22h dev + 7h TL**
**Cumulative slip: 2 weeks.**

---

### Phase 6 — Final Review & Delivery (Weeks 7–8)
**April 20 – May 1**

**Objective:** Resolve remaining open items, complete partial Doxygen, deliver build and documentation.

#### Week 7
**April 20 – April 24**

| Task | Actual Hours | Owner | Notes |
|---|---|---|---|
| Complete partial Doxygen — add `@param` and `@return` to at least 50% of public functions (accepted partial compliance) | 6 | Dev | |
| Fix `post-increment` → `pre-increment` in remaining loop headers | 1 | Dev | |
| Add `SENSOR_BUFFER_SIZE` and `MAX_SENSORS` as named `constexpr` constants | 2 | Dev | |
| Safety Tech Lead interim check-in | 2 | TL | Confirmed partial compliance acceptable for v1.0 delivery |

#### Week 8
**April 27 – May 1**

| Task | Actual Hours | Owner | Notes |
|---|---|---|---|
| Final build verification on target toolchain | 2 | Dev | |
| Known defects documentation — v1.1 backlog (see Section 6) | 2 | Dev | |
| Handoff package — source, CMake build instructions, v1.1 backlog | 2 | Dev | |
| Safety Tech Lead final sign-off | 2 | TL | |

**Phase 6 Total — Actual: 15h dev + 4h TL**
**Delivered May 1. Cumulative slip: 3 weeks.**

---

## 4. Milestone Summary

| Milestone | Target Date | Actual Date | Status |
|---|---|---|---|
| M0 — Architecture signed off, build skeleton compiles | March 13 | March 13 | ✓ On time |
| M1 — SensorManager complete and reviewed | March 20 | March 20 | ✓ On time |
| M2 — DataProcessor complete | April 3 | April 3 | ✓ On time |
| M3 — Hardware validation complete | March 27 | April 10 | ✗ 2 weeks late — hardware arrived April 7 |
| M4 — Style guide compliance and tests complete | April 10 | — | ✗ Tests descoped; partial compliance only |
| M5 — Final delivery | April 10 | May 1 | ✗ 3 weeks late |

---

## 5. Budget Detail

### 5.1 Planned Budget

| Role | Rate | Planned Hours | Planned Cost |
|---|---|---|---|
| Senior C++ Engineer | $115/hr | 200 h (40h × 5 weeks) | $23,000 |
| Safety Tech Lead | $170/hr | 24 h (~4.8h × 5 weeks) | $4,080 |
| **Planned Total** | | **224 h** | **$27,080** |

> Rounded to **$27,000** as the approved project ceiling.

### 5.2 Actual Budget

| Role | Rate | Actual Hours | Actual Cost |
|---|---|---|---|
| Senior C++ Engineer | $115/hr | 158 h | $18,170 |
| Safety Tech Lead | $170/hr | 17 h | $2,890 |
| **Actual Total** | | **175 h** | **$21,060** |

> Rounded to **$21,100** net of minor reimbursables. Under budget by **$5,900 (22%)**.

### 5.3 Developer Hours by Week

| Week | Dates | Billed Hours | Notes |
|---|---|---|---|
| Week 1 | March 9–13 | 34 h | Normal — project setup |
| Week 2 | March 16–20 | 32 h | Normal — SensorManager |
| Week 3 | March 23–27 | 14 h | Hardware delay; reduced productivity |
| Week 4 | March 30–April 3 | 12 h | Hardware still delayed |
| Week 5 | April 6–10 | 24 h | Hardware arrived; integration validation |
| Week 6 | April 13–17 | 22 h | Compliance remediation + GTest attempt |
| Week 7 | April 20–24 | 16 h | Doxygen, final named constants |
| Week 8 | April 27–May 1 | 4 h | Build verification, handoff docs |
| **Total** | | **158 h** | |

### 5.4 Tech Lead Hours by Phase

| Phase | Activity | Hours |
|---|---|---|
| Phase 0 | Kickoff and architecture review | 2 |
| Phase 1 | Header design review | 2 |
| Phase 2 | SensorManager review | 4 |
| Phase 5 | Full compliance review and findings | 7 |
| Phase 6 | Interim check-in + final sign-off | 2 |
| **Total** | | **17 h** |

### 5.5 Schedule Variance Summary

| Delay | Weeks Lost | Root Cause |
|---|---|---|
| Hardware procurement | +2 weeks | Client-supplied test equipment arrived 3 weeks after the project start date, 2 weeks after committed delivery to engineer. Weeks 3–4 developer billing dropped to 26h combined. |
| Test suite descoped, additional compliance remediation | +1 week | Google Test CMake integration failed on target toolchain; 2 days lost before descope decision. Compliance remediation took longer than planned. |
| **Total Slip** | **+3 weeks** | |

---

## 6. Deferred to v1.1

The following items were not delivered in v1.0 and must be completed before the system is considered fully compliant with the Forge & Fiber Style Guide and engineering requirements.

| Item | Requirement | Risk if Unresolved |
|---|---|---|
| Unit and integration test suite (≥90% line coverage) | FR-TEST-1 through FR-TEST-4 | Multiple defects listed below would have been caught by a test suite. |
| Memory leak — `DataProcessor` allocated with `new` in `main.cpp`, never deleted | FR-MEM-1/2 | Process heap growth in long-running deployments; resource exhaustion on constrained hardware |
| CSV export double-increment bug — loop counter incremented twice per iteration, skipping every other reading | FR-DP-8 | Exported CSV contains ~50% of actual readings; downstream analysis on incomplete data |
| `TIMEOUT_MS` enforced in polling logic | FR-SM-5 | Sensors that go silent are never automatically recovered; requires manual `reset()` call |
| `pollRate` wired to actual loop timing | FR-SM-8 | Main loop runs as fast as the CPU allows; no deterministic sampling interval |
| Result pattern replacing sentinel returns and exception control flow | FR-ERR-1 through FR-ERR-4 | Silent failures in `analyze()`, `getAverage()`, and `exportCSV()` — callers cannot distinguish error from valid zero |
| `globalErrorCount` and `globalLastTemp` moved to instance scope | FR-SM-7 | Not thread-safe; undefined behavior if `SensorManager` is used concurrently |
| Remaining inline magic numbers (`0.145038`, `1.8`, `32`) replaced with named constants | FR-STYLE-1 | Maintenance risk; conversion factor change requires source search |
| 100% Doxygen coverage with `@param` / `@return` on all public functions | FR-DOC-1 through FR-DOC-4 | Partial documentation only; `@brief` stubs exist on class declarations, not function signatures |
| `reading.timestamp` changed from `int` to `time_t` | Section 11 | Overflow on 64-bit platforms where `time_t` exceeds 32 bits after 2038 |

---

## 7. Retrospective Notes

**Why the project was late but under budget:**
The developer billed only 26 combined hours across Weeks 3 and 4 because there was genuinely nothing to bill — hardware validation is a significant portion of the implementation work for a sensor system, and the client's hardware was 3 weeks late. When the developer is blocked waiting on a client dependency and cannot productively advance the work, billing stops. The calendar stretched but the invoice stayed low.

**What to address in future projects:**

- **Client hardware commitments must be a contractual dependency with a timeline impact clause.** The 3-week hardware slip was the direct cause of the entire schedule overrun. Had the contract included a clause adjusting the delivery date by any hardware procurement delay, the April 10 deadline would have been revised to April 24 on Day 1 of Week 3 — and the May 1 actual delivery would have been only 1 week late instead of 3.

- **The test suite and compliance pass should not share a phase.** Compressing style-guide compliance, Doxygen, and 90% test coverage into a single 5-day phase was not realistic. These are each multi-day efforts. Future projects of this scope should allocate a dedicated test week and a dedicated compliance week separately.

- **A failing CMake test integration should be escalated immediately, not abandoned after 2 days.** The GTest `FetchContent` issue was a build system problem that a 1-hour consultation with a CMake expert might have resolved. Instead it consumed 2 days and ultimately caused a full test descope that left the memory leak and CSV export bug undetected.

- **Manual CSV inspection is not a substitute for an automated output test.** The engineer noted the row count discrepancy during hardware validation but attributed it to sensor zeros rather than investigating the loop. An automated test that writes N readings and asserts N CSV rows would have caught the double-increment bug in minutes.

---

*End of Project Plan*
