#ifndef _NODE_INFO_H_
#define _NODE_INFO_H_

#include "Task.h"

class NodeInfo{
public:
    NodeInfo(){
        p_task = NULL;
        n_dependency = 0;
        level = 0;
        priority = 0;
    }

    NodeInfo(Task* task){
        p_task = task;
        n_dependency = 0;
        level = 0;
        priority = 0;
    }
    
    NodeInfo(Task* task, int _n_dependency, int _level){
        p_task = task;
        n_dependency = _n_dependency;
        level = _level;
        priority = 0;
    }

    NodeInfo(Task* task, int _n_dependency, int _level, int _priority){
        p_task = task;
        n_dependency = _n_dependency;
        level = _level;
        priority = _priority;
    }


    // bool operator<(const NodeInfo &node) const {
    //     return level < node.level;
    // }

    // bool operator>(const NodeInfo &node) const {
    //     return level > node.level;
    // }

public:
    Task *p_task;
    std::atomic<int> n_dependency;
    int level;
    int priority;

    inline bool operator==(const NodeInfo &pt){
        if(p_task->id == pt.p_task->id)
            return true;
        else
            return false;
    }
};


struct cmp{
    bool operator()(const NodeInfo *n1, const NodeInfo *n2){
//        if(n1->p_task->priority != n2->p_task->priority)
//            return n1->p_task->priority < n2->p_task->priority;
        return n1->level < n2->level;
    }
};

#endif
