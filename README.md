# NTUA ECE Operating Systems Course

This repository showcases the laboratory assignments and programming implementations I completed during the Operating Systems course at the National Technical University of Athens (NTUA). Through this coursework, I acquired direct exposure in systems programming in C within a Linux environment, working with low-level POSIX process lifecycle management, signal handling, Inter-Process Communication (IPC) via pipes, I/O multiplexing with `select()`, and network socket communication.

---

## Laboratory Exercises

### Exercise 1: Process Creation & File I/O
*Introduction to low-level UNIX process management and POSIX file system system calls.*

* **Key Concepts:** `fork()`, `wait()`, `open()`, `stat()`, `write()`, file descriptors, process hierarchy.
* **Overview:** 
  * Implemented command-line argument validation and file existence verification using `stat()`.
  * Created a child process using `fork()`; both parent and child retrieve and write their execution identifiers (`PID` and `PPID`) to an output file using low-level `write()` calls.
  * Synchronized the parent process with child termination using `wait()`.
* **Files:** `ex1.c`

---

### Exercise 2: Signal Handling & Process Supervision ("The Gates")
*Asynchronous inter-process signaling, timer-driven actions, and active child process health monitoring.*

* **Key Concepts:** `sigaction()`, `kill()`, `alarm()`, `SIGUSR1`, `SIGUSR2`, `SIGALRM`, `SIGCHLD`, `SIGTERM`, process trees.
* **Overview:**
  * Modeled a multi-gate state controller where a parent process spawns $N$ child processes (one for each gate) initialized from a state string (e.g., `"tfttt"`).
  * Child processes maintain gate state (`open` / `closed`), report state periodically via periodic timer signals (`SIGALRM`), and dynamically toggle or display status upon receiving user/parent signals (`SIGUSR1`, `SIGUSR2`).
  * Implemented parent supervision logic catching `SIGCHLD`: automatically detecting stopped or killed children with `waitpid()` (`WUNTRACED`), resuming paused processes, and respawning fresh child gates to maintain exactly $N$ active gates at all times until a graceful `SIGTERM` broadcast.
* **Files:** `ask2_parent.c`, `ask2_child.c`

---

### Exercise 3: Inter-Process Communication & I/O Multiplexing
*Workload scheduling across multiple child workers using pipes and non-blocking I/O multiplexing.*

* **Key Concepts:** Anonymous pipes (`pipe()`), `select()`, `fd_set`, non-blocking I/O, IPC scheduling algorithms.
* **Overview:**
  * Developed a job distribution system where a parent creates $N$ worker processes connected through dedicated pipelines.
  * Implemented job delegation from terminal inputs using both **Round-Robin** and **Random** distribution policies.
  * Child processes read tasks, process calculations with simulated latency, and stream results back via return pipes.
  * Utilized `select()` in the parent process to concurrently multiplex user inputs from `STDIN_FILENO` alongside read-ends from all child pipelines without busy waiting.
* **Files:** `ask3_1pipe.c`, `ask3_2pipe.c`

---

### Exercise 4: Network Stream Sockets & Remote Server Communication
*Client-server communication using internet domain stream sockets and event-driven I/O.*

* **Key Concepts:** `socket()`, `connect()`, `bind()`, `AF_INET`, `SOCK_STREAM` (TCP), `gethostbyname()`, network byte order (`htons`), telemetry parsing.
* **Overview:**
  * Developed an interactive TCP client communicating with an external remote IoT server over the network.
  * Handled domain resolution via DNS, socket endpoint creation, and reliable TCP handshaking.
  * Integrated `select()` to multiplex user terminal commands (`help`, `get`, `exit`) with incoming server payloads.
  * Parsed incoming sensor packets from the server, decoding raw payloads into human-readable telemetry (boot/interval status, temperature readings, light levels, and UNIX timestamp conversions).
* **Files:** `ask4.c`

---

## Technologies & System Calls

* **Language:** C (C99 / POSIX)
* **Environment:** GNU/Linux
* **Core APIs & Headers:** `<unistd.h>`, `<signal.h>`, `<sys/wait.h>`, `<sys/select.h>`, `<sys/socket.h>`, `<netinet/in.h>`, `<netdb.h>`
* **Key Topics:** Process Lifecycles, Signal Handlers, IPC (Pipes), Synchronous I/O Multiplexing (`select`), Network Programming (TCP Sockets)
