## Install netcat

```bash
sudo apt update
sudo apt install netcat-openbsd
```

## Run Server

```bash
make run-server
```3we 

## Run Client

```bash
make run-client
```

## Test API

### View Products

```bash
echo "VIEW_PRODUCT" | nc 127.0.0.1 3030
```