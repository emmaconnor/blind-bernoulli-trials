
#define TIME_IT(F) do { double _end, _start; _start = get_time(); F; _end = get_time(); printf("TIME    %s:%d:    %012.8lf\n", __FILE__, __LINE__, _end - _start); } while (0)

int rand_range(int n);
int *shuffled_range(int n);
double get_time();
void alloc_failure();

