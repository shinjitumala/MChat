#include "LOG.hpp"

#ifndef _H_MH
#define _H_MH

#include <chrono>
#include <iostream>
#include <random>

#include "Message_Sender.cpp"
#include "Language.cpp"

#define QUANTUM_NUMBER 60 // an hour is divided into this number.

using namespace std;

/**
 * A data structure to take care of the weekly schedule.
 */
class Schedule{
  bool schedule_table[7][24 * QUANTUM_NUMBER]; // if this is set to true it means that the Message_Handler will send messages to the window.

public:
  /**
   * Constructor
   */
  Schedule(){
    for(int i = 0; i < 7; i++){
      for(int j = 0; j < 24 * QUANTUM_NUMBER; j++){
        schedule_table[i][j] = false; // default is false. meaning no sending messages.
      }
    }
    LOG("Schedule " << this << " >> new: " << 7 << " " << 24 * QUANTUM_NUMBER);
  }

  /**
   * Sets schedule.
   * int week: 0 = Mon, 1 = Tue, ... , 6 = Sun
   * int time: 0:00 - 0:14 = 0, 0:15 - 0:29 = 1, ... , 23:45 - 23:59 = 23 * 4
   * bool value: the value to set the schedule.
   */
  void set_schedule(int week, int time_frame, bool value){
    check(week, time_frame);
    schedule_table[week][time_frame] = value;
  }

  /**
   * Gets schedule.
   * int week: 0 = Sun, 1 = Mon, ... , 6 = Sat
   * int time: 0:00 - 0:14 = 0, 0:15 - 0:29 = 1, ... , 23:45 - 23:59 = 23 * 4
   * bool return value: the value of the schedule.
   */
  bool get_schedule(int week, int time_frame){
    return schedule_table[week][time_frame];
  }

private:
  void check(int week, int time_frame){
    if(week < 0 || 6 < week || time_frame < 0 || 24 * QUANTUM_NUMBER - 1 < time_frame){
      LOG("Schedule " << this << " >> check(): argument was out of range. week: " << week << ", time_frame: " << time_frame);
      exit(1);
    }
  }
};

//----------

/**
 * Interface for classes that recieves updates signals and
 * queue mesages to Message_Sender class objects if necessary.
 */
class Message_Handler {
protected:
  Schedule *m_schedule; // message sending schedule.
  Message_Sender *m_ms; // messanger for the window.
  int m_interval_min; // minimum message interval. in milliseconds. 300000 for 5 mins.
  int m_interval_max; // maximum message interval. in milliseconds. 300000 for 5 mins.
  int m_counter; // used to count how much time is left for the next message.
  int m_next_counter; // the randomly generated interval.
  tm m_previous; // used to store the tm when the previous call happened.
  mt19937 m_generator; // RNG used for the interval randomizer.

public:
  /**
   * Deconstuctor
   */
  virtual ~Message_Handler(){
    if(m_schedule != nullptr){
      delete m_schedule;
      m_schedule = nullptr;
    }
    LOG("Message_Handler " << this << " >> delete.");
  };

  /**
   * Takes tm of the current time. Will queue messages to its Message_Sender
   * if necessary.
   * tm *time: tm of the current time.
   */
  virtual void update(tm *time) = 0;

protected:
  /**
   * Updates the current counter. And will return true if it is time to queue a message.
   * tm *time: current time.
   * bool return: will return true if it is time to queue a message.
   */
  bool check_interval(tm *time){
    int d_day = ((time->tm_wday - (&m_previous)->tm_wday) % 7 + 7) % 7;
    int d_hour = ((time->tm_hour - (&m_previous)->tm_hour) % 24 + 24) % 24;
    int d_min = ((time->tm_min - (&m_previous)->tm_min) % 60 + 60) % 60;
    int d_sec = ((time->tm_sec - (&m_previous)->tm_sec) % 60 + 60 % 60);

    m_counter += d_day * 86400000 + d_hour * 3600000 + d_min * 60000 + d_sec * 1000;
    LOG("Word_Handler " << this << " >> counter: " << m_counter);

    if(m_counter > m_next_counter){
      m_counter = 0;
      set_next_counter();
      return true;
    }

    return false;
  }

  /**
   * Uses the RNG to set the next counter.
   */
  void set_next_counter(){
    m_next_counter = m_interval_min + m_generator() % (m_interval_max - m_interval_min);
    LOG("Word_Handler " << this << " >> set_next_counter(): " << m_next_counter);
  }
};

/**
 * Message_Handler for handling automatic typing.
 */
class Word_Handler : public Message_Handler{
protected:
  string m_message;

public:
  /**
   * Constructor
   */
  Word_Handler(Schedule *schedule, Message_Sender *ms, string message, int interval_min, int interval_max){
    m_schedule = schedule;
    m_ms = ms;
    m_message = message;
    m_interval_min = interval_min;
    m_interval_max = interval_max;
    m_counter = 999999999;
    time_t now = chrono::system_clock::to_time_t(chrono::system_clock::now());
    m_previous = *localtime(&now);
    set_next_counter();
    LOG("Word_Handler " << this << " >> new. Schedule: " << schedule << ", Message_Sender: " << ms);
  }

  /**
   * Deconstuctor
   */
  ~Word_Handler(){
    LOG("Word_Handler " << this << " >> delete.");
  };

  /**
   * Takes tm of the current time. Will queue messages to its Message_Sender
   * if necessary.
   * tm *time: tm of the current time.
   */
  void update(tm *time){
    LOG("Word_Handler " << this << " >> update()");
    if((*m_schedule).get_schedule(time->tm_wday, QUANTUM_NUMBER * time->tm_hour + time->tm_min / (60 / QUANTUM_NUMBER))){
      if(check_interval(time)){
        (*m_ms).queue_message(m_message);
      }
    }else{
      m_counter = 99999999; // resets timer when schedule is over.
    }

    m_previous = *time;
  }
};

/**
 * Message_Handler for generating text using Markov's chain.
 */
class Markov_Generator : public Message_Handler{
protected:
  Language *language;

public:
  /**
   * Constructor
   */
  Markov_Generator(Schedule *schedule, Message_Sender *ms, bool is_dictionary, string input_file_path, int interval_min, int interval_max){
    m_schedule = schedule;
    m_ms = ms;
    language = new Language();
    (*language).learn_file(input_file_path, is_dictionary);
    m_interval_min = interval_min;
    m_interval_max = interval_max;
    m_counter = 99999999;
    time_t now = chrono::system_clock::to_time_t(chrono::system_clock::now());
    m_previous = *localtime(&now);
    set_next_counter();
    LOG("Markov_Generator " << this << " >> new. Schedule: " << schedule << ", Message_Sender: " << ms);
  }

  /**
   * Deconstuctor
   */
  ~Markov_Generator(){
    LOG("Markov_Generator " << this << " >> delete.");
    delete language;
  };

  /**
   * Takes tm of the current time. Will queue messages to its Message_Sender
   * if necessary.
   * tm *time: tm of the current time.
   */
  void update(tm *time){
    LOG("Markov_Generator " << this << " >> update()");
    if((*m_schedule).get_schedule(time->tm_wday, QUANTUM_NUMBER * time->tm_hour + time->tm_min / (60 / QUANTUM_NUMBER))){
      if(check_interval(time)){
        (*m_ms).queue_message((*language).generate_sentence());
      }
    }else{
      m_counter = 99999999; // resets timer when schedule is over.
    }

    m_previous = *time;
  }
};

#endif
