#include "logger.h"
#include "PPintrin.h"

void Logger::addLog(const char *instruction, __pp_mask mask, int N)
{
  Log newLog;
  strcpy(newLog.instruction, instruction);
  newLog.mask = 0;
  for (int i = 0; i < N; i++)
  {
    if (mask.value[i])
    {
      newLog.mask |= (((unsigned long long)1) << i);
      stats.utilized_lane++;
    }
  }
  stats.total_lane += N;
  stats.total_instructions += (N > 0);
  log.push_back(newLog);
}

void Logger::printStats()
{
  printf("****************** Printing Vector Unit Statistics *******************\n");
  printf("Vector Width:              %d\n", VECTOR_WIDTH);
  printf("Total Vector Instructions: %lld\n", stats.total_instructions);
  printf("Vector Utilization:        %.1f%%\n", (double)stats.utilized_lane / stats.total_lane * 100);
  printf("Utilized Vector Lanes:     %lld\n", stats.utilized_lane);
  printf("Total Vector Lanes:        %lld\n", stats.total_lane);
}

void Logger::printLog()
{
  printf("***************** Printing Vector Unit Execution Log *****************\n");
  printf(" Instruction | Vector Lane Occupancy ('*' for active, '_' for inactive)\n");
  printf("------------- --------------------------------------------------------\n");
  for (int i = 0; i < log.size(); i++)
  {
    printf("%12s | ", log[i].instruction);
    for (int j = 0; j < VECTOR_WIDTH; j++)
    {
      if (log[i].mask & (((unsigned long long)1) << j))
      {
        printf("*");
      }
      else
      {
        printf("_");
      }
    }
    printf("\n");
  }
}

void Logger::refresh()
{
  stats.total_instructions = 0;
  stats.total_lane = 0;
  stats.utilized_lane = 0;
  fflush(stdout);
};

unsigned long long Logger::getTotalInstrs()
{
  return stats.total_instructions;
}
