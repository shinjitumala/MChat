#ifndef _H_LOG
#define _H_LOG

bool DEBUG = false;
#define LOG(x) if(DEBUG) {cerr << x << endl;};

#endif
