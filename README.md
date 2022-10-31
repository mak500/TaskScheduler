# Task Scheduler

Implements a generic task scheduler which schedules jobs on a thread pool.
Threads are spawned depending on the hardware supported number of threads or
the number of threads requested via the user whichever is minimum. Each thread
then acts as a worker thread, processing it's job queue.

class `Scheduler` exposes a generic `schedule` function which does scheduling
asynchronously. User can provide the function and arguments to the `schedule`
function. The `schedule` function appends the job to a specific queue.
Currently, job is appended to the threads in a round robin fashion. Results of
the task is communicated using `futures`.

Note: The job should always have a return type. The limitation is due to the
implemenation of `std::future<void>::set_value` function. If return type is void
then the program won't compile.
