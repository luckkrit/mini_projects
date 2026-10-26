## Install netcat

```bash
sudo apt update
sudo apt install netcat-openbsd
```

## Test Model

```bash
./run_test.sh test_models
```

## Run Server

```bash
./run_server.sh
```


### List of Commands

- ADD_BOARD      
- ADD_TASK      

### List of STATUS

- 0xXX: Success & Basic
    - STATUS_OK = 0
    - STATUS_FAIL = 1
    - STATUS_UNKNOWN = 3

- 2xXX: Client/Request Errors (200+)
    - STATUS_INVALID_ARGUMENTS = 200
    - STATUS_BOARD_NOT_FOUND = 201
    - STATUS_EMPTY_BOARD = 202
    - STATUS_TASK_NOT_FOUND = 203
    - STATUS_EMPTY_TASK = 204

- 5xXX: Server/System Errors (500+)
    - STATUS_SAVE_BOARD_FAIL = 500
    - STATUS_ADD_BOARD_FAIL = 501
    - STATUS_SAVE_TASK_FAIL = 502
    - STATUS_ADD_TASK_FAIL = 503


## Test API

### Add board

- If success return 0

```bash
echo "ADD_BOARD,Board 1\n" | nc 127.0.0.1 3030
ADD_BOARD,0
```

### Add task

- If success return 0

```bash
echo "ADD_TASK,1,Task 1\n" | nc 127.0.0.1 3030
ADD_TASK,0
```
