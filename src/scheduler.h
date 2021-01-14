
#ifndef PARALLEL_TASK_SCHEDULER_SCHEDULER_H
#define PARALLEL_TASK_SCHEDULER_SCHEDULER_H

#include <iostream>
#include <stdio.h>
#include <thread>
#include <future>
#include <unordered_map>
#include <vector>
#include <queue>
#include <stack>
#include <queue>
#include <sstream>
#include <sys/time.h>
#include <mutex>
#include "core/ThreadPool.h"
#include "core/Task.h"
#include "core/NodeInfo.h"
#include "queues/skipListQueue.h"
#include <set>

using namespace std;



ThreadPool *pool = NULL;
mutex mtx;

bool finish = false;
atomic<int> work_remaining;

unordered_map<NodeInfo *, future<long>> task_results;
vector<NodeInfo *> nodes;
unordered_map<NodeInfo *, set<NodeInfo *>> parents;

skipListQueue<NodeInfo *> mainQ;

atomic<int> unique_id(1);
int get_unique_id(){
    return atomic_fetch_add(&unique_id, 1);

}

long thread_fun(NodeInfo *node);

void topo_sort(vector<Task *> &tasks, unordered_map<Task *, vector<Task *>> &DAG);

void update_topo_sort(vector<Task *> &tasks,
        unordered_map<Task *, vector<Task *>> &DAG, int root_id);

void precede(unordered_map<Task *, vector<Task *>> &DAG, Task *a, Task *b) {
    DAG[b].push_back(a);
}

void update_tree(NodeInfo *node) {
    lock_guard<mutex> lk(mtx);  // lock
    if (node->level == 0) {  // a level 0 task is done
        if (atomic_fetch_sub(&work_remaining, 1) == 1) {
            finish = true;
//            cout<<"BEGA RAFT"<< "\n";
        }

        return;
    }

    set<NodeInfo *> prt = parents[node];
    if (!prt.empty()) {

        for (NodeInfo *pa : prt) {
            if (atomic_fetch_sub(&(pa->n_dependency), 1) == 1) {
                mainQ.enqueue(pa->p_task->priority, pa);
//                cout << "height: " << mainQ.height() << endl;

            }
        }

        for (NodeInfo *pa : prt) {
            if (pa->n_dependency== 0) {
                NodeInfo *node = mainQ.dequeue();

                task_results[node] = pool->add_job(thread_fun, node);
                cout << "add task_" << node->p_task->id << endl;
            }
        }
    }
}

void  update_topo_sort(vector<Task *> &tasks, unordered_map<Task *, vector<Task *>> &DAG, int root_id) {
//    vector<NodeInfo *> new_nodes;
skipListQueue<NodeInfo *> new_nodes;

    unordered_map<Task *, int> in;

    NodeInfo *root;
    for (NodeInfo * n: nodes){
        if(n->p_task->id == root_id){
        root = n;
    }
    }
    int num_tasks = tasks.size() - 1;
    for (int i = 1; i <= num_tasks; i++) {
        in[tasks[i]] = 0;
        tasks[i]->priority = UINTV_MAX - ((unique_id + 1) * tasks[i]->priority + tasks[i]->id);
//        cout << "P S " << tasks[i]->id <<": "<<tasks[i]->priority  << endl;
    }

    for (auto it = DAG.begin(); it != DAG.end(); it++) {
        for (int i = 0; i < it->second.size(); i++) {
//cout << "DAG: " << it->first << "   " << it->second[i] << endl;
//cout << "DAG: " << it->first->id << "   " << it->second[i]->id << endl;

in[it->second[i]]++;

        }
    }


    queue<Task *> que;
    for (auto it = in.begin(); it != in.end(); it++) {
        if (it->second == 0) {
            que.push(it->first);
        }
    }

// mapping: Task -> NodeInfo
    unordered_map<Task *, NodeInfo *> map;
    for (int i = 1; i <= num_tasks; i++) {
        nodes.push_back(new NodeInfo(tasks[i]));
        map[tasks[i]] = nodes[nodes.size() - 1];
    }

    int level = root->level;
    while (!que.empty()) {
        int n = que.size();
        while (n--) {
            Task *now = que.front();
            que.pop();

            map[now]->level = level;
            if (level == 0)
                atomic_fetch_add(&work_remaining, 1);
            if (DAG[now].size() == 0 && n == que.size()) {    //fixme: ACHILLES
//                new_nodes.push_back(map[now]);
                new_nodes.enqueue(map[now]->p_task->priority, map[now]);
            }

            int cnt = 0;
            for (int i = 0; i < DAG[now].size(); i++) {
                in[DAG[now][i]]--;
                que.push(DAG[now][i]);
                parents[map[DAG[now][i]]].insert(map[now]);
                cnt++;
            }
            map[now]->n_dependency = cnt;
        }
        level++;

    }
//    for (NodeInfo *node : new_nodes) {
//        task_results[node] = pool->add_job(thread_fun, node);
//        cout << "adding_leaf_" << node->p_task->id << endl;
//    }
    while(!new_nodes.isEmpty()){
        NodeInfo * new_node = new_nodes.dequeue();
task_results[new_node] = pool->add_job(thread_fun, new_node);
//        cout << "adding_leaf_" << new_node->p_task->id << endl;
    }

    for (int i = 1; i <= num_tasks; i++) {

        if (parents[map[tasks[i]]].empty()) {
            for (NodeInfo *ni : parents[root]) {
                atomic_fetch_add(&(ni->n_dependency), 1);
            }
            parents[map[tasks[i]]] = parents[root];
        }
    }

//    return map;
}

