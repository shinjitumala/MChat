#include "LOG.hpp"

#ifndef _H_MS
#define _H_MS

#include <string>
#include <iostream>
#include <windows.h>
#include <queue>

using namespace std;

/**
 * Interface for classes that send strings to a target window.
 */
class Message_Sender {
public:
  /**
   * Will add the string to the message queue.
   * The contents of the queue will be sent at once when send() is called.
   */
  virtual void queue_message(string s) = 0;

  /**
   * Attempts to send queued strings to the target window.
   */
  virtual bool send() = 0;

  /**
   * Deconstuctor
   */
  virtual ~Message_Sender(){
    LOG("Message_Sender " << this << " >> delete.");
  };
};

class MS_Window : public Message_Sender {
protected:
  int m_input_delay; // the delay between inputs. in milliseconds.
  string m_window_name; // the window name to send the input to. the window that contains this name will be selected.
  string m_return_window_name; // the window name to return to at the end of send().
  int m_max_windows; // the maximum number of windows that the activate_window() method will look through. This is to avoid infinite alt tabbing.
  queue<string> m_message_queue; // message queue to send multiple messages in one alt tab operation.


public:
  /**
   * Constructor
   */
  MS_Window(int delay, int maxW, string WName, string ret_WName){
    LOG("MS_Window " << this << " >> new");
    m_input_delay = delay; // in milliseconds
    m_max_windows = maxW;
    m_window_name = WName;
    m_return_window_name = ret_WName;
  }

  /**
   * Queues a string to be sent to the window in the next operation.
   * string s: the string that is to be queued. (*WARNING*: read method send_string() for details.)
   */
  void queue_message(string s){
    LOG("MS_Window " << this << " >> queue_message(), Window: " << m_window_name << ", String: " << s);
    m_message_queue.push(s);
  }

  /**
   * Attempts to send strings that are queued as keyboard input to the window that contains "m_window_name"
   * in its title. Will attempt change the window to paycheck.exe after.
   * bool return value: returns false if failed. otherwise returns true
   */
  bool send(){
    if(m_message_queue.empty()) return true; // do nothing if queue is empty.
    LOG("MS_Window " << this << " >> send(), Window: " << m_window_name);
    bool ret = activate_window(m_window_name);
    if(ret){ // if success.
      while(!m_message_queue.empty()){
        Sleep(m_input_delay);
        send_string(m_message_queue.front());
        m_message_queue.pop();
      }
      Sleep(m_input_delay);
    }else{
      LOG("MS_Window " << this << " >> send() not found, Window: " << m_window_name);
    }
    activate_window(m_return_window_name);
    return ret;
  }

protected:
  /**
   * Sends a string as keyboard input.
   * *WARNING*: It's incomplete.
   * string s: The string to be send as keyboard input. (Can only accpets a handful of inputs.)
   */
  void send_string(string s){
    for(unsigned int i = 0; i < s.length() + 1; i++){
      if('a' <= s[i] && 'z' >= s[i]){
        send_key(s[i] - 32, false);
      }else if('A' <= s[i] && 'Z' >= s[i]){
        send_key(s[i], true);
      }else if(s[i] == '\n'){ // enter key.
        send_key(13, false);
      }else if(s[i] == '!'){
        send_key(49, true);
      }else if(s[i] == '@'){ // no mentioning
        //send_key(50, true);
      }else if(s[i] == '#'){
        send_key(51, true);
      }else if('$' == s[i]){
        send_key(52, true);
      }else if(s[i] == '%'){
        send_key(53, true);
      }else if(s[i] == '^'){
        send_key(54, true);
      }else if(s[i] == '&'){
        send_key(55, true);
      }else if(s[i] == '*'){
        send_key(56, true);
      }else if(s[i] == '('){
        send_key(57, true);
      }else if(s[i] == ')'){
        send_key(48, true);
      }else if('-' == s[i]){
        send_key(109, false);
      }else if('_' == s[i]){
        send_key(109, true);
      }else if('=' == s[i]){
        send_key(110, false);
      }else if('+' == s[i]){
        send_key(110, true);
      }else if('0' <= s[i] && '9' >= s[i]){
        send_key(s[i], false);
      }else if('`' == s[i]){
        send_key(192, false);
      }else if('~' == s[i]){
        send_key(192, true);
      }else if('[' == s[i]){
        send_key(219, false);
      }else if('{' == s[i]){
        send_key(219, true);
      }else if('\\' == s[i]){ // not working.
        send_key(220, false);
      }else if('|' == s[i]){
        send_key(220, true);
      }else if(']' == s[i]){
        send_key(221, false);
      }else if('}' == s[i]){
        send_key(221, true);
      }else if(';' == s[i]){
        send_key(186, false);
      }else if(':' == s[i]){
        send_key(186, true);
      }else if('\'' == s[i]){
        send_key(222, false);
      }else if('\"' == s[i]){
        send_key(222, true);
      }else if(',' == s[i]){
        send_key(188, false);
      }else if('<' == s[i]){
        send_key(188, true);
      }else if('.' == s[i]){
        send_key(190, false);
      }else if('>' == s[i]){
        send_key(190, true);
      }else if('/' == s[i]){
        send_key(191, false);
      }else if('?' == s[i]){
        send_key(191, true);
      }else if('\0' == s[i]){
        // do nothing.
      }else if(' ' == s[i]){
        send_key(32, false);
      }else{
        LOG("MS_Window " << this << " >> send_string(): invalid character: " << s[i] <<". skipped.");
      }

      Sleep(m_input_delay);
    }
  }

