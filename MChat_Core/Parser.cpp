#include "My_Library\LOG.hpp"
#include "Timer.cpp"

#ifndef _H_PARSER
#define _H_PARSER

#include "My_Library\Message_Handler.cpp"
#include "My_Library\Message_Sender.cpp"

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <list>
#include <unordered_map>

using namespace std;

int INPUT_DELAY = 80;
int MAX_WINDOWS = 8;
string RETURN_WINDOW_NAME = "MChat";
int UPDATE_INTERVAL = 10000;


/**
 * Class that is responsible for reading the config file and setting up the list of
 * Message_Sender and Message_Handler objects for the MChat_Base class
 */
class Main_Parser{
private:
  unordered_map<string, Message_Sender*> MS_map;
public:
  /**
   * Takes a file input stream and reads it. Will build the list of Message_Sender
   * and Message_Handler objects.
   * ifstream& ss: File input stream of the config file.
   * list<Message_Handler*>& MH_list: List of Message_Handler objects to be built.
   * list<Message_Sender*>& MS_list: List of Message_Sender objects to be built.
   */
  void parse(ifstream& ss, list<Message_Handler*>& MH_list, list<Message_Sender*>& MS_list){
    string line;
    Schedule *current_schedule = new Schedule();
    while(getline(ss, line)){
      if(!(line[0] == '/' && line[1] == '/') && !(line[0] == '\r') && !(line[0] == '\n')){ // skip line if it starts with "//" or is a empty line.
        switch(line[0]){
          case '>':
            parse_MH(ss, current_schedule, MH_list, MS_list);
            break;
          case '{':
            parse_schedule(ss, current_schedule);
            current_schedule = new Schedule();
            break;
          case 'g':
            parse_global(line);
            break;
          default:
            LOG("Main_Parser " << this << " >> parse(): Unknown line \"" << line << "\" Exiting program...");
            exit(1);
        }
      }
    }
    LOG("Main_Parser " << this << " >> parse(): Finished parsing.");
  }

private:
  void parse_MH(ifstream& ss, Schedule *current_schedule, list<Message_Handler*>& MH_list, list<Message_Sender*>& MS_list){
    string line;
    if(getline(ss, line)){
      if(line == "MH_AUTO"){
        parse_MH_AUTO(ss, current_schedule, MH_list, MS_list);
        return;
      }else if(line == "MH_MARKOV"){
        parse_MH_MARKOV(ss, current_schedule, MH_list, MS_list);
        return;
      }
    }
    LOG("Main_Parser " << this << " >> parse_MH(): Error at line \"" << line << "\" Exiting program...");
    exit(1);
  }

  void parse_MH_AUTO(ifstream& ss, Schedule *current_schedule, list<Message_Handler*>& MH_list, list<Message_Sender*>& MS_list){
    string line, message;
    int min, max;
    Message_Sender* ms;
    Message_Handler* mh;

    if(getline(ss, line)){
      message = line;
      message.append("\n");
    }else{
      goto error;
    }

    if(getline(ss, line)){
      stringstream ss(line);
      ss >> min >> max;
      if(ss.fail()) goto error;
    }else{
      goto error;
    }

    if(getline(ss, line)){
      if(line == "MS_STANDARD"){
        if(!getline(ss, line)) goto error;
        ms = add_MS(line, MS_list);
      }else if(line == "MS_CT"){
        if(!getline(ss, line)) goto error;
        string tmp;
        if(!getline(ss, tmp)) goto error;
        ms = add_MS(line, tmp, MS_list);
      }
    }

    if(!getline(ss, line)) goto error;
    if(!(line == "<")) goto error;

    mh = new Word_Handler(current_schedule, ms, message, min, max);

    MH_list.push_back(mh);

    return;

  error:
    LOG("Main_Parse " << this << " >> parse_MH_AUTO(): Error at line \"" << line << "\" Exiting program...");
    exit(1);
  }

  void parse_MH_MARKOV(ifstream& ss, Schedule *current_schedule, list<Message_Handler*>& MH_list, list<Message_Sender*>& MS_list){
    string line, message, path;
    int min, max;
    Message_Sender* ms;
    Message_Handler* mh;
    bool dictionary;

    if(getline(ss, line)){
      stringstream ss(line);
      string tmp;
      ss >> tmp >> path;

      if(ss.fail()) goto error;

      if(tmp == "TRUE"){
        dictionary = true;
      }else if(tmp == "FALSE"){
        dictionary = false;
      }else{
        goto error;
      }
    }else{
      goto error;
    }

    if(getline(ss, line)){
      stringstream ss(line);
      ss >> min >> max;
      if(ss.fail()) goto error;
    }else{
      goto error;
    }

    if(getline(ss, line)){
      if(line == "MS_STANDARD"){
        if(!getline(ss, line)) goto error;
        ms = add_MS(line, MS_list);
      }else if(line == "MS_CT"){
        if(!getline(ss, line)) goto error;
        string tmp;
        if(!getline(ss, tmp)) goto error;
        ms = add_MS(line, tmp, MS_list);
      }
    }

    if(!getline(ss, line)) goto error;
    if(!(line == "<")) goto error;

    mh = new Markov_Generator(current_schedule, ms, dictionary, path, min, max);

    MH_list.push_back(mh);

    return;

  error:
    LOG("Main_Parse " << this << " >> parse_MH_MARKOV(): Error at line \"" << line << "\" Exiting program...");
    exit(1);
  }

