//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// QuartzCore/CAMetalDisplayLink.hpp
//
// Copyright 2020-2024 Apple Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#include "../Foundation/NSObject.hpp"
#include "../Foundation/NSString.hpp"
#include "../Foundation/NSTypes.hpp"
#include <CoreFoundation/CFDate.h>

#include "CADefines.hpp"
#include "CAMetalDrawable.hpp"
#include "CAMetalLayer.hpp"
#include "CAPrivate.hpp"

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace CA
{

// Layout matches CAFrameRateRange.
struct FrameRateRange
{
    float minimum;
    float maximum;
    float preferred;
} _CA_PACKED;

class MetalDisplayLinkUpdate : public NS::Referencing<MetalDisplayLinkUpdate>
{
public:
    class MetalDrawable* drawable() const;
    CFTimeInterval       targetTimestamp() const;
    CFTimeInterval       targetPresentationTimestamp() const;
};

class MetalDisplayLink : public NS::Referencing<MetalDisplayLink>
{
public:
    static class MetalDisplayLink* alloc();
    class MetalDisplayLink*        init(const class MetalLayer* layer);

    NS::Object*                    delegate() const;
    void                           setDelegate(const NS::Object* delegate);

    FrameRateRange                 preferredFrameRateRange() const;
    void                           setPreferredFrameRateRange(FrameRateRange range);

    float                          preferredFrameLatency() const;
    void                           setPreferredFrameLatency(float latency);

    bool                           isPaused() const;
    void                           setPaused(bool paused);

    void                           addToRunLoop(const NS::Object* runLoop, const NS::String* mode);
    void                           removeFromRunLoop(const NS::Object* runLoop, const NS::String* mode);
    void                           invalidate();
};

} // namespace CA

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE CA::MetalDrawable* CA::MetalDisplayLinkUpdate::drawable() const
{
    return Object::sendMessage<MetalDrawable*>(this, _CA_PRIVATE_SEL(drawable));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE CFTimeInterval CA::MetalDisplayLinkUpdate::targetTimestamp() const
{
    return Object::sendMessage<CFTimeInterval>(this, _CA_PRIVATE_SEL(targetTimestamp));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE CFTimeInterval CA::MetalDisplayLinkUpdate::targetPresentationTimestamp() const
{
    return Object::sendMessage<CFTimeInterval>(this, _CA_PRIVATE_SEL(targetPresentationTimestamp));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE CA::MetalDisplayLink* CA::MetalDisplayLink::alloc()
{
    return NS::Object::alloc<MetalDisplayLink>(_CA_PRIVATE_CLS(CAMetalDisplayLink));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE CA::MetalDisplayLink* CA::MetalDisplayLink::init(const CA::MetalLayer* layer)
{
    return Object::sendMessage<MetalDisplayLink*>(this, _CA_PRIVATE_SEL(initWithMetalLayer_), layer);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE NS::Object* CA::MetalDisplayLink::delegate() const
{
    return Object::sendMessage<NS::Object*>(this, _CA_PRIVATE_SEL(delegate));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE void CA::MetalDisplayLink::setDelegate(const NS::Object* delegate)
{
    Object::sendMessage<void>(this, _CA_PRIVATE_SEL(setDelegate_), delegate);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE CA::FrameRateRange CA::MetalDisplayLink::preferredFrameRateRange() const
{
    return Object::sendMessage<FrameRateRange>(this, _CA_PRIVATE_SEL(preferredFrameRateRange));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE void CA::MetalDisplayLink::setPreferredFrameRateRange(CA::FrameRateRange range)
{
    Object::sendMessage<void>(this, _CA_PRIVATE_SEL(setPreferredFrameRateRange_), range);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE float CA::MetalDisplayLink::preferredFrameLatency() const
{
    return Object::sendMessage<float>(this, _CA_PRIVATE_SEL(preferredFrameLatency));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE void CA::MetalDisplayLink::setPreferredFrameLatency(float latency)
{
    Object::sendMessage<void>(this, _CA_PRIVATE_SEL(setPreferredFrameLatency_), latency);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE bool CA::MetalDisplayLink::isPaused() const
{
    return Object::sendMessage<bool>(this, _CA_PRIVATE_SEL(isPaused));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE void CA::MetalDisplayLink::setPaused(bool paused)
{
    Object::sendMessage<void>(this, _CA_PRIVATE_SEL(setPaused_), paused);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE void CA::MetalDisplayLink::addToRunLoop(const NS::Object* runLoop, const NS::String* mode)
{
    Object::sendMessage<void>(this, _CA_PRIVATE_SEL(addToRunLoop_forMode_), runLoop, mode);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE void CA::MetalDisplayLink::removeFromRunLoop(const NS::Object* runLoop, const NS::String* mode)
{
    Object::sendMessage<void>(this, _CA_PRIVATE_SEL(removeFromRunLoop_forMode_), runLoop, mode);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_CA_INLINE void CA::MetalDisplayLink::invalidate()
{
    Object::sendMessage<void>(this, _CA_PRIVATE_SEL(invalidate));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