  /**
   * Sends a keypress.
   * unsigned char code: The code for the simulted keypress.
   * bool shift: Weather the key is pressed with the shift key or not.
   */
  void send_key(unsigned char code, bool shift){
    if(shift){
      keybd_event(160, 0x00, KEYEVENTF_EXTENDEDKEY | 0, 0);
      keybd_event(code, 0x00, KEYEVENTF_EXTENDEDKEY | 0, 0);
      Sleep(m_input_delay);
      keybd_event(code, 0x00, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
      keybd_event(160, 0x00, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
    }else{
      keybd_event(code, 0x00, KEYEVENTF_EXTENDEDKEY | 0, 0);
      keybd_event(code, 0x00, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
    }
  }

  /**
   * Simulates keypress to alt tab into the next window.
   * int count: how many windows to tab through.
   */
  void alt_tab(int count){
    keybd_event(18, 0x00, KEYEVENTF_EXTENDEDKEY | 0, 0);
    for(int i = 0; i < count; i++){
      keybd_event(9, 0x00, KEYEVENTF_EXTENDEDKEY | 0, 0);
      Sleep(m_input_delay);
      keybd_event(9, 0x00, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
      Sleep(m_input_delay);
    }
    keybd_event(18, 0x00, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
  }

  /**
   * Gets the window title of the current active window.
   * string return value: the name of the forground window. Will return a empty string if failed.
   */
  string get_foreground_window_name(){
    wchar_t wnd_title[128];
    HWND window = GetForegroundWindow();
    int length = GetWindowText(window, wnd_title, sizeof(wnd_title));
    if(length == 0) return "";
    wstring ws(wnd_title);
    string str(ws.begin(), ws.end());
    return str;
  }

  /**
   * Will attempt to alt tab to the window with the title that contains the given string.
   * string windowTitle: the window you will search for.
   * bool return value: will return false if failed. otherwise will return true.
   */
  bool activate_window(string windowTitle){
    string firstWindow = get_foreground_window_name();
    string currentWindow = firstWindow.c_str();
    int tabCount = 1;
    while(currentWindow.find(windowTitle) == std::string::npos){
      alt_tab(tabCount);
      tabCount++;
      Sleep(m_input_delay);
      currentWindow = get_foreground_window_name();
      if(firstWindow == currentWindow) return false;
      if(tabCount >= m_max_windows) return false;
    }
    return true;
  }
};

/**
 * A version of MS_Window that will send messages into sub-window of windows using Ctrl-Tab.
 */
class MS_Window_CT : public MS_Window {
protected:
  string m_sub_window_name;

public:
  MS_Window_CT(int delay, int maxW, string WName, string ret_WName, string sub_window_name) : MS_Window(delay, maxW, WName, ret_WName){
    LOG("MS_Window_CT " << this << " >> new");
    this->m_sub_window_name = sub_window_name;
  }

  /** Will send the queued messages to the desired window. */
  bool send(){
    if(m_message_queue.empty()) return true; // do nothing if queue is empty.
    LOG("MS_Window_CT " << this << " >> send(), Window: " << m_window_name << ", Sub window: " << m_sub_window_name);
    bool ret = activate_window(m_window_name);
    if(ret) ret = activate_sub_window();
    if(ret){ // if success.
      while(!m_message_queue.empty()){
        Sleep(m_input_delay);
        send_string(m_message_queue.front());
        m_message_queue.pop();
      }
      Sleep(m_input_delay);
    }else{
      LOG("MS_Window_CT " << this << " >> send() not found, Window: " << m_window_name << ", Sub window: " << m_sub_window_name);
    }
    activate_window(m_return_window_name);
    return ret;
  }
protected:
  /**
   * Uses Ctrl-tab to navigate through sub windows.
   * int count: the number of times to Ctrl-tab.
   */
  void ctrl_tab(int count){
    keybd_event(17, 0x00, KEYEVENTF_EXTENDEDKEY | 0, 0);
    for(int i = 0; i < count; i++){
      keybd_event(9, 0x00, KEYEVENTF_EXTENDEDKEY | 0, 0);
      Sleep(m_input_delay);
      keybd_event(9, 0x00, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
      Sleep(m_input_delay);
    }
    keybd_event(17, 0x00, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
  }

  /**
   * Will attempt to Ctrl-tab to the desired sub-window.
   */
  bool activate_sub_window(){
    string firstWindow = get_foreground_window_name();
    string currentWindow = firstWindow.c_str();
    int tabCount = 1;
    while(currentWindow.find(m_sub_window_name) == std::string::npos){
      ctrl_tab(1);
      tabCount++;
      Sleep(m_input_delay);
      currentWindow = get_foreground_window_name();
      if(firstWindow == currentWindow) return false;
      if(tabCount >= m_max_windows) return false;
    }
    return true;
  }
};

#endif
