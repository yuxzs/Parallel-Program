#include "bfs.h"

#include <cstdlib>
#include <omp.h>

#include "../common/graph.h"

#define VERBOSE
// #define VERBOSE_top_down
// #define VERBOSE_bottom_up
// #define VERBOSE_hybrid
#ifdef VERBOSE
#include "../common/cycle_timer.h"
#include <stdio.h>
#endif // VERBOSE

#define ALPHA 0.05
#define BETA 0.1
#define Cpy 16384

constexpr int ROOT_NODE_ID = 0;
constexpr int NOT_VISITED_MARKER = -1;

void vertex_set_clear(VertexSet *list)
{
    list->count = 0;
}

void vertex_set_init(VertexSet *list, int count)
{
    list->max_vertices = count;
    list->vertices = new int[list->max_vertices];
    vertex_set_clear(list);
}

void vertex_set_destroy(VertexSet *list)
{
    delete[] list->vertices;
}

// Take one step of "top-down" BFS.  For each vertex on the frontier,
// follow all outgoing edges, and add all neighboring vertices to the
// new_frontier.
void top_down_step(Graph g, VertexSet *frontier, VertexSet *new_frontier, int *distances)
{
    #pragma omp parallel for schedule(dynamic, Cpy)
    for (int i = 0; i < frontier->count; i++)
    {
        int node = frontier->vertices[i];
        int start_edge = g->outgoing_starts[node];
        int end_edge = (node == g->num_nodes - 1) ? g->num_edges : g->outgoing_starts[node + 1];

        // attempt to add all neighbors to the new frontier
        for (int neighbor = start_edge; neighbor < end_edge; neighbor++)
        {
            int outgoing = g->outgoing_edges[neighbor];

            // if (distances[outgoing] == NOT_VISITED_MARKER)
            // {
            //     #pragma omp critical
            //     {
            //         distances[outgoing] = distances[node] + 1;
            //         int index = new_frontier->count++;
            //         new_frontier->vertices[index] = outgoing;
            //     }
            // }
            if (distances[outgoing] == NOT_VISITED_MARKER) {
                if (__sync_bool_compare_and_swap(&distances[outgoing], NOT_VISITED_MARKER, distances[node] + 1)) {
                    int index;
                    #pragma omp atomic capture
                    index = new_frontier->count++;
                    new_frontier->vertices[index] = outgoing;
                }
            }
        }
    }

    // #pragma omp parallel
    // {
    //     int tid = omp_get_thread_num();

    //     // 每個 thread 分配一塊暫存空間
    //     int local_vertices[1024];
    //     int local_count = 0;

    //     #pragma omp for schedule(dynamic, 64)
    //     for (int i = 0; i < frontier->count; i++) {
    //         int node = frontier->vertices[i];
    //         int start_edge = g->outgoing_starts[node];
    //         int end_edge = (node == g->num_nodes - 1) ? g->outgoing_starts[node + 1] : g->outgoing_starts[node + 1];

    //         for (int neighbor = start_edge; neighbor < end_edge; neighbor++) {
    //             int outgoing = g->outgoing_edges[neighbor];

    //             // 使用 atomic 保護對 distances 的存取（確保只有一個 thread 更新）
    //             if (__sync_bool_compare_and_swap(&distances[outgoing], NOT_VISITED_MARKER, distances[node] + 1)) {
    //                 local_vertices[local_count++] = outgoing;
    //             }
    //         }
    //     }

    //     // 將 local_frontier 合併進 global frontier
    //     int index;
    //     #pragma omp critical
    //     {
    //         index = new_frontier->count;
    //         new_frontier->count += local_count;
    //     }

    //     // 寫入 global frontier
    //     for (int i = 0; i < local_count; i++) {
    //         new_frontier->vertices[index + i] = local_vertices[i];
    //     }
    // }
}

