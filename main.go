package main

import (
	"C"
	"encoding/binary"
	"fmt"
	"os"
	"os/signal"
	"syscall"

	"github.com/cilium/ebpf/link"
	"github.com/cilium/ebpf/ringbuf"
)
import "bytes"

//go:generate go run github.com/cilium/ebpf/cmd/bpf2go -cc clang BpfProgram accept.c -- -I./headers -I.

type AcceptEvent struct {
	PID  uint32
	TID  uint32
	COMM [32]byte
}

func main() {
	// Load the pre-compiled programs and maps into the kernel.
	objs := BpfProgramObjects{}
	if err := LoadBpfProgramObjects(&objs, nil); err != nil {
		fmt.Fprintf(os.Stderr, "loading objects: %v\n", err)
		os.Exit(1)
	}
	defer objs.Close()

	// Attach the kprobe to sys_accept4
	kprobe, err := link.Kprobe("__sys_accept4", objs.BpfProgramPrograms.HandleAccept, nil)
	if err != nil {
		fmt.Fprintf(os.Stderr, "creating kprobe: %v\n", err)
		os.Exit(1)
	}
	defer kprobe.Close()

	// Create a new ring buffer reader from the `events` map
	rd, err := ringbuf.NewReader(objs.Events)
	if err != nil {
		fmt.Fprintf(os.Stderr, "creating ringbuf reader: %v\n", err)
		os.Exit(1)
	}
	defer rd.Close()

	// Set up signal handling to gracefully exit
	sig := make(chan os.Signal, 1)
	signal.Notify(sig, os.Interrupt, syscall.SIGTERM)
	go func() {
		<-sig
		fmt.Println("\nReceived signal, exiting...")
		rd.Close()
		os.Exit(0)
	}()

	fmt.Println("Waiting for events...")

	// Read events
	for {
		record, err := rd.Read()
		if err != nil {
			if err == ringbuf.ErrClosed {
				return
			}
			fmt.Fprintf(os.Stderr, "reading from ringbuf: %v\n", err)
			continue
		}

		var event AcceptEvent
		if err := binary.Read(bytes.NewBuffer(record.RawSample), binary.LittleEndian, &event); err != nil {
			fmt.Fprintf(os.Stderr, "parsing ringbuf event: %v\n", err)
			continue
		}

		fmt.Printf("Accept Event: PID=%d TID=%d COMM=%s\n", event.PID, event.TID, event.COMM)
	}
}
