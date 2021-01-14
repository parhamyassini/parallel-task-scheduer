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

using namespace std;

//#include <boost/program_options.hpp>
//namespace po = boost::program_options;


ThreadPool *pool = NULL;
mutex mtx;
bool finish = false;    // whether all tasks have been finished
unordered_map<NodeInfo *, future<string>> task_results;


vector<NodeInfo *> nodes;
unordered_map<NodeInfo *, vector<NodeInfo *>> tree;
unordered_map<NodeInfo *, NodeInfo *> parents;
// push node with no outcoming edges into queue. (sorting by node's level)
//priority_queue<NodeInfo *, vector<NodeInfo *>, cmp> mainQ;
//priority_queue<NodeInfo *, vector<NodeInfo *>, cmp> mainQ;
skipListQueue<NodeInfo *> mainQ;

//void initDAG(vector<Task*> &tasks, unordered_map<Task*, vector<Task*>> &DAG){
//    // create a DAG
//    int num_tasks = 8;
//
//    tasks.push_back(new Task(-1)); // dummy
//    for(int i = 1; i <= num_tasks; i++){
//        tasks.push_back(new Task(i));
//    }
//    // DAG[tasks[0]].push_back(tasks[1]);
//
//    DAG[tasks[1]].push_back(tasks[2]);
//    DAG[tasks[1]].push_back(tasks[3]);
//    DAG[tasks[1]].push_back(tasks[4]);
//    DAG[tasks[2]].push_back(tasks[5]);
//    DAG[tasks[2]].push_back(tasks[6]);
//    DAG[tasks[3]].push_back(tasks[4]);
//    DAG[tasks[3]].push_back(tasks[7]);
//    DAG[tasks[4]].push_back(tasks[7]);
//    DAG[tasks[4]].push_back(tasks[8]);
//}
void precede(unordered_map<Task *, vector<Task *>> &DAG, Task *a, Task *b) {
    DAG[b].push_back(a);
}

// data processing: merge all children's info
string process_data(NodeInfo *node, unordered_map<NodeInfo *, vector<NodeInfo *>> &tree) {
//    this_thread::sleep_for(chrono::seconds(1));
    long z = 0;
    for (int k = 0; k < 1000000000; k++) {
        z++;
    }
    string new_data;
    int n = tree[node].size();
    for (int i = 0; i < n; i++) {
        new_data += tree[node][i]->p_task->data + " ";
    }
    new_data += "x" + to_string(node->p_task->id);
    node->p_task->data = new_data;
    return new_data;
}

// declear thread function
string thread_fun(NodeInfo *node, unordered_map<NodeInfo *, vector<NodeInfo *>> &tree);

// update tree's info
void update_tree(NodeInfo *node, unordered_map<NodeInfo *, vector<NodeInfo *>> &tree) {
    lock_guard<mutex> lk(mtx);  // lock
    if (node->p_task->id == 1) {  // all tasks have done
        finish = true;
        return;
    }

    NodeInfo *prt = parents[node];
    prt->n_dependency--;
    if (prt->n_dependency == 0) {
        mainQ.insert(prt->p_task->id, prt);
//        mainQ.push(prt);
//        NodeInfo *node = mainQ.top();
//        mainQ.pop();
        NodeInfo *node = mainQ.pop();
        // task_results.emplace_back(
        //     pool->add_job(thread_fun, node, tree)
        // );
        task_results[node] = pool->add_job(thread_fun, node, tree);
        cout << "add task_" << node->p_task->id << endl;
    }
}

// thread function
string thread_fun(NodeInfo *node, unordered_map<NodeInfo *, vector<NodeInfo *>> &tree) {
    string rst = process_data(node, tree);
    update_tree(node, tree);
    return rst;
}