// Implements top-down BFS.
//
// Result of execution is that, for each node in the graph, the
// distance to the root is stored in sol.distances.
void bfs_top_down(Graph graph, solution *sol)
{

    VertexSet list1;
    VertexSet list2;
    vertex_set_init(&list1, graph->num_nodes);
    vertex_set_init(&list2, graph->num_nodes);

    VertexSet *frontier = &list1;
    VertexSet *new_frontier = &list2;

    // initialize all nodes to NOT_VISITED
    // #pragma omp parallel for schedule(static, 8)
    for (int i = 0; i < graph->num_nodes; i++)
        sol->distances[i] = NOT_VISITED_MARKER;

    // setup frontier with the root node
    frontier->vertices[frontier->count++] = ROOT_NODE_ID;
    sol->distances[ROOT_NODE_ID] = 0;

    while (frontier->count != 0)
    {

#ifdef VERBOSE_top_down
        double start_time = CycleTimer::current_seconds();
#endif

        vertex_set_clear(new_frontier);

        top_down_step(graph, frontier, new_frontier, sol->distances);

#ifdef VERBOSE_top_down
        double end_time = CycleTimer::current_seconds();
        printf("frontier=%-10d %.4f sec\n", frontier->count, end_time - start_time);
#endif

        // swap pointers
        VertexSet *tmp = frontier;
        frontier = new_frontier;
        new_frontier = tmp;
    }

    // free memory
    vertex_set_destroy(&list1);
    vertex_set_destroy(&list2);
}

void bottom_up_step(Graph g, VertexSet *frontier, VertexSet *new_frontier, int *distances){
    VertexSet threadtemp[omp_get_max_threads()];

    for (int i = 0; i < omp_get_max_threads(); i++)
        vertex_set_init(&threadtemp[i], g->num_nodes);

    int level = distances[frontier->vertices[0]];
    #pragma omp parallel for schedule(dynamic, Cpy)
    for (int u = 0; u < g->num_nodes; u++) {
        if (distances[u] != NOT_VISITED_MARKER)
            continue;  // 已經訪問過

        int start = g->incoming_starts[u];
        int end = (u == g->num_nodes - 1) ? g->num_edges : g->incoming_starts[u + 1];
        
        for (int i = start; i < end; i++) {
            int v = g->incoming_edges[i];  // v 是 u 的父節點

            
            // if (distances[v] == level) {
            //     if (__sync_bool_compare_and_swap(&distances[u], NOT_VISITED_MARKER, level + 1)) {
            //         int tid = omp_get_thread_num();
            //         threadtemp[tid].vertices[(threadtemp[tid].count)++] = u;
            //         break;
            //     }
            // }
            if (distances[v] == level && distances[u] == NOT_VISITED_MARKER) {
                int tid = omp_get_thread_num();
                threadtemp[tid].vertices[(threadtemp[tid].count)++] = u;
                break;
            }
            // if (distances[v] == distances[frontier->vertices[0]]) {
            //     // thread-safe 加入 frontier
            //     #pragma omp critical
            //     {
            //         distances[u] = distances[frontier->vertices[0]] + 1;
            //         new_frontier->vertices[new_frontier->count++] = u;
            //     }
            //     break;
            // }

            // int level = distances[frontier->vertices[0]];

            // if (distances[v] == level) {
            //     // 如果 distances[u] 尚未訪問，就設成下一層
            //     if (__sync_bool_compare_and_swap(&distances[u], NOT_VISITED_MARKER, level + 1)) {
            //         // 加入 new_frontier，需保護 count++
            //         int index;
            //         #pragma omp atomic capture
            //         index = new_frontier->count++;

            //         new_frontier->vertices[index] = u;
            //     }
            // }
        }
    }
    // int *offsets = (int *)malloc(sizeof(int) * (max_threads + 1));
    // offsets[0] = 0;
    // for (int t = 0; t < max_threads; t++) {
    //     offsets[t + 1] = offsets[t] + local_counts[t];
    // }
    // new_frontier->count = offsets[max_threads];  // 最終總數

    // #pragma omp parallel for
    for (int t = 0; t < omp_get_max_threads(); t++) {
        int offset = new_frontier->count;
        for (int j = 0; j < threadtemp[t].count; j++) {
            new_frontier->vertices[offset + j] = threadtemp[t].vertices[j];
            distances[threadtemp[t].vertices[j]] = level + 1;
            // new_frontier->vertices[offsets[t] + j] = threadtemp[t].vertices[j];
        }
        new_frontier->count += threadtemp[t].count;
        vertex_set_destroy(&threadtemp[t]);
        // free(local_buffers[t]);
    }
    // #pragma omp parallel for
    // for (int node = 0; node < g->num_nodes; node++) {
    //     if (distances[node] != NOT_VISITED_MARKER)
    //         continue;

    //     int start = g->incoming_starts[node];
    //     int end = (node == g->num_nodes - 1) ? g->num_edges : g->incoming_starts[node + 1];

    //     for (int edge = start; edge < end; edge++) {
    //         int neighbor = g->incoming_edges[edge];

    //         if (distances[neighbor] == *current_level) {
    //             #pragma omp critical
    //             {
    //                 distances[node] = *current_level + 1;
    //                 frontier->vertices[++(frontier->count)] = node;
    //             }
    //             break;
    //         }
    //     }
    // }
}

