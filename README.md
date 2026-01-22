## Install netcat

```bash
sudo apt update
sudo apt install netcat-openbsd
```

## Test Client

```bash
echo "BUY 1101 1" | nc 127.0.0.1 3030
```