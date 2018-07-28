#include "My_Library\LOG.hpp"
#include "My_Library\Message_Handler.cpp"
#include "My_Library\Message_Sender.cpp"
#include "Parser.cpp"

#include <list>
#include <windows.h>
#include <pthread.h>

extern int UPDATE_INTERVAL;

/**
 * The base class for MChat. Manages all the Message_Sender and Message_Handler objects.
 * Also, will create a new thread and start sending update signals to them once start() is called.
 */
class MChat_Base {
private:
  list<Message_Handler*> m_MH_list;
  list<Message_Sender*> m_MS_list;
  volatile bool m_run;
  pthread_t m_thread;

public:
  /**
   * Constructor
   */
  MChat_Base(){
    m_run = true;
  }

  /**
   * Will attempt to start the operation. Program will exit if it fails.
   */
  void start(){
    LOG("MChat_Base " << this << " >> start(): Attempting to start Operation...");
    bool status = (pthread_create(&m_thread, NULL, run_helper, this) == 0);
    if(status){
      LOG("MChat_Base " << this << " >> start(): Operation start success.");
    }else{
      LOG("MChat_Base " << this << " >> start(): Operation start failed. Exiting program...");
      exit(1);
    }
  }

  /**
   * Will properly stop the operation. Also clears the Message_Handler and Message_Sender obects.
   */
  void stop(){
    m_run = false;
    (void) pthread_join(m_thread, NULL);


    for(auto itr = m_MH_list.begin(); itr != m_MH_list.end(); itr++){
      delete *itr;
    }
    for(auto itr = m_MS_list.begin(); itr != m_MS_list.end(); itr++){
      delete *itr;
    }
  }

  /**
   * Will wait for the operation end. It will never end unless stop() is called.
   * This method is used for debugging so that the main method ending does not kill the thread.
   */
  void wait(){
    (void) pthread_join(m_thread, NULL);
  }

private:
  /**
   * This method is run from the thread. After reading the config file will send update signals to all Message_Handler and
   * Message_Sender every UPDATE_INTERVAL miliseconds.
   */
  void run(){
    ifstream ss("config.txt");
    Main_Parser main = Main_Parser();
    main.parse(ss, m_MH_list, m_MS_list);

    Timer timer_clock = Timer(UPDATE_INTERVAL);
    while(m_run){
      for(auto itr = m_MH_list.begin(); itr != m_MH_list.end(); itr++){
        (**itr).update(timer_clock.get_tm());
      }
      for(auto itr = m_MS_list.begin(); itr != m_MS_list.end(); itr++){
        (**itr).send();
      }
      timer_clock.wait_next();
    }
  }

  /** Method used to call run() in the thread */
  static void* run_helper(void* This){
    ((MChat_Base*)This)->run();
    return NULL;
  }
};
