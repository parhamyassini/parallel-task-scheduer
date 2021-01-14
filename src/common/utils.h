#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <limits.h>
#include "quick_sort.h"

#define intV int32_t
#define uintV int32_t
#define UINTV_MAX INT_MAX

#define intE int32_t
#define uintE int32_t

#define DEFAULT_NUMBER_OF_PRODUCERS "1"
#define DEFAULT_NUMBER_OF_CONSUMERS "1"
#define DEFAULT_SECONDS "1"
#define DEFAULT_INIT_ALLOCATOR "10000001"

// CAS operation
template <class ET>
inline bool CAS(ET *ptr, ET oldv, ET newv)
{
  if (sizeof(ET) == 1)
  {
    return __sync_bool_compare_and_swap((bool *)ptr, *((bool *)&oldv), *((bool *)&newv));
  }
  else if (sizeof(ET) == 4)
  {
    return __sync_bool_compare_and_swap((int *)ptr, *((int *)&oldv), *((int *)&newv));
  }
  else if (sizeof(ET) == 8)
  {
    return __sync_bool_compare_and_swap((long *)ptr, *((long *)&oldv), *((long *)&newv));
  }
  else
  {
    std::cout << "CAS bad length : " << sizeof(ET) << std::endl;
    abort();
  }
}

template <class T>
bool checkEqual(T *array1, T *array2, long n)
{
  bool flag = true;
  quickSort(array1, n, [](uintV val1, uintV val2) {
      return val1 < val2;
  });

  quickSort(array2, n, [](uintV val1, uintV val2) {
      return val1 < val2;
  });


  for(long i = 0; i < n; i++)
  {
    if (array1[i] != array2[i])
    {
      flag = false;
      break;
    }
  }
  return flag;
}

#endif
