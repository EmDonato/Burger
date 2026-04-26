#ifndef TASK_HPP
#define TASK_HPP

#pragma once
#include <Arduino.h>
#include <STM32FreeRTOS.h>
#include <event_groups.h>
#include <initializer_list>
#include <array>
#include <cstddef> // for size_t
#include "utility.hpp"

// =============================================================
// === TaskSpec: Task configuration descriptor ===
// =============================================================
//
// This structure defines the basic parameters required to create
// a FreeRTOS task in a modular and portable way. Instead of calling
// xTaskCreate() directly with many parameters, we group them together
// into this struct.
//
// This makes task definitions much cleaner and easier to maintain.
//
struct TaskSpec {
  const char*  name;          // Descriptive name for the task (used in debugging)
  uint16_t     stackWords;    // Stack size in words (1 word = 4 bytes). 
  UBaseType_t  prio;          // Task priority (higher = more important).
  TickType_t   periodTicks;   // Task period in OS ticks (0 means non-periodic).
};

// =============================================================
// === TaskBase: Abstract base class for all RTOS tasks ===
// =============================================================
//
// This class provides a unified and extensible interface to define 
// real-time tasks in C++ on top of FreeRTOS.
//
// Instead of manually writing C-style task functions, each task 
// inherits from TaskBase and overrides the virtual methods:
//
//   - setup(): initialization code that runs once at startup.
//   - loop():  code executed periodically or continuously.
//   - wait():  synchronization logic (e.g. wait for state, event, or message).
//
// The design goal is to enforce **clean separation of concerns**:
// each task focuses only on its logic, while this base class handles
// the FreeRTOS creation, timing, and scheduling boilerplate.
//
// Two FreeRTOS Event Groups are passed by pointer to every task:
// - egEv_:   used for event signaling (e.g. new data available)
// - egCtrl_: used for FSM state control (e.g. enable/disable task)
// These are non-owning pointers — the memory is managed externally.
//
class TaskBase {

  protected:
    // ---------------------------------------------------------
    // === Attributes (protected to allow subclass access) ===
    // ---------------------------------------------------------

    TaskSpec              spec_;             // Task configuration (name, stack, priority, period)
    TaskHandle_t          handle_ = nullptr; // Handle returned by xTaskCreate(), needed for management

  public:
    // =============================================================
    // === Constructor ===
    // =============================================================
    //
    // Initializes all task parameters and stores references to the
    // shared synchronization primitives (EventGroups and TimeTable).
    //
    // Parameters:
    //  - s:                 task configuration (name, stack, priority, period)
    //  - egEvents:          pointer to EventGroup for events
    //  - egCtrl:            pointer to EventGroup for FSM state control
    //  - flags:             event/state bitmask used for task synchronization
    //  - timeTableHandle:   pointer to global timing table for watchdog supervision
    //  - entryTimeTable:    index in the watchdog table for this task
    //
    TaskBase(const TaskSpec& s)
    : spec_(s){}

    // =============================================================
    // === Destructor ===
    // =============================================================
    //
    // When the object is destroyed, if the task is active,
    // it deletes its associated FreeRTOS task to free resources.
    //
    virtual ~TaskBase() {
      if (handle_) vTaskDelete(handle_);
    }

    // =============================================================
    // === Begin() : Create and start the RTOS task ===
    // =============================================================
    //
    // This method is called once from setup() to create the underlying
    // FreeRTOS task. It uses 'entryThunk' as a static entry function
    // that redirects execution to the correct instance.
    //
    // Returns 'true' if the task was created successfully.
    //
    bool Begin() {
      return xTaskCreate(entryThunk,          // Static entry function
                        spec_.name,           // Task name
                        spec_.stackWords,     // Stack size (in words)
                        this,                 // 'this' passed as argument
                        spec_.prio,           // Task priority
                        &handle_) == pdPASS;  // Handle returned
    }

    // =============================================================
    // === Utility Accessors ===
    // =============================================================

    TaskHandle_t getHandle() { return handle_; }

    // Returns the remaining unused stack space for diagnostics
    UBaseType_t checkStack() const {
        if (handle_) {
            return uxTaskGetStackHighWaterMark(handle_);
        }
        return 0;
    }

  protected:
    // =============================================================
    // === Virtual Methods to Override in Derived Tasks ===
    // =============================================================

    // setup(): Runs once at task start. Used for initialization.
    virtual void setup()= 0;

    // loop(): Main body of the task. Runs continuously or periodically.
    virtual void loop() = 0;

    // wait(): Defines synchronization logic (e.g. wait for event or state).
    // Returns true when the task should execute its loop.
    virtual bool wait() = 0;


  private:
    // =============================================================
    // === Static Task Entry Function (entryThunk) ===
    // =============================================================
    //
    // This function acts as the "universal entry point" for all tasks.
    // FreeRTOS requires a C-style function pointer, so we use this static
    // method to recover the correct C++ instance (`this`) and call its logic.
    //
    // Inside the function:
    //  1. Call setup() once.
    //  2. Enter the main infinite loop.
    //  3. Depending on the periodTicks, either run at a fixed rate
    //     (using vTaskDelayUntil) or continuously.
    //  4. Each loop iteration:
    //       - wait() decides whether the task should proceed
    //       - loop() performs the actual work
    //
    // This structure guarantees both periodic and event-driven behavior
    // with minimal overhead and deterministic timing.
    //
    static void entryThunk(void* arg) {
      auto* self = static_cast<TaskBase*>(arg); // Recover the C++ object pointer

      self->setup(); // Run user-defined setup once before main loop

      // Case 1: Periodic task
      if (self->spec_.periodTicks > 0) {
        TickType_t next = xTaskGetTickCount(); // Initialize reference time
        for(;;) {
          if (self->wait()) {   // Wait logic (customizable per task)
            self->loop();       // Execute main task body
          }
          vTaskDelayUntil(&next, self->spec_.periodTicks); // Maintain fixed rate
        }

      // Case 2: Non-periodic (event-driven) task
      } else {
        for(;;) {
          if (self->wait()) {
            self->loop();
          }
        }
      }
    }
};

#endif