// topological sorting: convert DAG to tree-struction
unordered_map<NodeInfo *, vector<NodeInfo *>>
topo_sort(vector<Task *> &tasks, unordered_map<Task *, vector<Task *>> &DAG) {
// the number of incoming edges for each node
    unordered_map<Task *, int> in;

    int num_tasks = tasks.size() - 1;
    for (int i = 1; i <= num_tasks; i++) {
        in[tasks[i]] = 0;
    }

    for (auto it = DAG.begin(); it != DAG.end(); it++) {
        for (int i = 0; i < it->second.size(); i++) {
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

    unordered_map<NodeInfo *, vector<NodeInfo *>> tree;
    int level = 0;
    while (!que.empty()) {
        int n = que.size();
        while (n--) {
            Task *now = que.front();
            que.pop();

            map[now]->level = level;

            if (DAG[now].size() == 0) {
//mainQ.push(map[now]);
                mainQ.insert(map[now]->p_task->id, map[now]);
            }
//push to a new list, after while push to mainQ based on priority

            int cnt = 0;
            for (int i = 0; i < DAG[now].size(); i++) {
                in[DAG[now][i]]--;
                if (in[DAG[now][i]] == 0) {
                    que.push(DAG[now][i]);

                    tree[map[now]].push_back(map[DAG[now][i]]);
                    parents[map[DAG[now][i]]] = map[now];
                    cnt++;
                }
            }
            map[now]->n_dependency = cnt;
        }
        level++;

    }
//cout << "**********************************" <<endl;
//while (!mainQ.empty())
//{
//std::cout << mainQ.top()->p_task->id << "   "  << mainQ.top()->level<< "\n ";
//mainQ.pop();
//}
//std::cout << std::endl;
//cout << "**********************************" <<endl;
//exit(0);
// print nodes' info
    cout << "all nodes' info:" << endl;
    for (int i = 0; i < nodes.size(); i++) {
        cout << "node_" << nodes[i]->p_task->id << ": level=" << nodes[i]->level << " d=" << nodes[i]->n_dependency
             << endl;
    }
    cout << endl;

// print tree-struction
    cout << "tree-struction:" << endl;
    for (auto it = tree.begin(); it != tree.end(); it++) {
        for (int j = 0; j < it->second.size(); j++) {
            cout << it->first->p_task->id << " " << it->second[j]->p_task->id << endl;
        }
    }
    cout << endl;

    return tree;
}

// dynamic task scheduling
void scheduling(unordered_map<NodeInfo *, vector<NodeInfo *>> &tree, ThreadPool *pool) {
    while (!mainQ.isEmpty()) {
        NodeInfo *node = mainQ.pop();
        //NodeInfo *node = mainQ.top();
        //mainQ.pop();
        // task_results.emplace_back(
        //     pool->add_job(thread_fun, node, tree)
        // );
        task_results[node] = pool->add_job(thread_fun, node, tree);
        cout << "add task_" << node->p_task->id << endl;
    }
}


int main(int argc, char **argv) {

    int num_threads = thread::hardware_concurrency();   // default size of thread pool
    cout << "number of threads: " << num_threads << endl;

    // create a thread pool
    pool = new ThreadPool(num_threads);

    // init task dependency graph (DAG)
    vector<Task *> tasks;
    unordered_map<Task *, vector<Task *>> DAG;


    //===============================

    int num_tasks = 8;

    tasks.push_back(new Task(-1)); // dummy
    for (int i = 1; i <= num_tasks; i++) {
        tasks.push_back(new Task(i));
    }

//    tasks[8]->priority = 2;
    precede(DAG, tasks[2], tasks[1]);
    precede(DAG, tasks[3], tasks[1]);
    precede(DAG, tasks[4], tasks[1]);
    precede(DAG, tasks[5], tasks[2]);
    precede(DAG, tasks[6], tasks[2]);
    precede(DAG, tasks[4], tasks[3]);
    precede(DAG, tasks[7], tasks[3]);
    precede(DAG, tasks[7], tasks[4]);
    precede(DAG, tasks[8], tasks[4]);


    //===============================



//    initDAG(tasks, DAG);

    struct timeval start, end;
    double topolsort_dura, processtasks_dura;

    // get topological order (a tree struction)
    gettimeofday(&start, 0);
    tree = topo_sort(tasks, DAG);
    gettimeofday(&end, 0);
    topolsort_dura = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

    // task scheduling
    gettimeofday(&start, 0);
    scheduling(tree, pool);
    while (!finish) {}    // waiting for all tasks to be finished
    gettimeofday(&end, 0);
    processtasks_dura = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

    // print all tasks result
    cout << "\nresult:" << endl;
    for (int i = 0; i < nodes.size(); i++) {
        cout << "task_" << (i + 1) << ": " << task_results[nodes[i]].get() << endl;
    }

    // print execution time
    printf("\ntopological sorting execution time: %.2f s\n", topolsort_dura);
    printf("process all tasks execution time: %.2f s\n", processtasks_dura);

    delete pool;

    return 0;
}