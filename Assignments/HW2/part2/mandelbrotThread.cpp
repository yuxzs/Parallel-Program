#include <stdio.h>
#include <stdlib.h>
#include <thread>

typedef struct
{
    float x0, x1;
    float y0, y1;
    unsigned int width;
    unsigned int height;
    int maxIterations;
    int *output;
    int threadId;
    int numThreads;
} WorkerArgs;

extern void mandelbrotSerial(
    float x0, float y0, float x1, float y1,
    int width, int height,
    int startRow, int numRows,
    int maxIterations,
    int output[]);

//
// workerThreadStart --
//
// Thread entrypoint.
void workerThreadStart(WorkerArgs *const args)
{

    // TODO FOR PP STUDENTS: Implement the body of the worker
    // thread here. Each thread could make a call to mandelbrotSerial()
    // to compute a part of the output image. For example, in a
    // program that uses two threads, thread 0 could compute the top
    // half of the image and thread 1 could compute the bottom half.
    // Of course, you can copy mandelbrotSerial() to this file and
    // modify it to pursue a better performance.

    // printf("Hello world from thread %d\n", args->threadId);

    // int remainder = args->height % args->numThreads;

    // int startRow = args->threadId * (args->height / args->numThreads) + (args->threadId-1 < remainder ? 1 : 0)*args->threadId;
    // int endRow = (args->threadId+1) * (args->height / args->numThreads) + (args->threadId < remainder ? 1 : 0)*(args->threadId+1);
    
    // int j, k, index;
    // unsigned int i;
    // float x, y, z_re, z_im, new_re, new_im;
    // float dx = (args->x1 - args->x0) / args->width;
    // float dy = (args->y1 - args->y0) / args->height;

    for ( unsigned int m = 0; m < args->height; m-=-args->numThreads){
        mandelbrotSerial(
            args->x0, args->y0, args->x1, args->y1,
            args->width, args->height,
            m+args->threadId, 1,
            args->maxIterations,
            args->output
        );
    
        // j = m+args->threadId;
        // for (i = 0; i < args->width; ++i)
        // {
        //     x = args->x0 + i * dx;
        //     y = args->y0 + j * dy;

        //     index = (j * args->width + i);

        //     z_re = x;
        //     z_im = y;
        //     for(k=0; k < args->maxIterations; ++k){
        //         if (z_re * z_re + z_im * z_im > 4.f)
        //             break;
        //         new_re = z_re * z_re - z_im * z_im;
        //         new_im = 2.f * z_re * z_im;
        //         z_re = x + new_re;
        //         z_im = y + new_im;
        //     }
        //     args->output[index] = k;
        // }
    }

}

//
// MandelbrotThread --
//
// Multi-threaded implementation of mandelbrot set image generation.
// Threads of execution are created by spawning std::threads.
void mandelbrotThread(
    int numThreads,
    float x0, float y0, float x1, float y1,
    int width, int height,
    int maxIterations, int output[])
{
    static constexpr int MAX_THREADS = 32;

    if (numThreads > MAX_THREADS)
    {
        fprintf(stderr, "Error: Max allowed threads is %d\n", MAX_THREADS);
        exit(1);
    }

    // Creates thread objects that do not yet represent a thread.
    std::thread workers[MAX_THREADS];
    WorkerArgs args[MAX_THREADS] = {};

    for (int i = 0; i < numThreads; i++)
    {
        // TODO FOR PP STUDENTS: You may or may not wish to modify
        // the per-thread arguments here.  The code below copies the
        // same arguments for each thread
        args[i].x0 = x0;
        args[i].y0 = y0;
        args[i].x1 = x1;
        args[i].y1 = y1;
        args[i].width = width;
        args[i].height = height;
        args[i].maxIterations = maxIterations;
        args[i].numThreads = numThreads;
        args[i].output = output;

        args[i].threadId = i;
    }

    // Spawn the worker threads.  Note that only numThreads-1 std::threads
    // are created and the main application thread is used as a worker
    // as well.
    for (int i = 1; i < numThreads; i++)
    {
        workers[i] = std::thread(workerThreadStart, &args[i]);
    }

    workerThreadStart(&args[0]);

    // join worker threads
    for (int i = 1; i < numThreads; i++)
    {
        workers[i].join();
    }
}
