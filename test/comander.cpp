#include <sys/wait.h> // for wait()
#include <unistd.h>   // for pipe(), read(), write(), close(), fork(), and _exit()
#include <cctype>     // for toupper()
#include <cerrno>     // for errno
#include <cstdlib>    // for EXIT_SUCCESS and EXIT_FAILURE
#include <cstring>    // for strerror()
#include <deque>      // for deque (used for ready and blocked queues)
#include <fstream>    // for ifstream (used for reading simulated programs)
#include <iostream>   // for cout, endl, and cin
#include <iostream>
#include <sstream> // for stringstream (used for parsing simulated programs)
#include <string>
#include <vector> // for vector (used for PCB table) using namespace std;

using namespace std;

const std::string whiteSpaces(" \f\n\r\t\v");

void trimRight(std::string &str, const std::string &trimChars = whiteSpaces)
{
  std::string::size_type pos = str.find_last_not_of(trimChars);
  if (pos != std::string::npos)
  {
    str.erase(pos + 1);
  }
  else
  {
    // If no character was found that was not in trimChars, clear the string.
    str.clear();
  }
}

void trimLeft(std::string &str, const std::string &trimChars = whiteSpaces)
{
  std::string::size_type pos = str.find_first_not_of(trimChars);
  if (pos != std::string::npos)
  {
    str.erase(0, pos);
  }
  else
  {
    // If no character was found that was not in trimChars, clear the string.
    str.clear();
  }
}

void trim(std::string &str, const std::string &trimChars = whiteSpaces)
{
  trimRight(str, trimChars);
  trimLeft(str, trimChars);
}

class Instruction
{
public:
  char operation;
  int intArg;
  string stringArg;
};
class Cpu
{
public:
  vector<Instruction> *pProgram;
  int programCounter;
  int value;
  int timeSlice;
  int timeSliceUsed;
};
enum State
{
  STATE_READY,
  STATE_RUNNING,
  STATE_BLOCKED,
  STATE_TERMINATED
};
class PcbEntry
{
public:
  int processId;
  int parentProcessId;
  vector<Instruction> program;
  unsigned int programCounter;
  int value;
  unsigned int priority;
  State state;
  unsigned int startTime;
  unsigned int timeUsed;
};
vector<PcbEntry> pcbTable; // Correct definition of pcbTable
Cpu cpu;
deque<int> readyState;
deque<int> blockedState;
int runningState = -1;
unsigned int timestamp = 0;

PcbEntry pcbEntry[10];
// For the states below, -1 indicates empty (since it is an invalid index).
// In this implementation, we'll never explicitly clear PCB entries and the
// index in the table will always be the process ID. These choices waste memory,
// but since this program is just a simulation it the easiest approach.
// Additionally, debugging is simpler since table slots and process IDs are
// never re-used.
double cumulativeTimeDiff = 0;
int numTerminatedProcesses = 0;
bool createProgram(const string &filename, vector<Instruction> &program)
{
  ifstream file;
  int lineNum = 0;
  file.open(filename.c_str());
  if (!file.is_open())
  {
    cout << "Error opening file " << filename << endl;
    return false;
  }
  while (file.good())
  {
    string line;
    getline(file, line);
    trim(line);
    if (line.size() > 0)
    {
      Instruction instruction;
      instruction.operation = toupper(static_cast<unsigned char>(line[0]));
      line.erase(0, 1); // Erase the first character from line
      trim(line);       // Trim the modified line
      instruction.stringArg =
          line; // Assign the trimmed line to instruction.stringArg
      stringstream argStream(instruction.stringArg);
      switch (instruction.operation)
      {
      case 'S': // Integer argument.
      case 'A': // Integer argument.
      case 'D': // Integer argument.
      case 'F': // Integer argument.
        if (!(argStream >> instruction.intArg))
        {
          cout << filename << ":" << lineNum << " - Invalid integer argument "
               << instruction.stringArg << " for " << instruction.operation
               << " operation" << endl;
          file.close();
          return false;
        }
        break;
      case 'B': // No argument.
      case 'E': // No argument.
        break;
      case 'R': // String argument.
        // Note that since the string is trimmed on both ends, filenames
        // with leading or trailing whitespace (unlikely) will not work.
        if (instruction.stringArg.size() == 0)
        {
          cout << filename << ":" << lineNum << " -Missing string argument"
               << endl;
          file.close();
          return false;
        }
        break;
      default:
        cout << filename << ":" << lineNum << " - Invalid operation, "
             << instruction.operation << endl;
        file.close();
        return false;
      }
      program.push_back(instruction);
    }
    lineNum++;
  }
  file.close();
  return true;
}
// Implements the S operation.
void set(int value)
{
  // TODO: Implement
  // 1. Set the CPU value to the passed-in value.
  cpu.value = value;
}
// Implements the A operation.
void add(int value)
{
  // TODO: Implement
  // 1. Add the passed-in value to the CPU value.
  cpu.value += value;
}
// Implements the D operation.
void decrement(int value)
{
  // TODO: Implement
  // 1. Subtract the integer value from the CPU value.
  cpu.value -= value;
}
// Performs scheduling.
void schedule() {
    if (runningState != -1) {
        cout << "A process is already running." << endl;
        return; // Keeps the current process running if it's already there
    }

    if (readyState.empty()) {
        cout << "Ready queue is empty. No process to schedule." << endl;
        runningState = -1; // Explicitly set no running process
        return;
    }

    // Get the next process from the ready queue
    int nextProcessId = readyState.front();
    readyState.pop_front();

    PcbEntry &nextProcess = pcbTable[nextProcessId];
    if (nextProcess.state == STATE_TERMINATED) {
        schedule(); // Recursively call schedule if the next process is terminated
        return;
    }

    // Set the next process as the running process
    nextProcess.state = STATE_RUNNING;
    runningState = nextProcessId;
    cpu.pProgram = &nextProcess.program;
    cpu.programCounter = nextProcess.programCounter;
    cpu.value = nextProcess.value;

    cout << "Scheduling process " << nextProcessId << " as running." << endl;
}





