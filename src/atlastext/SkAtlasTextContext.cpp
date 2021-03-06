/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAtlasTextContext.h"
#include "SkAtlasTextRenderer.h"
#include "SkInternalAtlasTextContext.h"

sk_sp<SkAtlasTextContext> SkAtlasTextContext::Make(sk_sp<SkAtlasTextRenderer> renderer) {
    return sk_sp<SkAtlasTextContext>(new SkAtlasTextContext(std::move(renderer)));
}

SkAtlasTextContext::SkAtlasTextContext(sk_sp<SkAtlasTextRenderer> renderer)
        : fInternalContext(SkInternalAtlasTextContext::Make(std::move(renderer))) {}
