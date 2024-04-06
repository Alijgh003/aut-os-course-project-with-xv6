

#define MAX_HISTORY 16
#define INPUT_BUF_SIZE 128


struct history_buffer
{
    char bufferArr[MAX_HISTORY][INPUT_BUF_SIZE];
    uint64 lengthArr[MAX_HISTORY];
    uint64 lastCommandIndex;
    int numOfCommandsInMemory;
    int currentHistory;

    uint64 IDsArr[MAX_HISTORY];
    uint64 lastCommnadID;
};

struct history
{
    int historyId;
    char command[INPUT_BUF_SIZE];
};
