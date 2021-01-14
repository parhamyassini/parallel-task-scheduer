#include <iostream>
#include <stdio.h>
#include "scheduler.h"
#include <functional>
//#include "common/graph.h"

long dynamic_data(int id, int start, int stop){

    cout << "Dynamic "<<id << endl ;

    return start;
}

long process_data(int id, int start, int stop) {

   this_thread::sleep_for(chrono::seconds(1));

   if(start == 50){

        vector<Task *> tasks;
       unordered_map<Task *, vector<Task *>> DAG;

       tasks.push_back(new Task(-1)); // dummy
       for (int i = 9; i <= 14; i++) {
           int x = get_unique_id();
           tasks.push_back(new Task(x, 1, dynamic_data, 10*x, 50*x));
       }


       tasks[6]->priority = 5;
       tasks[1]->priority = 9;
       precede(DAG, tasks[6], tasks[2]);
       precede(DAG, tasks[4], tasks[2]);
       precede(DAG, tasks[5], tasks[2]);
       precede(DAG, tasks[5], tasks[4]);



       update_topo_sort(tasks, DAG, id);

   }

   return start;
}

int main(int argc, char **argv) {

//    int num_threads = thread::hardware_concurrency();   // default size of thread pool
    int num_threads = 2;   // default size of thread pool
    cout << "number of threads: " << num_threads << endl;
    // create a thread pool
//    pool = new ThreadPool(num_threads);

    // init task dependency graph (DAG)
    vector<Task *> tasks;
    unordered_map<Task *, vector<Task *>> DAG;


    //===============================

    int num_tasks = 8;

    tasks.push_back(new Task(-1)); // dummy
    for (int i = 1; i <= num_tasks; i++) {
        int x = get_unique_id();
        tasks.push_back(new Task(x, 1, process_data,10*x, 50*x));
    }
//
//    tasks[1]->priority = 9;
//    tasks[5]->priority = 10;
//    tasks[10]->priority = 15;
    precede(DAG, tasks[2], tasks[1]);
    precede(DAG, tasks[3], tasks[1]);
    precede(DAG, tasks[4], tasks[1]);
    precede(DAG, tasks[5], tasks[2]);
    precede(DAG, tasks[6], tasks[2]);
    precede(DAG, tasks[4], tasks[3]);
    precede(DAG, tasks[7], tasks[3]);
    precede(DAG, tasks[7], tasks[4]);
    precede(DAG, tasks[8], tasks[4]);

//    precede(DAG, tasks[2], tasks[1]);
//    precede(DAG, tasks[3], tasks[1]);
//    precede(DAG, tasks[4], tasks[1]);
////    precede(DAG, tasks[5], tasks[2]);
//    precede(DAG, tasks[6], tasks[2]);
//    precede(DAG, tasks[4], tasks[3]);
//    precede(DAG, tasks[7], tasks[3]);
//    precede(DAG, tasks[7], tasks[4]);
//    precede(DAG, tasks[8], tasks[4]);
//    precede(DAG, tasks[2], tasks[5]);
//    precede(DAG, tasks[3], tasks[5]);
//    precede(DAG, tasks[4], tasks[5]);
//
//    precede(DAG, tasks[2], tasks[9]);
//    precede(DAG, tasks[3], tasks[9]);
//    precede(DAG, tasks[4], tasks[9]);
//    precede(DAG, tasks[2], tasks[10]);
//    precede(DAG, tasks[3], tasks[10]);
//    precede(DAG, tasks[4], tasks[10]);


    //===============================



//    initDAG(tasks, DAG);

    struct timeval start, end;
    double topolsort_dura, processtasks_dura;

    // get topological order (a tree struction)
    gettimeofday(&start, 0);
    topo_sort(tasks, DAG);
    gettimeofday(&end, 0);
    topolsort_dura = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

    // task scheduling
    gettimeofday(&start, 0);
    scheduling(num_threads);


    while (!finish) {}    // waiting for all tasks to be finished
    gettimeofday(&end, 0);
    processtasks_dura = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

    // print all tasks result
    cout << "\nresult:" << endl;
    cout << nodes.size() << endl;

    for (int i = 0; i < nodes.size(); i++) {
        cout << "task_" << (i + 1) << ": " << task_results[nodes[i]].get() << endl;
    }

    // print execution time
    printf("\ntopological sorting execution time: %.2f s\n", topolsort_dura);
    printf("process all tasks execution time: %.2f s\n", processtasks_dura);
//    this_thread::sleep_for(chrono::seconds(10));
    custom_free();
//    delete pool;

    return 0;
}