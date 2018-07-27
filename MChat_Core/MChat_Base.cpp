#include "My_Library\LOG.hpp"
#include "My_Library\Message_Handler.cpp"
#include "My_Library\Message_Sender.cpp"
#include "Parser.cpp"

#include <list>
#include <windows.h>

extern int UPDATE_INTERVAL;

class MChat_Base {
private:
  list<Message_Handler*> m_MH_list;
  list<Message_Sender*> m_MS_list;
  bool run;

public:
  MChat_Base(){
    run = true;
  }

  void start(){
    LOG("MChat_Base " << this << " >> start(): Operation started.");
    ifstream ss("config.txt");
    Main_Parser main = Main_Parser();
    main.parse(ss, m_MH_list, m_MS_list);

    Timer timer_clock = Timer(UPDATE_INTERVAL);
    while(run){
      for(auto itr = m_MH_list.begin(); itr != m_MH_list.end(); itr++){
        (**itr).update(timer_clock.get_tm());
      }
      for(auto itr = m_MS_list.begin(); itr != m_MS_list.end(); itr++){
        (**itr).send();
      }
      timer_clock.wait_next();
    }
    LOG("MChat_Base " << this << " >> start(): Operation stopped.");
  }
};