// Implements the B operation.
void block()
{
  // TODO: Implement
  // 1. Add the PCB index of the running process (stored in runningState) to the
  // blocked queue.
  // 2. Update the process's PCB entry
  // a. Change the PCB's state to blocked.
  // b. Store the CPU program counter in the PCB's program counter.
  // c. Store the CPU's value in the PCB's value.
  // 3. Update the running state to -1 (basically mark no process as running).
  // Note that a new process will be chosen to run later (via the Q command code
  // calling the schedule() function).

  // 1. Add the PCB index of the running process (stored in runningState) to the
  // blocked queue.
  if (runningState != -1)
  { // Ensure there is a running process to block
    blockedState.push_back(runningState);

    // 2. Update the process's PCB entry
    // a. Change the PCB's state to blocked.
    pcbEntry[runningState].state = STATE_BLOCKED;

    // b. Store the CPU program counter in the PCB's program counter.
    pcbEntry[runningState].programCounter = cpu.programCounter;

    // c. Store the CPU's value in the PCB's value.
    pcbEntry[runningState].value = cpu.value;

    // 3. Update the running state to -1 (basically mark no process as running).
    runningState = -1;
  }
  else
  {
    cout << "No process is currently running to block." << endl;
  }
}
// Implements the E operation.
void end() {
    if (runningState != -1) {
        PcbEntry &currentProcess = pcbTable[runningState];
        currentProcess.state = STATE_TERMINATED;
        cout << "Process " << runningState << " terminated." << endl;
        
        cumulativeTimeDiff += (timestamp + 1 - currentProcess.startTime);
        numTerminatedProcesses++;
        
        runningState = -1;
        schedule();
    } else {
        cout << "No process is currently running to end." << endl;
    }
}


// Implements the F operation.
void fork(int value) {
  // Get a free PCB index
  int freeIndex = pcbTable.size();
  pcbTable.resize(freeIndex + 1); // Ensure space for the new process

  if (runningState == -1) {
    cout << "No process is currently running." << endl;
    return;
  }
  PcbEntry &parentProcess = pcbTable[runningState];
  parentProcess.program = *cpu.pProgram;
  parentProcess.programCounter = cpu.programCounter;
  parentProcess.value = cpu.value;

  if (value < 0) {
    cout << "Value is out of bounds." << endl;
    return;
  }

  // Initialize the new process (child)
  pcbTable[freeIndex].processId = freeIndex;
  pcbTable[freeIndex].parentProcessId = parentProcess.processId;
  pcbTable[freeIndex].programCounter = parentProcess.programCounter; // Child starts from the next instruction
  pcbTable[freeIndex].value = parentProcess.value;                       // Inherit parent's value
  pcbTable[freeIndex].priority = parentProcess.priority;
  pcbTable[freeIndex].state = STATE_RUNNING;
  pcbTable[freeIndex].startTime = timestamp;
  pcbTable[freeIndex].program = vector<Instruction>(parentProcess.program);

  cout << "current program address: " << cpu.pProgram << endl;
  cout << "Parent process " << parentProcess.processId << " program address: " << &parentProcess.program << endl;
  cout << "Child process " << pcbTable[freeIndex].processId << " program address: " << &pcbTable[freeIndex].program << endl;

  // Temporarily set the child as the running process
  int oldRunningState = runningState;
  runningState = freeIndex;
  cpu.pProgram = &pcbTable[freeIndex].program;


  // Update the parent's program counter and push it to the ready queue
  parentProcess.programCounter += value; // Parent continues after 'value' more instructions
  parentProcess.state = STATE_READY; // Move parent to ready state
  readyState.push_back(oldRunningState); // Add parent back to ready queue

  // Now call the scheduler to decide the next step
  schedule();
}


