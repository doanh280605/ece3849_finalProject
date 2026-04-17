1. 
vSnakeTask updates shared state snake[] and snakeLength, while vRenderTask reads that same state, and there is no mutex, critical section, queue or other synchronization around those 
accesses. One task can be preempted in the middle of updating the snake, and the render task
can see partially updated data. 
the Cortex-M4 does not guarantee that multi-word struct accesses are atomic across tasks. 
A context switch can happen between separate loads/stores so the renderer may read inconsistent snake state. 

2. 
No, volatile is not enought to make this safe. volatile only tells the compiler not to optimize away the read/write and to actually access memory each time. It does not provide mutual exclusion, atomic coordination, or prevent a task switch in the middle of a larger operation 

3. 
If vRenderTask runs every 33ms, then in 1 second it redraws about: 1000 / 33 = 30.3
When the game is paused, it causes about 30 unnecessary redraws per second, since nothing on screen is changing but the render task still keep drawing. 

Part 1: I would use one mutex for the shared game state rather than separate mutexes for each variable. The snake position, snake length, direction, food and score are logically related and are read together by the render task. One mutex keeps the design simpler and ensures the renderer sees a consistent game state. The critical section should be kept short by copying the shared state into local variables and release the mutex before performing the LCD draw. 

Part 2: 
Keep wrap around and not implement wall collision