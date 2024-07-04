# Verify PID in accept

The eBPF program and maps are managed in Go using github.com/cilium/ebpf.
 Ensure Go is installed and github.com/cilium/ebpf/cmd/bpf2go is installed
 (`go install github.com/cilium/ebpf/cmd/bpf2go`). The minimal set of headers
 (e.g., no "vmlinux.h") are under `headers` directory.

## compile and run server

```console
mkdir ./bin
gcc -o ./bin/server server.c
./bin/server &
PID 13201 waiting for connections...
# verify the server is running: 
ncat 127.0.0.1 12345
.^C
```

Note the process id reported by server.

## generate the eBPF program and loader

```console
go generate . 
go build -o ./bin/accept_pid .
sudo ./bin/accept_pid
Waiting for events...
```

Test connection and eBPF output by connecting to server: `ncat 127.0.0.1 12345`
