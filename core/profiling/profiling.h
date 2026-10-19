/**************************************************************************/
/*  profiling.h                                                           */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#pragma once

#include "core/profiling/profiling.gen.h" // IWYU pragma: keep.

// This header provides profiling primitives (implemented as macros) for various backends.
// See the "No profiling" branch at the bottom for a short description of the functions.

// To configure / use the profiler, use the --profiler_path and other --profiler_* arguments
// when compiling Godot. You can also find details in the SCSub file (in this folder).

// Note: It is highly recommended to avoid including this header in other header files.
//       Prefer including it in .cpp files only. The reason is that we want to keep
//       the recompile cost of changing the profiler as low as possible.

#if defined(GODOT_USE_TRACY)
// Use the tracy profiler.

#include "core/string/string_name.h"

#define TRACY_ENABLE

GODOT_MSVC_WARNING_PUSH_AND_IGNORE(4366) // The result of the unary operator may be unaligned.
#include <tracy/Tracy.hpp>
GODOT_MSVC_WARNING_POP

// Hijacking the tracy namespace so we can use their macros.
namespace tracy {
const SourceLocationData *intern_source_location(const void *p_function_ptr, const StringName &p_file, const StringName &p_function, const StringName &p_name, uint32_t p_line, bool p_is_script);
} //namespace tracy

