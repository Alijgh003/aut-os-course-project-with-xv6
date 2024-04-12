//
// Console input and output, to the uart.
// Reads are line at a time.
// Implements special input characters:
//   newline -- end of line
//   control-h -- backspace
//   control-u -- kill line
//   control-d -- end of file
//   control-p -- print process list
//

#include <stdarg.h>
#include <stddef.h> 

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"
#include "console.h"

#define BACKSPACE 0x100
#define C(x)  ((x)-'@')  // Control-x


struct history_buffer history_buffer_array;

int escdown = 0;
int specialkeydown = 0;
int historyCursor = 0;


void
increaseHistoryCursor()
{
  historyCursor = historyCursor == history_buffer_array.numOfCommandsInMemory-1? historyCursor : historyCursor+1;
}

void
decreaseHistoryCursor()
{
  historyCursor = historyCursor <= 0 ? 0 : historyCursor-1;
}

void
resetHistoryCursor()
{
  historyCursor = 0;
}

//
// send one character to the uart.
// called by printf(), and to echo input characters,
// but not from write().
//
void
consputc(int c)
{
  if(c == BACKSPACE){
    // if the user typed backspace, overwrite with a space.
    uartputc_sync('\b'); uartputc_sync(' '); uartputc_sync('\b');
  } else {
    uartputc_sync(c);
  }
}

struct {
  struct spinlock lock;
  
  // input
  char buf[INPUT_BUF_SIZE];
  uint r;  // Read index
  uint w;  // Write index
  uint e;  // Edit index
} cons;


//
// user write()s to the console go here.
//
int
consolewrite(int user_src, uint64 src, int n)
{
  int i;

  for(i = 0; i < n; i++){
    char c;
    if(either_copyin(&c, user_src, src+i, 1) == -1)
      break;
    uartputc(c);
  }

  return i;
}

//
// user read()s from the console go here.
// copy (up to) a whole input line to dst.
// user_dist indicates whether dst is a user
// or kernel address.
//
int
consoleread(int user_dst, uint64 dst, int n)
{
  uint target;
  int c;
  char cbuf;


  target = n;
  acquire(&cons.lock);
  while(n > 0){
    // wait until interrupt handler has put some
    // input into cons.buffer.
    while(cons.r == cons.w){
      if(killed(myproc())){
        release(&cons.lock);
        return -1;
      }
      sleep(&cons.r, &cons.lock);
    }

    c = cons.buf[cons.r++ % INPUT_BUF_SIZE];

    if(c == C('D')){  // end-of-file
      if(n < target){
        // Save ^D for next time, to make sure
        // caller gets a 0-byte result.
        cons.r--;
      }
      break;
    }

    // copy the input byte to the user-space buffer.
    cbuf = c;
    if(either_copyout(user_dst, dst, &cbuf, 1) == -1)
      break;

    dst++;
    --n;

    if(c == '\n'){
      // a whole line has arrived, return to
      // the user-level read().
      break;
    }
  }
  release(&cons.lock);

  return target - n;
}

void
consputstr(char *str)
{
  for(int i=0;i<strlen(str);i++){
    consputc(str[i]);
  }
}


void
killline()
{
  while(cons.e != cons.w &&
          cons.buf[(cons.e-1) % INPUT_BUF_SIZE] != '\n'){
      cons.e--;
      consputc(BACKSPACE);
    }
}

void
printhistory(int index){
  if(history_buffer_array.numOfCommandsInMemory < 1){
    consputstr("history buffer is empty.\n");
  }else{
    int currentIndex = history_buffer_array.currentHistory + 1;
    char* resultCommand = "\0";
    consputstr("commnads in history are:\n");
    for(int i=0; i<history_buffer_array.numOfCommandsInMemory; i++){
      consputstr(history_buffer_array.bufferArr[currentIndex % history_buffer_array.numOfCommandsInMemory]);
      consputc('\n');
      if(i == index){
        resultCommand = history_buffer_array.bufferArr[i];
      }
      currentIndex++;
    }
    consputstr("the target command is:\n");
    if(strncmp(resultCommand,"\0",strlen(resultCommand))){
      consputstr(resultCommand);
      consputc('\n');
    }else{
      consputstr("invalid index\n");
    }
  }
}

