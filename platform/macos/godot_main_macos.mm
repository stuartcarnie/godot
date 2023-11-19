/**************************************************************************/
/*  godot_main_macos.mm                                                   */
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

#include "os_macos.h"

#include "main/main.h"

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/time.h>

// Function to check if a debugger is attached to the current process
bool is_debugger_attached() {
	int mib[4];
	struct kinfo_proc info{};
	size_t size = sizeof(info);

	// Initialize the flags so that, if sysctl fails, info.kp_proc.p_flag will be 0.
	info.kp_proc.p_flag = 0;

	// Initialize mib, which tells sysctl the info we want, in this case we're looking for information
	// about a specific process ID.
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_PID;
	mib[3] = getpid();

	if (sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, nullptr, 0) != 0) {
		perror("sysctl");
		return false;
	}

	return (info.kp_proc.p_flag & P_TRACED) != 0;
}

// Function to wait for a debugger to attach until a specified time has elapsed
bool wait_for_debugger(CFTimeInterval wait_time) {
	CFAbsoluteTime start = CFAbsoluteTimeGetCurrent();
	while (!is_debugger_attached()) {
		if (CFAbsoluteTimeGetCurrent() > start + wait_time) {
			return false;
		}
		// Sleep for 100ms
		[NSThread sleepForTimeInterval:0.100];
	}
	return true;
}

#if defined(SANITIZERS_ENABLED)
#include <sys/resource.h>
#endif

int main(int argc, char **argv) {
#if defined(VULKAN_ENABLED)
	// MoltenVK - enable full component swizzling support.
	setenv("MVK_CONFIG_FULL_IMAGE_VIEW_SWIZZLE", "1", 1);
#endif

#if defined(SANITIZERS_ENABLED)
	// Note: Set stack size to be at least 30 MB (vs 8 MB default) to avoid overflow, address sanitizer can increase stack usage up to 3 times.
	struct rlimit stack_lim = { 0x1E00000, 0x1E00000 };
	setrlimit(RLIMIT_STACK, &stack_lim);
#endif

	int first_arg = 1;
	const char *dbg_arg = "-NSDocumentRevisionsDebugMode";
	for (int i = 0; i < argc; i++) {
		if (strcmp(dbg_arg, argv[i]) == 0) {
			first_arg = i + 2;
		}
	}

	OS_MacOS os;
	Error err;

	// We must override main when testing is enabled.
	TEST_MAIN_OVERRIDE

	@autoreleasepool {
		err = Main::setup(argv[0], argc - first_arg, &argv[first_arg]);
	}

	if (err == ERR_HELP) { // Returned by --help and --version, so success.
		return 0;
	} else if (err != OK) {
		return 255;
	}

	bool ok;
	@autoreleasepool {
		ok = Main::start();
	}
	if (ok) {
		os.run(); // It is actually the OS that decides how to run.
	}

	@autoreleasepool {
		Main::cleanup();
	}

	return os.get_exit_code();
}