// Implements the R operation.
void replace(string &argument)
{
    if (runningState == -1)
    {
        cout << "No process is currently running to replace its program." << endl;
        return;
    }

    // Clear the CPU's program
    cpu.pProgram->clear();

    // Use createProgram() to read in the filename specified by the argument into the CPU
    if (!createProgram(argument, *cpu.pProgram))
    {
        cout << "Error: Failed to load the program from file '" << argument << "'." << endl;
        cpu.programCounter++; // Increment the program counter to skip the failed instruction
        return;
    }

    // Set the program counter to 0
    cpu.programCounter = 0;

    pcbTable[runningState].program = *cpu.pProgram;
    pcbTable[runningState].programCounter = cpu.programCounter;

    cout << "Replaced current program with program from file '" << argument << "'." << endl;
}


void reportSystemState()
{
  cout << "****************************************************************\n";
  cout << "The current system state is as follows:\n";
  cout << "****************************************************************\n";

  int processCount = 0;
  for (size_t i = 0; i < pcbTable.size(); ++i)
  {
    if (pcbTable[i].state == STATE_READY || pcbTable[i].state == STATE_BLOCKED || pcbTable[i].state == STATE_RUNNING || pcbTable[i].state == STATE_TERMINATED)
    {
      processCount++;
    }
  }

  cout << processCount << " processes in the PCB table." << endl;

  for (size_t i = 0; i < pcbTable.size(); ++i)
  {
    if (pcbTable[i].state == STATE_READY || pcbTable[i].state == STATE_BLOCKED || pcbTable[i].state == STATE_RUNNING || pcbTable[i].state == STATE_TERMINATED)
    {
      cout << "Process ID: " << pcbTable[i].processId << ", State: ";
      switch (pcbTable[i].state)
      {
      case STATE_READY:
        cout << "Ready";
        break;
      case STATE_BLOCKED:
        cout << "Blocked";
        break;
      case STATE_RUNNING:
        cout << "Running";
        break;
      case STATE_TERMINATED:
        cout << "Terminated";
        break;
      default:
        cout << "Unknown";
        break;
      }
      cout << ", Program Counter: " << pcbTable[i].programCounter << ", Value: " << pcbTable[i].value << endl;
    }
  }
  cout << "Current running state: " << runningState << endl;
}

void spawnReporter()
{
  pid_t reporterPid = fork();
  if (reporterPid == 0)
  {
    // Child process - Reporter
    reportSystemState();
    _exit(0);
  }
  else if (reporterPid > 0)
  {
    // Parent process - Process Manager
    wait(NULL); // Wait for the reporter to finish
  }
  else
  {
    // Fork failed
    perror("Failed to create reporter process");
  }
}

void calculateAverageTurnaroundTime()
{
  if (numTerminatedProcesses > 0)
  {
    double avgTurnaroundTime = cumulativeTimeDiff / numTerminatedProcesses;
    cout << "Average Turnaround Time: " << avgTurnaroundTime << " units." << endl;
  }
  else
  {
    cout << "No processes have completed; cannot calculate turnaround time." << endl;
  }
}