void topo_sort(vector<Task *> &tasks, unordered_map<Task *, vector<Task *>> &DAG) {
    unordered_map<Task *, int> in;
    int num_tasks = tasks.size() - 1;
    for (int i = 1; i <= num_tasks; i++) {
        in[tasks[i]] = 0;
        tasks[i]->priority = UINTV_MAX - ((unique_id + 1) * tasks[i]->priority + tasks[i]->id);
//        cout << "P S " << tasks[i]->id <<": "<<tasks[i]->priority << endl;
    }

    for (auto it = DAG.begin(); it != DAG.end(); it++) {
        for (int i = 0; i < it->second.size(); i++) {
//            cout << "DAG: " << it->first->id << "   " << it->second[i]->id << endl;
            in[it->second[i]]++;
        }
    }

    queue<Task *> que;
    for (auto it = in.begin(); it != in.end(); it++) {
        if (it->second == 0) {
            que.push(it->first);
        }
    }

    unordered_map<Task *, NodeInfo *> map;
    for (int i = 1; i <= num_tasks; i++) {
        nodes.push_back(new NodeInfo(tasks[i]));
        map[tasks[i]] = nodes[nodes.size() - 1];
    }

    int level = 0;
    while (!que.empty()) {
        int n = que.size();
        while (n--) {
            Task *now = que.front();
            que.pop();

            map[now]->level = level;
            if (level == 0)
                atomic_fetch_add(&work_remaining, 1);
            if (DAG[now].size() == 0) {
                mainQ.enqueue(map[now]->p_task->priority, map[now]);
cout << "height: " << mainQ.height() << endl;
            }

            int cnt = 0;
            for (int i = 0; i < DAG[now].size(); i++) {
                in[DAG[now][i]]--;
//                if (in[DAG[now][i]] == 0) {
                que.push(DAG[now][i]);
                parents[map[DAG[now][i]]].insert(map[now]);

                cnt++;
//                }
            }
            map[now]->n_dependency = cnt;
        }
        level++;

    }
//    cout << "all nodes' info:" << endl;
//    for (int i = 0; i < nodes.size(); i++) {
//        cout << "node_" << nodes[i]->p_task->id << ": level=" << nodes[i]->level << " d=" << nodes[i]->n_dependency
//             << endl;
////        cout << "parent: ";
////        for (NodeInfo *pa: parents[nodes[i]])
////            cout << pa->p_task->id;
//
//        cout << endl;
//    }
//    cout << endl;

}

// dynamic task scheduling
void scheduling(int num_threads) {
    pool = new ThreadPool(num_threads);
    while (!mainQ.isEmpty()) {
        NodeInfo *node = mainQ.dequeue();

        task_results[node] = pool->add_job(thread_fun, node);
        cout << "add task_" << node->p_task->id << endl;
    }
}

// thread function
long thread_fun(NodeInfo *node) {
    //string rst = process_data(node);
    Task *node_task = node->p_task;

   long rst = node_task->task_function(node_task->id, node_task->start, node_task->stop);
    update_tree(node);
    return rst;
}

// thread function
void custom_free() {
    //string rst = process_data(node);
    delete pool;
}


#endif //PARALLEL_TASK_SCHEDULER_SCHEDULER_H