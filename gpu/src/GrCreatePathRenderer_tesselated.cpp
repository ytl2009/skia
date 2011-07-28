
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrTesselatedPathRenderer.h"


GrPathRenderer* GrPathRenderer::CreatePathRenderer() { return new GrTesselatedPathRenderer(); }