// Implements the Q command.
// Implements the Q command.
void quantum() {
    cout << cpu.pProgram << endl;

    if (runningState == -1) {
        cout << "No processes are running" << endl;
        ++timestamp;
        return;
    }
    
    Instruction instruction;
    if (cpu.programCounter < cpu.pProgram->size()) {
        instruction = (*cpu.pProgram)[cpu.programCounter];
        ++cpu.programCounter;
    } else {
        cout << "End of program reached without E operation" << endl;
        instruction.operation = 'E';
    }
    
    cout << "In quantum instruction " << instruction.operation;
    if (instruction.operation == 'S' || instruction.operation == 'A' || instruction.operation == 'D' || instruction.operation == 'F') {
        cout << " " << instruction.intArg;
    }
    if (instruction.operation == 'R') {
        cout << " " << instruction.stringArg;
    }
    cout << endl;
    
    switch (instruction.operation) {
        case 'S':
            set(instruction.intArg);
            break;
        case 'A':
            add(instruction.intArg);
            break;
        case 'D':
            decrement(instruction.intArg);
            break;
        case 'B':
            block();
            return;
        case 'E':
            end();
            return;
        case 'F':
            fork(instruction.intArg);
            break;
        case 'R':
            replace(instruction.stringArg);
            break;
    }
    
    if (runningState != -1) {
        pcbTable[runningState].programCounter = cpu.programCounter;
        pcbTable[runningState].value = cpu.value;
    }
    
    ++timestamp;
    schedule();
}



// Implements the U command.
void unblock()
{
  // 1. If the blocked queue contains any processes:
  // a. Remove a process form the front of the blocked queue.
  // b. Add the process to the ready queue.
  // c. Change the state of the process to ready (update its PCB entry).
  // 2. Call the schedule() function to give an unblocked process a chance to
  // run (if possible).
  // 1. If the blocked queue contains any processes:
  if (!blockedState.empty())
  {
    // a. Remove a process from the front of the blocked queue.
    int processId = blockedState.front();
    blockedState.pop_front();

    // b. Add the process to the ready queue.
    readyState.push_back(processId);

    // c. Change the state of the process to ready (update its PCB entry).
    pcbEntry[processId].state = STATE_READY;
  }

  // 2. Call the schedule() function to give an unblocked process a chance to
  // run (if possible).
  schedule();
}
// Implements the P command.
void print()
{
  cout << "Print command is not implemented until iLab 3" << endl;
}
// Function that implements the process manager.
int runProcessManager(int fileDescriptor)
{
  pcbTable.resize(1);
  if (!createProgram("init", pcbEntry[0].program))
  {
    return EXIT_FAILURE;
  }
  pcbEntry[0].processId = 0;
  pcbEntry[0].parentProcessId = -1;
  pcbEntry[0].programCounter = 0;
  pcbEntry[0].value = 0;
  pcbEntry[0].priority = 0;
  pcbEntry[0].state = STATE_RUNNING;
  pcbEntry[0].startTime = 0;
  pcbEntry[0].timeUsed = 0;
  runningState = 0; // Set the initial process to running
  cpu.pProgram = &(pcbEntry[0].program);
  cpu.programCounter = pcbEntry[0].programCounter;
  cpu.value = pcbEntry[0].value;
  timestamp = 0;
  double avgTurnaroundTime = 0;

  char ch;
  do
  {
    if (read(fileDescriptor, &ch, sizeof(ch)) != sizeof(ch))
    {
      // Assume the parent process exited, breaking the pipe.
      break;
    }

    switch (ch)
    {
    case 'Q':
      quantum();
      break;
    case 'U':
      unblock();
      break;
    case 'P':
      spawnReporter();
      break;
    case 'T':
      spawnReporter();
      calculateAverageTurnaroundTime();
      return EXIT_SUCCESS; // Terminate after handling 'T'
    default:
      cout << "Invalid command received: " << ch << endl;
      break;
    }
  } while (ch != 'T');
  return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    int pipeDescriptors[2];
    pid_t processMgrPid;
    char ch;
    int result;
    
    pipe(pipeDescriptors);
    
    if ((processMgrPid = fork()) == -1)
        exit(1); /* FORK FAILED */
    if (processMgrPid == 0) {
        close(pipeDescriptors[1]);
        result = runProcessManager(pipeDescriptors[0]);
        close(pipeDescriptors[0]);
        _exit(result);
    } else {
        close(pipeDescriptors[0]);
        do {
            cout << "Enter Q, P, U or T" << endl;
            cout << "$ ";
            cin >> ch;
            if (write(pipeDescriptors[1], &ch, sizeof(ch)) != sizeof(ch)) {
                break;
            }
        } while (ch != 'T');
        write(pipeDescriptors[1], &ch, sizeof(ch));
        close(pipeDescriptors[1]);
        wait(&result);
    }
    return result;
}