// Define tracing macros.
#define GodotProfileFrameMark FrameMark
#define GodotProfileZone(m_zone_name) ZoneNamedN(GD_UNIQUE_NAME(__godot_tracy_szone_), m_zone_name, true)
#define GodotProfileZoneGroupedFirst(m_group_name, m_zone_name) ZoneNamedN(__godot_tracy_zone_##m_group_name, m_zone_name, true)
#define GodotProfileZoneGroupedEndEarly(m_group_name, m_zone_name) __godot_tracy_zone_##m_group_name.~ScopedZone();
#ifndef TRACY_CALLSTACK
#define GodotProfileZoneGrouped(m_group_name, m_zone_name) \
	GodotProfileZoneGroupedEndEarly(m_group_name, m_zone_name); \
	static constexpr tracy::SourceLocationData TracyConcat(__tracy_source_location, TracyLine){ m_zone_name, TracyFunction, TracyFile, (uint32_t)TracyLine, 0 }; \
	new (&__godot_tracy_zone_##m_group_name) tracy::ScopedZone(&TracyConcat(__tracy_source_location, TracyLine), true)
#else
#define GodotProfileZoneGrouped(m_group_name, m_zone_name) \
	GodotProfileZoneGroupedEndEarly(m_group_name, m_zone_name); \
	static constexpr tracy::SourceLocationData TracyConcat(__tracy_source_location, TracyLine){ m_zone_name, TracyFunction, TracyFile, (uint32_t)TracyLine, 0 }; \
	new (&__godot_tracy_zone_##m_group_name) tracy::ScopedZone(&TracyConcat(__tracy_source_location, TracyLine), TRACY_CALLSTACK, true)
#endif

#define GodotProfileZoneScript(m_ptr, m_file, m_function, m_name, m_line) \
	tracy::ScopedZone __godot_tracy_script(tracy::intern_source_location(m_ptr, m_file, m_function, m_name, m_line, true))
#define GodotProfileZoneScriptSystemCall(m_ptr, m_file, m_function, m_name, m_line) \
	tracy::ScopedZone __godot_tracy_zone_system_call(tracy::intern_source_location(m_ptr, m_file, m_function, m_name, m_line, false))

// Memory allocation
#ifdef GODOT_PROFILER_TRACK_MEMORY
#define GodotProfileAlloc(m_ptr, m_size) \
	GODOT_GCC_WARNING_PUSH_AND_IGNORE("-Wmaybe-uninitialized") \
	TracyAlloc(m_ptr, m_size); \
	GODOT_GCC_WARNING_POP
#define GodotProfileFree(m_ptr) TracyFree(m_ptr)
#else
#define GodotProfileAlloc(m_ptr, m_size)
#define GodotProfileFree(m_ptr)
#endif

void godot_init_profiler();
void godot_cleanup_profiler();

#elif defined(GODOT_USE_PERFETTO)
// Use the perfetto profiler.

#include "core/typedefs.h"
#include "main/performance.h"

#include <perfetto.h>

PERFETTO_DEFINE_CATEGORIES(
		perfetto::Category("godot")
				.SetDescription("Godot Engine Events"),
		perfetto::Category("godot_scripting")
				.SetDescription("Godot Scripting Events"), );

// See PERFETTO_INTERNAL_SCOPED_EVENT_FINALIZER
struct PerfettoGroupedEventEnder {
	_FORCE_INLINE_ void _end_now() {
		TRACE_EVENT_END("godot");
	}

	_FORCE_INLINE_ ~PerfettoGroupedEventEnder() {
		_end_now();
	}
};

#define GodotProfileFrameMark \
	perfetto::CounterTrack __frame_time_track = perfetto::CounterTrack("Frame time", "ms").set_unit_multiplier(1000); \
	TRACE_COUNTER("godot", __frame_time_track, Performance::get_singleton()->get_monitor(Performance::Monitor::TIME_PROCESS));
#define GodotProfileZone(m_zone_name) TRACE_EVENT("godot", m_zone_name);
#define GodotProfileZoneGroupedFirst(m_group_name, m_zone_name) \
	TRACE_EVENT_BEGIN("godot", m_zone_name); \
	PerfettoGroupedEventEnder __godot_perfetto_zone_##m_group_name
#define GodotProfileZoneGroupedEndEarly(m_group_name, m_zone_name) __godot_perfetto_zone_##m_group_name.~PerfettoGroupedEventEnder()
#define GodotProfileZoneGrouped(m_group_name, m_zone_name) \
	__godot_perfetto_zone_##m_group_name._end_now(); \
	TRACE_EVENT_BEGIN("godot", m_zone_name);

static thread_local HashSet<StringName> __tracing_system_call;

/**
 * Script tracing may cross function boundaries (tracing started in the caller script), so the logic below only triggers
 * a TRACE_EVENT_BEGIN for `GodotProfileZoneScript` if tracing hasn't already been initiated by
 * `GodotProfileZoneScriptSystemCall` in the caller.
 */
struct PerfettoScriptTracer {
	StringName name;
	bool is_system_call;
	bool tracing;

	PerfettoScriptTracer(const StringName &p_file, const StringName &p_function, const StringName &p_name, int p_line, bool p_system_call) : name(p_name), is_system_call(p_system_call) {
		if (is_system_call || !__tracing_system_call.erase(name)) {
			TRACE_EVENT_BEGIN("godot_scripting", perfetto::DynamicString(p_name.string().utf8().get_data()), "source file", p_file.string().utf8().get_data(), "line number", p_line);
			tracing = true;
		}

		if (is_system_call) {
			__tracing_system_call.insert(name);
		}
	}

	_FORCE_INLINE_ void _end_now() {
		if (tracing) {
			TRACE_EVENT_END("godot_scripting");
		}
	}

	_FORCE_INLINE_ ~PerfettoScriptTracer() {
		_end_now();
		if (is_system_call) {
			__tracing_system_call.erase(name);
		}
	}
};

#define GodotProfileZoneScript(m_ptr, m_file, m_function, m_name, m_line) PerfettoScriptTracer __godot_perfetto_script_tracer(m_file, m_function, m_name, m_line, false);
#define GodotProfileZoneScriptSystemCall(m_ptr, m_file, m_function, m_name, m_line) PerfettoScriptTracer __godot_perfetto_script_system_call_tracer(m_file, m_function, m_name, m_line, true);

#define GodotProfileAlloc(m_ptr, m_size)
#define GodotProfileFree(m_ptr)

void godot_init_profiler();
void godot_cleanup_profiler();

#elif defined(GODOT_USE_INSTRUMENTS)

#include <os/log.h>
#include <os/signpost.h>

class StringName;

namespace apple::instruments {

extern os_log_t LOG;
extern os_log_t LOG_TRACING;

#ifdef INSTRUMENTS_SCRIPT_ZONES
struct ScriptLocation {
	const char *name;
	const char *file;
};

// Returns UTF-8 copies of p_name and p_file that stay valid until exit.
ScriptLocation intern_script_location(const StringName &p_file, const StringName &p_name);
#endif

// IDs of the zones open on this thread, innermost last.
struct ZoneStack {
	static constexpr uint32_t MAX_DEPTH = 256;
	os_signpost_id_t ids[MAX_DEPTH] = {};
	uint32_t depth = 0;
};

// Tells other files that zone_stack needs no setup at run time,
// so they read it directly instead of calling a TLS wrapper.
[[clang::require_constant_initialization]] extern thread_local ZoneStack zone_stack;

// Like Tracy, ending a zone ends the innermost open zone on the thread,
// so a group can switch zones while a nested zone is still open.
class Zone {
public:
	os_signpost_id_t id;
	Zone() {
		start();
	}
	~Zone() {
		end();
	}
	void start() {
		// Generate first, so zone_stack is not touched while signposts are disabled.
		id = os_signpost_id_generate(LOG_TRACING);
		if (id != OS_SIGNPOST_ID_NULL) {
			ZoneStack &stack = zone_stack;
			if (stack.depth < ZoneStack::MAX_DEPTH) {
				stack.ids[stack.depth++] = id;
			} else {
				id = OS_SIGNPOST_ID_NULL;
			}
		}
	}
	void end() {
		if (id != OS_SIGNPOST_ID_NULL) {
			id = OS_SIGNPOST_ID_NULL;
			ZoneStack &stack = zone_stack;
			os_signpost_interval_end(LOG_TRACING, stack.ids[--stack.depth], "Zone");
		}
	}
};

} // namespace apple::instruments

#define GodotProfileFrameMark \
	os_signpost_event_emit(apple::instruments::LOG, OS_SIGNPOST_ID_EXCLUSIVE, "Frame");

#define GodotProfileZoneGroupedFirst(m_group_name, m_zone_name) \
	apple::instruments::Zone _GD_VARNAME_CONCAT_(__instruments_zone__, _, m_group_name); \
	os_signpost_interval_begin(apple::instruments::LOG_TRACING, _GD_VARNAME_CONCAT_(__instruments_zone__, _, m_group_name).id, "Zone", "%{public}s", m_zone_name);

#define GodotProfileZoneGroupedEndEarly(m_group_name, m_zone_name) \
	_GD_VARNAME_CONCAT_(__instruments_zone__, _, m_group_name).end();

#define GodotProfileZoneGrouped(m_group_name, m_zone_name) \
	GodotProfileZoneGroupedEndEarly(m_group_name, m_zone_name); \
	_GD_VARNAME_CONCAT_(__instruments_zone__, _, m_group_name).start(); \
	os_signpost_interval_begin(apple::instruments::LOG_TRACING, _GD_VARNAME_CONCAT_(__instruments_zone__, _, m_group_name).id, "Zone", "%{public}s", m_zone_name);

#define GodotProfileZone(m_zone_name) \
	GodotProfileZoneGroupedFirst(__COUNTER__, m_zone_name)

#ifdef INSTRUMENTS_SCRIPT_ZONES
#define _GodotProfileZoneScriptNamed(m_var, m_file, m_name, m_line) \
	apple::instruments::Zone m_var; \
	if (m_var.id != OS_SIGNPOST_ID_NULL) { \
		const apple::instruments::ScriptLocation _loc = apple::instruments::intern_script_location(m_file, m_name); \
		os_signpost_interval_begin(apple::instruments::LOG_TRACING, m_var.id, "Zone", "%{public}s %{public}s:%u", _loc.name, _loc.file, (uint32_t)(m_line)); \
	}

#define GodotProfileZoneScript(m_ptr, m_file, m_function, m_name, m_line) \
	_GodotProfileZoneScriptNamed(__instruments_zone_script, m_file, m_name, m_line)
#define GodotProfileZoneScriptSystemCall(m_ptr, m_file, m_function, m_name, m_line) \
	_GodotProfileZoneScriptNamed(__instruments_zone_system_call, m_file, m_name, m_line)
#else
#define GodotProfileZoneScript(m_ptr, m_file, m_function, m_name, m_line)
#define GodotProfileZoneScriptSystemCall(m_ptr, m_file, m_function, m_name, m_line)
#endif

// Instruments has its own memory profiling, so these are no-ops.
#define GodotProfileAlloc(m_ptr, m_size)
#define GodotProfileFree(m_ptr)

void godot_init_profiler();
void godot_cleanup_profiler();

#else
// No profiling; all macros are stubs.

void godot_init_profiler();
void godot_cleanup_profiler();

// Tell the profiling backend that a new frame has started.
#define GodotProfileFrameMark
// Defines a profile zone from here to the end of the scope.
#define GodotProfileZone(m_zone_name)
// Defines a profile zone group. The first profile zone starts immediately,
// and ends either when the next zone starts, or when the scope ends.
#define GodotProfileZoneGroupedFirst(m_group_name, m_zone_name)
// End the profile zone group's current profile zone now.
#define GodotProfileZoneGroupedEndEarly(m_group_name, m_zone_name)
// Replace the profile zone group's current profile zone.
// The new zone ends either when the next zone starts, or when the scope ends.
#define GodotProfileZoneGrouped(m_group_name, m_zone_name)
// Tell the profiling backend that an allocation happened, with its location and size.
#define GodotProfileAlloc(m_ptr, m_size)
// Tell the profiling backend that an allocation was freed.
// There must be a one to one correspondence of GodotProfileAlloc and GodotProfileFree calls.
#define GodotProfileFree(m_ptr)

// Define a zone for a script call (dynamic source location).
// m_ptr is a pointer to the function instance, which will be used for the lookup.
// m_file, m_function, m_name are StringNames, and m_line is uint32_t
#define GodotProfileZoneScript(m_ptr, m_file, m_function, m_name, m_line)
// Define a zone for a system call from a script (dynamic source location).
#define GodotProfileZoneScriptSystemCall(m_ptr, m_file, m_function, m_name, m_line)

#endif
