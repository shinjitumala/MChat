#ifndef _H_Timer
#define _H_Timer

#include <chrono>
#include <windows.h>

using namespace std;

/**
 * Acts as the central control for all update operations.
 */
class Timer{
  time_t now_c;
  int wait_time; // the time to wait between updates. in milliseconds. recommended to set it above 60000 (60 sec).

public:
  /**
   * Constructor
   */
  Timer(int wait){
    update();
    wait_time = wait;
  }

  /**
   * Get the tm for the current time.
   * tm return value: the tm of the current time.
   */
  tm *get_tm(){
    update();
    return localtime(&now_c);
  }

  /**
   * Waits until the next update cycle.
   */
  void wait_next(){
    Sleep(wait_time);
  }
private:
  /**
   * Updates the time with the current time.
   */
  void update(){
    now_c = chrono::system_clock::to_time_t(chrono::system_clock::now());
  }
};

#endif
