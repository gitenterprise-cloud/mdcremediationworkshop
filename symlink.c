#include <vmlinux.h>
#include <bpf/bpf_helpers.h>
#include <gadget/buffer.h>
#include <gadget/macros.h>
#include <gadget/mntns_filter.h>
#include <gadget/types.h>

#define NAME_MAX 255

struct event {
    gadget_mntns_id mntns_id;
    __u8 oldname[NAME_MAX];

};

GADGET_TRACER_MAP(events, 1024 * 256);

GADGET_TRACER(symlink, events, event);

SEC("tracepoint/syscalls/sys_enter_symlinkat")

int enter_symlinkat(struct syscall_trace_enter *ctx)
{
    u64 mntns_id;
    struct event *event;

    mntns_id = gadget_get_mntns_id();
    if (gadget_should_discard_mntns_id(mntns_id))
        return 0;
    
    event = gadget_reserve_buf(&events, sizeof(*event));
    if (!event)
        return 0;
    
    event->mntns_id = mntns_id;
    bpf_core_read_user_str(&event->oldname, sizeof(event->oldname), (void *)ctx->args[0]);

    gadget_submit_buf(ctx, &events, event, sizeof(*event));

    return 0;
}

char LICENSE[] SEC("license") = "GPL";