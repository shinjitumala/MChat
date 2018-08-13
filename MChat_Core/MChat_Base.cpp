#include "My_Library\LOG.hpp"
#include "My_Library\Message_Handler.cpp"
#include "My_Library\Message_Sender.cpp"
#include "Parser.cpp"

#include <list>
#include <pthread.h>
#include <windows.h>

extern int UPDATE_INTERVAL;

/**
 * The base class for MChat. Manages all the Message_Sender and Message_Handler
 * objects. Also, will create a new thread and start sending update signals to
 * them once start() is called.
 */
class MChat_Base {
private:
  list<Message_Handler *> m_MH_list;
  list<Message_Sender *> m_MS_list;
  list<Schedule *> m_Schedule_list;
  volatile bool m_run;
  volatile bool m_ready;
  pthread_t m_thread;

public:
  /**
   * Constructor
   */
  MChat_Base() {
    m_run = true;
    m_ready = false;
  }

  /**
   * Will attempt to start the operation. Program will exit if it fails.
   */
  void start() {
    m_run = true;
    m_ready = false;
    LOG("MChat_Base " << this
                      << " >> start(): Attempting to start Operation...");
    bool status = (pthread_create(&m_thread, NULL, run_helper, this) == 0);
    if (status) {
      LOG("MChat_Base " << this << " >> start(): Operation start success.");
    } else {
      LOG("MChat_Base "
          << this << " >> start(): Operation start failed. Exiting program...");
      exit(1);
    }
  }

  /**
   * Will properly stop the operation. Also clears the Message_Handler and
   * Message_Sender obects.
   */
  void stop() {
    m_run = false;
    m_ready = false;
    (void)pthread_join(m_thread, NULL);

    // dellocate memory
    for (auto itr = m_MH_list.begin(); itr != m_MH_list.end(); itr++) {
      delete *itr;
    }
    for (auto itr = m_MS_list.begin(); itr != m_MS_list.end(); itr++) {
      delete *itr;
    }
    for (auto itr = m_Schedule_list.begin(); itr != m_Schedule_list.end();
         itr++) {
      delete *itr;
    }
    m_MH_list.clear();
    m_MS_list.clear();
    m_Schedule_list.clear();
  }

  /**
   * Will wait for the operation end. It will never end unless stop() is called.
   * This method is used for debugging so that the main method ending does not
   * kill the thread.
   */
  void wait() { (void)pthread_join(m_thread, NULL); }

  /**
   * Gets the progress of a certain Message_Handler
   * int id: gets the Message_Handler specified by this id
   * double return: the progress
   */
  double get_progress(int id) {
    int i = 0;
    for (auto itr = m_MH_list.begin(); itr != m_MH_list.end(); itr++) {
      if (id == i)
        return (**itr).get_progress();
      i++;
    }
  }

  /** gets the number of Message_Handler objects */
  int get_MH_count() { return m_MH_list.size(); }

  /** wait until ready */
  void wait_until_ready() {
    while (!m_ready) {
      Sleep(1000);
    }

    return;
  }

private:
  /**
   * This method is run from the thread. After reading the config file will send
   * update signals to all Message_Handler and Message_Sender every
   * UPDATE_INTERVAL miliseconds.
   */
  void run() {
    ifstream ss("config.txt");
    Main_Parser main = Main_Parser();
    main.parse(ss, m_MH_list, m_MS_list, m_Schedule_list);

    m_ready = true;
    Timer timer_clock = Timer(UPDATE_INTERVAL);
    while (m_run) {
      for (auto itr = m_MH_list.begin(); itr != m_MH_list.end(); itr++) {
        (**itr).update(timer_clock.get_tm());
      }
      for (auto itr = m_MS_list.begin(); itr != m_MS_list.end(); itr++) {
        (**itr).send();
      }
      timer_clock.wait_next();
    }
  }

  /** Method used to call run() in the thread */
  static void *run_helper(void *This) {
    ((MChat_Base *)This)->run();
    return NULL;
  }
};
