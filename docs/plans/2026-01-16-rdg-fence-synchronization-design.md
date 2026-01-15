# RDG Fence-Based Synchronization Design

## Overview

Add `API_TRAIT_EXPLICIT_DEPENDENCIES` as an alternative to `API_TRAIT_HONORS_PIPELINE_BARRIERS`. When enabled, the RenderingDeviceGraph uses MTLFence for intra-command-buffer synchronization instead of pipeline barriers. Commands execute in recording order with inline fence wait/signal operations.

## Motivation

Metal's pipeline barriers have different semantics than Vulkan's. Rather than emulating barriers, use Metal's native fence primitives for explicit dependency tracking. This provides:

- Native Metal synchronization semantics
- Fine-grained stage-level waits (critical for TBDR)
- Simpler execution model (no topological sort needed)
- Cross-encoder parallelism handled by GPU scheduler

## Driver Interface

### New API Trait

```cpp
API_TRAIT_EXPLICIT_DEPENDENCIES  // Mutually exclusive with API_TRAIT_HONORS_PIPELINE_BARRIERS
```

### Fence Operations

```cpp
virtual FenceID fence_allocate() = 0;
virtual void fence_free(FenceID p_fence) = 0;
virtual void command_signal_fence(CommandBufferID p_cmd, FenceID p_fence,
                                   BitField<PipelineStageBits> p_stages) = 0;
virtual void command_wait_fence(CommandBufferID p_cmd, FenceID p_fence,
                                 BitField<PipelineStageBits> p_stages) = 0;
```

### Metal Implementation

```objc
// fence_allocate
id<MTLFence> fence = [device newFence];

// command_signal_fence
[encoder updateFence:fence afterStages:mtl_stages];

// command_wait_fence
[encoder waitForFence:fence beforeStages:mtl_stages];
```

Stage bits map from Godot's `PipelineStageBits` to Metal's `MTLRenderStages`.

## SPIR-V Reflection

### Per-Binding Access Data

During SPIR-V reflection, capture stage access per binding:

```cpp
struct BindingAccess {
    BitField<PipelineStageBits> read_stages;   // Stages that read this binding
    BitField<PipelineStageBits> write_stages;  // Stages that write this binding
};
```

Derived from:
- **Stages**: Which entry points (vertex/fragment/compute) reference the binding
- **Write access**: Absence of `NonWritable` decoration on storage resources

### Pipeline Resource Access Map

Stored on pipeline at creation:

```cpp
struct PipelineResourceAccess {
    HashMap<uint64_t, BindingAccess> bindings;  // Key: (set << 32) | binding
    BitField<PipelineStageBits> any_write_stages;  // Union of all write stages
};
```

## Class Hierarchy

```
RenderingDeviceGraphBase (abstract)
├── RenderingDeviceGraphBarrier  (existing behavior)
└── RenderingDeviceGraphFence    (new implementation)
```

### Base Class

```cpp
class RenderingDeviceGraphBase {
protected:
    RenderingDeviceDriver *driver = nullptr;

    // Command storage (shared)
    LocalVector<uint8_t> command_data;
    LocalVector<uint32_t> command_data_offsets;
    uint32_t command_count = 0;

    // Resource tracking, labels, secondary buffers (shared)

public:
    virtual ~RenderingDeviceGraphBase() = default;

    // Recording API (shared)
    void add_buffer_clear(BufferID p_buffer, ...);
    void add_texture_copy(TextureID p_src, TextureID p_dst, ...);
    void add_draw_list(DrawListID p_list, ...);
    void add_compute_list(ComputeListID p_list, ...);

    virtual void end(bool p_reorder = true, bool p_full_barriers = false) = 0;

protected:
    virtual void _on_resource_access(RecordedCommand *p_cmd,
                                      ResourceTracker *p_tracker,
                                      ResourceUsage p_usage,
                                      BitField<PipelineStageBits> p_stages) = 0;
};
```

