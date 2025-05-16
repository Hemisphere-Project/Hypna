#include <Timer.h>      // https://github.com/JChristensen/Timer
#include <Arduino.h>

// It binds the Timer class to FreeRTOS task
// and runs the update method in a separate task 
// to avoid blocking the main loop

class Ttimer : public Timer {
    public:
        Ttimer() {
            // Update task
            xTaskCreate(this->task,       // function
              "ktimer",                     // task name
              2000,                         // stack memory
              (void *)this,                 // args
              0,                            // priority
              NULL);                        // handler
        }

        static void task(void *p)
        {
            Ttimer *that = (Ttimer *)p;
            while (that->_running) {
                that->update();
                delay(1);
            }
            vTaskDelete(NULL);
        }

        void stop() {
            _running = false;
        }

    private :
        bool _running = true;
};