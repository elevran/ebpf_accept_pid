//go:build ignore

#include "common.h"
#include "bpf_tracing.h"

char __license[] SEC("license") = "Dual MIT/GPL";

struct accept_event {
	u32 pid;
    u32 tid;
    char comm[32];
};

struct {
	__uint(type, BPF_MAP_TYPE_RINGBUF);
	__uint(max_entries, 1 << 24);
} events SEC(".maps");

// Force emitting struct event into the ELF.
const struct event *unused __attribute__((unused));

SEC("kprobe/__sys_accept4")
int BPF_KPROBE(handle_accept)
{
    struct accept_event *event;
    __u32 pid = bpf_get_current_pid_tgid() >> 32;
    __u32 tid = bpf_get_current_pid_tgid();

    event = bpf_ringbuf_reserve(&events, sizeof(*event), 0);
    if (!event) {
        return 0; // Couldn't reserve space in the ring buffer
    }

    event->pid = pid;
    event->tid = tid;
    bpf_get_current_comm(event->comm, sizeof(event->comm));

    bpf_ringbuf_submit(event, 0);
    
    return 0;
}