void bfs_bottom_up(Graph graph, solution *sol)
{
    // For PP students:
    //
    // You will need to implement the "bottom up" BFS here as
    // described in the handout.
    //
    // As a result of your code's execution, sol.distances should be
    // correctly populated for all nodes in the graph.
    //
    // As was done in the top-down case, you may wish to organize your
    // code by creating subroutine bottom_up_step() that is called in
    // each step of the BFS process.

    // Graph g = graph;
    // int num_nodes = g->num_nodes;

    // 初始化距離
    // for (int i = 0; i < graph->num_nodes; i++) {
    //     sol->distances[i] = NOT_VISITED_MARKER;
    // }

    // sol->distances[ROOT_NODE_ID] = 0; 
    // int frontier_size = 1;
    // int current_level = 0;

    //// init start
    VertexSet list1;
    VertexSet list2;
    vertex_set_init(&list1, graph->num_nodes);
    vertex_set_init(&list2, graph->num_nodes);

    VertexSet *frontier = &list1;
    VertexSet *new_frontier = &list2;

    for (int i = 0; i < graph->num_nodes; i++)
        sol->distances[i] = NOT_VISITED_MARKER;

    frontier->vertices[frontier->count++] = ROOT_NODE_ID;
    sol->distances[ROOT_NODE_ID] = 0;
    // int current_level = 0;

    //// init END

    while (frontier->count > 0) {

#ifdef VERBOSE_bottom_up
    double start_time = CycleTimer::current_seconds();
#endif
        vertex_set_clear(new_frontier);
        bottom_up_step(graph, frontier, new_frontier, sol->distances);

#ifdef VERBOSE_bottom_up
        double end_time = CycleTimer::current_seconds();
        printf("frontier=%-10d %.4f sec\n", frontier->count, end_time - start_time);
#endif
        // current_level++;

        VertexSet *tmp = frontier;
        frontier = new_frontier;
        new_frontier = tmp;
    }

    // free memory
    vertex_set_destroy(&list1);
    vertex_set_destroy(&list2);
}

void bfs_hybrid(Graph graph, solution *sol)
{
    // For PP students:
    //
    // You will need to implement the "hybrid" BFS here as
    // described in the handout.

    //// init start
    VertexSet list1;
    VertexSet list2;
    vertex_set_init(&list1, graph->num_nodes);
    vertex_set_init(&list2, graph->num_nodes);

    VertexSet *frontier = &list1;
    VertexSet *new_frontier = &list2;

    for (int i = 0; i < graph->num_nodes; i++)
        sol->distances[i] = NOT_VISITED_MARKER;

    frontier->vertices[frontier->count++] = ROOT_NODE_ID;
    sol->distances[ROOT_NODE_ID] = 0;
    int current_level = 0;

    //// init END

    //// RUN START
    while (frontier->count != 0)
    {

#ifdef VERBOSE_hybrid
        double start_time = CycleTimer::current_seconds();
#endif
        int unvisited_nodes = 0;
        for (int i = 0; i < graph->num_nodes; i++){
            if (sol->distances[i] == NOT_VISITED_MARKER)
                unvisited_nodes++;
        }

        vertex_set_clear(new_frontier);
        if (frontier->count > ALPHA * graph->num_nodes && unvisited_nodes * BETA < frontier->count){
            // Bottom-up
            bottom_up_step(graph, frontier, new_frontier, sol->distances);

        }else{
            // Top-down
            
            top_down_step(graph, frontier, new_frontier, sol->distances);
            // swap pointers
        }
        
        VertexSet *tmp = frontier;
        frontier = new_frontier;
        new_frontier = tmp;

#ifdef VERBOSE_hybrid
        double end_time = CycleTimer::current_seconds();
        printf("frontier=%-10d %.4f sec\n", frontier->count, end_time - start_time);
#endif
        current_level++;
    }

    // free memory
    vertex_set_destroy(&list1);
    vertex_set_destroy(&list2);
}
