#include <iostream>
#include <stdio.h>
#include "scheduler.h"
#include <functional>
#include "common/graph.h"
#include "common/utils.h"


Graph g;
uintV countTriangles(uintV *array1, uintE len1, uintV *array2, uintE len2, uintV u, uintV v) {

    uintE i = 0, j = 0; // indexes for array1 and array2
    uintV count = 0;
    while ((i < len1) && (j < len2)) {
        if (array1[i] == array2[j]) {
            if ((array1[i] != u) && (array1[j] != v)) {
                count++;
            }
            i++;
            j++;
        } else if (array1[i] < array2[j]) {
            i++;
        } else {
            j++;
        }
    }
    return count;
}

long compute(int id, int start_vertex, int end_vertex) {


    long triangle_num = 0;

    for (uintV u = start_vertex; u < end_vertex; u++) {
        // For each outNeighbor v, find the intersection of inNeighbor(u) and outNeighbor(v)

        uintE out_degree = g.vertices_[u].getOutDegree();
//        cout << u << endl;
        for (uintE i = 0; i < out_degree; i++) {
            uintV v = g.vertices_[u].getOutNeighbor(i);
            triangle_num += countTriangles(g.vertices_[u].getInNeighbors(),
                                           g.vertices_[u].getInDegree(),
                                           g.vertices_[v].getOutNeighbors(),
                                           g.vertices_[v].getOutDegree(),
                                           u,
                                           v);
        }
    }
    return triangle_num;
//    return 5;
}


long process_data(int id, int start, int stop) {

    this_thread::sleep_for(chrono::seconds(1));

    return start;
}

int main(int argc, char **argv) {
    uint n_workers = 6;
    std::string input_file_path = "/home/pdcl-11/courses/fall2019/parallel-distributed-computing/assignments/input_graphs/lj";
    std::cout << std::fixed;
    std::cout << "Number of workers : " << n_workers << "\n";
//    Graph g;

    std::cout << "Reading graph\n";
    g.read_graph_from_binary<int>(input_file_path);
    std::cout << "Created graph\n";
    uintV n = g.n_;
    // init task dependency graph (DAG)
    vector < Task * > tasks;
    unordered_map < Task * , vector < Task * >> DAG;


    int num_tasks = n_workers + 1;


    cout << "n : " << n << endl;
    tasks.push_back(new Task(-1)); // dummy

    uint numtasks = n / n_workers;
    uint remainder = n % n_workers;
    uint index0;
    uint index1;

    for (int tid = 1; tid <= n_workers; ++tid) {

        index0 = ((tid -1) < remainder ? (tid-1) * (numtasks + 1) : n - (n_workers - (tid-1)) * numtasks);
        index1 = index0 + numtasks + ((tid-1) < remainder);
//        t[tid] = std::thread(compute, std::ref(g), tid, index0, index1);
//        cout << index0 << "  "  << index1 << endl;
        int x = get_unique_id();
        tasks.push_back(new Task(x,1,compute, index0, index1));

    }


    struct timeval start, end;
    double topolsort_dura, processtasks_dura;

    // get topological order (a tree struction)
    gettimeofday(&start, 0);
    topo_sort(tasks, DAG);
    gettimeofday(&end, 0);
    topolsort_dura = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

    // task scheduling
    gettimeofday(&start, 0);
    scheduling(n_workers);


    while (!finish) {}    // waiting for all tasks to be finished
    gettimeofday(&end, 0);
    processtasks_dura = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

    // print all tasks result
    cout << "\nresult:" << endl;
//    cout << nodes.size() << endl;

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