  Message_Sender* add_MS(string window_name, list<Message_Sender*>& MS_list){
    Message_Sender* ret;
    try{
      ret = MS_map.at(window_name);
    }catch(out_of_range&){
      Message_Sender *new_MS = new MS_Window(INPUT_DELAY, MAX_WINDOWS, window_name, RETURN_WINDOW_NAME);
      MS_list.push_front(new_MS);
      MS_map.emplace(window_name, new_MS);
      ret = new_MS;
    }

    return ret;
  }

  Message_Sender* add_MS(string window_name, string sub_window_name, list<Message_Sender*>& MS_list){
    Message_Sender* ret;
    string key;

    // create custom key for CT.
    key = "MChat MS_CT ";
    key.append(window_name);
    key.append(" ");
    key.append(sub_window_name);

    try{
      ret = MS_map.at(key);
    }catch(out_of_range&){
      Message_Sender *new_MS = new MS_Window_CT(INPUT_DELAY, MAX_WINDOWS, window_name, RETURN_WINDOW_NAME, sub_window_name);
      MS_list.push_front(new_MS);
      MS_map.emplace(key, new_MS);
      ret = new_MS;
    }

    return ret;
  }

  /**
   * Parses the input for setting up schedules.
   * ifstream& ss: file input stream
   * Schedule *schedule: the schedule that is to be built
   */
  void parse_schedule(ifstream& ss, Schedule *schedule){
    string line;
    int current_day_of_week;
    while(getline(ss, line)){
      if(!(line[0] == '/' && line[1] == '/') || line[0] == '\n'){// skips line if it starts with "//" or "\n".
        switch(line[0]){
          case '+': // get day
            current_day_of_week = get_day_of_week(line);
            break;
          case '=': // get time
            set_time(line, current_day_of_week, schedule);
            break;
          case '}': // finish parsing schedule
            return;
            break;
          default:
            LOG("Main_Parser " << this << " >> parse_schedule(): Unknown line \"" << line << "\" Exiting program...");
            break;
        }
      }
    }
  }

  /**
   * Gets the day of a week from the given line.
   * string line: given line
   * int return: the day of the week. Sunday is 0, Saturday is 6
   */
  int get_day_of_week(string line){
    if(line == "+ Su"){
      return 0;
    }else if(line == "+ Mo"){
      return 1;
    }else if(line == "+ Tu"){
      return 2;
    }else if(line == "+ We"){
      return 3;
    }else if(line == "+ Th"){
      return 4;
    }else if(line == "+ Fr"){
      return 5;
    }else if(line == "+ Sa"){
      return 6;
    }else{
      LOG("Main_Parser " << this << " >> get_day_of_week(): Unknown line \"" << line << "\"");
      return -1;
    }
  }

  /**
   * Reads a line and sets up the time for a schedule.
   * string line: given line.
   * int current_day_of_week: the current day of week to set the time.
   * Schedule *schedule: the schedule to be edited.
   */
  void set_time(string line, int current_day_of_week, Schedule *schedule){
    if(current_day_of_week == -1 || schedule == NULL){ // exit if error is detected.
      LOG("Main_Parser " << this << " >> set_time(): Error at line \"" << line << "\" Exiting program...");
      exit(1);
    }
    stringstream ss(line);
    int s_hour, s_minute, e_hour, e_minute;
    char dummy;
    ss >> dummy >> s_hour >> dummy >> s_minute >> dummy >> e_hour >> dummy >> e_minute;
    s_hour = s_hour % 24; e_hour = e_hour % 24; s_minute = s_minute % 60; e_minute = e_minute % 60;

    int start, end;
    start = s_hour * QUANTUM_NUMBER + s_minute / (60 / QUANTUM_NUMBER);
    end = e_hour * QUANTUM_NUMBER + e_minute / (60 / QUANTUM_NUMBER);
    for(int i = start; i < end; i++){
      (*schedule).set_schedule(current_day_of_week, i, true);
    }
  }

  void parse_global(string line){
    stringstream ss(line);
    string token;
    int tmp;

    ss >> token;
    if(ss.fail()) goto error;
    if(!(token == "global")) goto error;

    ss >> token;
    if(ss.fail()) goto error;
    if(token == "input_delay"){
      ss >> tmp;
      if(ss.fail()) goto error;
      INPUT_DELAY = tmp;
    }else if(token == "max_windows"){
      ss >> tmp;
      if(ss.fail()) goto error;
      MAX_WINDOWS = tmp;
    }else if(token == "update_interval"){
      ss >> tmp;
      if(ss.fail()) goto error;
      UPDATE_INTERVAL = tmp;
    }else if(token == "return_window_name"){
      ss >> token;
      if(ss.fail()) goto error;
      RETURN_WINDOW_NAME = token;
    }else{
      goto error;
    }
    return;

  error:
    LOG("Main_Parser " << this << " >> parse_global: Error at line \"" << line << "\"");
    exit(1);
  }
};

#endif