### Barrier Implementation

Existing behavior: adjacency edges, topological sort, level batching, grouped barrier emission.

### Fence Implementation

```cpp
class RenderingDeviceGraphFence : public RenderingDeviceGraphBase {
    LocalVector<FenceID> fence_pool;
    uint32_t fence_pool_index = 0;

    void _on_resource_access(...) override;
    void end(bool p_reorder, bool p_full_barriers) override;
};
```

### Factory

```cpp
RenderingDeviceGraphBase *RenderingDeviceGraph::create(RenderingDeviceDriver *p_driver) {
    if (p_driver->api_trait_get(RDD::API_TRAIT_EXPLICIT_DEPENDENCIES)) {
        return memnew(RenderingDeviceGraphFence(p_driver));
    }
    return memnew(RenderingDeviceGraphBarrier(p_driver));
}
```

## Fence Implementation Behavior

### Recording Phase

When a command accesses a resource:

```cpp
void _on_resource_access(cmd, tracker, usage, stages) {
    if (is_write(usage)) {
        // Allocate fence, store on command
        cmd->write_fence = _allocate_fence();
        cmd->fence_signal_stages = stages;
        tracker->last_write_command = cmd;
    } else {
        // Record wait if prior writer exists
        if (tracker->last_write_command) {
            cmd->waits.push_back({
                tracker->last_write_command->write_fence,
                stages  // Wait before these read stages
            });
        }
    }
}
```

### Execution Phase

```cpp
void end(bool, bool) override {
    for (cmd : commands_in_recording_order) {
        // Emit waits
        for (wait : cmd->waits) {
            driver->command_wait_fence(cmd_buffer, wait.fence, wait.stages);
        }

        // Execute command
        _execute_command(cmd);

        // Signal fence
        if (cmd->write_fence.is_valid()) {
            driver->command_signal_fence(cmd_buffer, cmd->write_fence,
                                         cmd->fence_signal_stages);
        }
    }
}
```

### Fence Lifecycle

- Pool managed by `RenderingDeviceGraphFence`
- Recycled per-frame (MTLFence has no CPU state to reset)
- Allocated on-demand, freed at shutdown

## Execution Model

### No Sorting Required

Commands execute in recording order. Fences handle synchronization explicitly. The encoder-per-list granularity provides natural parallelism:

- Different lists → different encoders → GPU parallelizes via fence deps
- Same list → same encoder → sequential (intentional)

### Stage Granularity (TBDR)

Per-binding stage masks enable optimal TBDR performance:

- Wait before only the stages that read (vertex work unblocked for fragment→fragment deps)
- Signal after stages that write

Example: If pipeline A writes in fragment and pipeline B reads in fragment, B's vertex stage can overlap with A's fragment work.

### Cross-Encoder Parallelism

Metal overlaps execution across encoders when fences allow. Since each draw/compute list creates its own encoder, independent work parallelizes automatically without graph analysis.

## Simplifications vs Barrier Path

| Removed | Reason |
|---------|--------|
| Topological sort | Fences handle ordering |
| Level grouping | No batching needed |
| Priority tiers | GPU scheduler decides |
| Barrier vectors | Replaced by fence ops |
| Adjacency lists | No edges needed |

## Implementation Steps

1. Add `API_TRAIT_EXPLICIT_DEPENDENCIES` to driver traits
2. Add fence operations to `RenderingDeviceDriver` interface
3. Extend SPIR-V reflection to capture per-binding stage access
4. Add `PipelineResourceAccess` to pipeline objects
5. Create `RenderingDeviceGraphBase` abstract class
6. Rename existing implementation to `RenderingDeviceGraphBarrier`
7. Implement `RenderingDeviceGraphFence`
8. Add factory function for graph creation
9. Implement Metal driver fence operations
10. Set `API_TRAIT_EXPLICIT_DEPENDENCIES` in Metal driver