void 
getbufstr(char* c)
{
  char command[INPUT_BUF_SIZE]= "\0";
  int counter = 0;
  for(int i=cons.w;i<cons.e;i++){
    int c  = cons.buf[(i) % INPUT_BUF_SIZE];
    if(c == '\n'){
      break;
    }else{
      command[counter] = c; 
      counter++ ; 
    }
  }
  strncpy(c, command,strlen(command));
}

void
addcommand(){
  char command[INPUT_BUF_SIZE] = "\0";
  getbufstr(command);
  int commandLen = strlen(command);
  //TODO: isValid to chekc validation of command;
  if(!strstr(command,"history")){
    history_buffer_array.currentHistory = (history_buffer_array.numOfCommandsInMemory == 0) ? 0 : (history_buffer_array.lastCommandIndex + 1) % MAX_HISTORY;
    history_buffer_array.numOfCommandsInMemory = (history_buffer_array.numOfCommandsInMemory < MAX_HISTORY) ? history_buffer_array.numOfCommandsInMemory+1 : MAX_HISTORY;
    strncpy(history_buffer_array.bufferArr[history_buffer_array.currentHistory],"\0",history_buffer_array.lengthArr[history_buffer_array.currentHistory]+1);
    history_buffer_array.lengthArr[history_buffer_array.currentHistory] = commandLen;
    strncpy(history_buffer_array.bufferArr[history_buffer_array.currentHistory],command,commandLen);
    history_buffer_array.lastCommandIndex = history_buffer_array.currentHistory;
  }
} 

void 
storeforconsumption(int c)
{
  // store for consumption by consoleread().
  cons.buf[cons.e++ % INPUT_BUF_SIZE] = c;
}

void
showHistoryCursor()
{
  killline();
  char *command = history_buffer_array.bufferArr[(history_buffer_array.lastCommandIndex-historyCursor)%history_buffer_array.numOfCommandsInMemory];
  for(int i=0;i<strlen(command);i++){
    consputc(command[i]);
    storeforconsumption(command[i]);
  }
}

void
upkeyhandler()
{
  //TODO
  showHistoryCursor();
  increaseHistoryCursor();
}

void 
downkeyhandler()
{
  //TODO
  decreaseHistoryCursor();
  showHistoryCursor();
}

void
specialkeyhandler(int c)
{
  switch (c)
  {
  case 'A':
    upkeyhandler();
    break;
  
  case 'B':
    downkeyhandler();
    break;

  default:
    break;
  }
}

void
defaultconsoleintrhandler(int c)
{
  resetHistoryCursor();
  if(c != 0 && cons.e-cons.r < INPUT_BUF_SIZE){
      c = (c == '\r') ? '\n' : c;
      
      // echo back to the user.
      consputc(c);

      storeforconsumption(c);

      if(c == '\n' || c == C('D') || cons.e-cons.r == INPUT_BUF_SIZE){
        // wake up consoleread() if a whole line (or end-of-file)
        // has arrived.
        addcommand();
        cons.w = cons.e;
        wakeup(&cons.r);
      }
    }
}


//
// the console input interrupt handler.
// uartintr() calls this for input character.
// do erase/kill processing, append to cons.buf,
// wake up consoleread() if a whole line has arrived.
//
void
consoleintr(int c)
{
  acquire(&cons.lock);

  switch(c){
  case C('P'):  // Print process list.
    resetHistoryCursor();
    procdump();
    break;
  case C('U'):  // Kill line.
    resetHistoryCursor();
    killline();
    break;
  case C('H'): // Backspace
  case '\x7f': // Delete key
    resetHistoryCursor();
    if(cons.e != cons.w){
      cons.e--;
      consputc(BACKSPACE);
    }
    break;

  case '\x1B': //Escape Key
    escdown = 1;
    break;

  case '\x5B': // [ char
    if(escdown){
      escdown = 0;
      specialkeydown = 1;
    }else{
      defaultconsoleintrhandler(c);
    }
    break;
  
  case 'A':
  case 'B':
    if(specialkeydown){
      specialkeydown = 0;
      specialkeyhandler(c);
    }else{
      defaultconsoleintrhandler(c);
    }
    break;

  default:
    defaultconsoleintrhandler(c);
    break;
  }
  release(&cons.lock);
}

void
consoleinit(void)
{
  initlock(&cons.lock, "cons");

  uartinit();

  // connect read and write system calls
  // to consoleread and consolewrite.
  devsw[CONSOLE].read = consoleread;
  devsw[CONSOLE].write = consolewrite;
}
