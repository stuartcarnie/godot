/**************************************************************************/
/*  metal_fwd.h                                                           */
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

// C++ compatible forward declarations for Metal driver types
// Use this header when you need to reference Metal types from pure C++ code

#pragma once

#include <Metal/Metal.hpp>

class RenderingContextDriverMetal;
class RenderingDeviceDriver;

namespace MTL3 {
class MDCommandBuffer;
class RenderingDeviceDriverMetal;

// C++ compatible method to get MTL::CommandBuffer* from MDCommandBuffer
// Implemented in metal3_objects.mm
MTL::CommandBuffer *get_command_buffer_cpp(MDCommandBuffer *p_cmd_buffer);

// Factory function to create MTL3 driver (implemented in rendering_device_driver_metal3.mm)
RenderingDeviceDriver *create_rendering_device_driver(RenderingContextDriverMetal *p_context);
} // namespace MTL3

#ifdef METAL4_ENABLED
namespace MTL4 {
class MDCommandBuffer;
class RenderingDeviceDriverMetal;

// Factory function to create MTL4 driver (implemented in rendering_device_driver_metal4.mm)
RenderingDeviceDriver *create_rendering_device_driver(RenderingContextDriverMetal *p_context);
} // namespace MTL4
#endif
