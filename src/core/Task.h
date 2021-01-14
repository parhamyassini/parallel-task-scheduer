#ifndef _TASK_H_
#define _TASK_H_

#include <functional>
#include <string>

using namespace std;

class Task{
public:
    Task(int _id) : id(_id) {}
    Task(int _id, long _start, long _stop) : id(_id), start(_start), stop(_stop){}
    Task(int _id, int _priority, function<long(int, int, int)> _task_func, long _start, long _stop) :
        id(_id), priority(_priority), task_function(_task_func), start(_start), stop(_stop) {}


    int id;
    int priority;
    long start;
    long stop;
    function<long(int, int, int)> task_function;

};

#endif
