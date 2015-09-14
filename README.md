# Netpaxos
Netpaxos required Libevent

## Build Netpaxos

```
mkdir build; cd build
cmake ..
make
```

## Run Netpaxos server
```
./replica -s -t 1000 -n 1000 eth0
```

## Run Netpaxos client
```
./replica -c -t 1000 -n 1000 eth0
```
