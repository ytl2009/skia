
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrGpuGL_DEFINED
#define GrGpuGL_DEFINED

#include "GrGpu.h"
#include "GrGLIRect.h"
#include "GrGLTexture.h"

#include "GrGLVertexBuffer.h"
#include "GrGLIndexBuffer.h"

#include "SkString.h"

class GrGpuGL : public GrGpu {
public:
    virtual ~GrGpuGL();

protected:
    GrGpuGL();

    struct {
        size_t                  fVertexOffset;
        GrVertexLayout          fVertexLayout;
        const GrVertexBuffer*   fVertexBuffer;
        const GrIndexBuffer*    fIndexBuffer;
        bool                    fArrayPtrsDirty;
    } fHWGeometryState;

    struct AAState {
        bool fMSAAEnabled;
        bool fSmoothLineEnabled;
    } fHWAAState;

    DrState   fHWDrawState;
    bool      fHWStencilClip;

    // read these once at begining and then never again
    SkString fExtensionString;
    float fGLVersion;

    // As flush of GL state proceeds it updates fHDrawState
    // to reflect the new state. Later parts of the state flush
    // may perform cascaded changes but cannot refer to fHWDrawState.
    // These code paths can refer to the dirty flags. Subclass should
    // call resetDirtyFlags after its flush is complete
    struct {
        bool fRenderTargetChanged : 1;
        int  fTextureChangedMask;
    } fDirtyFlags;
    GR_STATIC_ASSERT(8 * sizeof(int) >= kNumStages);

    // clears the dirty flags
    void resetDirtyFlags();

    // last scissor / viewport scissor state seen by the GL.
    struct {
        bool        fScissorEnabled;
        GrGLIRect   fScissorRect;
        GrGLIRect   fViewportRect;
    } fHWBounds;

    // GrGpu overrides
    // overrides from GrGpu
    virtual void resetContext();

    virtual GrTexture* onCreateTexture(const GrTextureDesc& desc,
                                       const void* srcData,
                                       size_t rowBytes);
    virtual GrVertexBuffer* onCreateVertexBuffer(uint32_t size,
                                                 bool dynamic);
    virtual GrIndexBuffer* onCreateIndexBuffer(uint32_t size,
                                               bool dynamic);
    virtual GrResource* onCreatePlatformSurface(const GrPlatformSurfaceDesc& desc);
    virtual GrRenderTarget* onCreateRenderTargetFrom3DApiState();

    virtual void onClear(const GrIRect* rect, GrColor color);

    virtual void onForceRenderTargetFlush();

    virtual bool onReadPixels(GrRenderTarget* target,
                              int left, int top, int width, int height,
                              GrPixelConfig, void* buffer);

    virtual void onGpuDrawIndexed(GrPrimitiveType type,
                                  uint32_t startVertex,
                                  uint32_t startIndex,
                                  uint32_t vertexCount,
                                  uint32_t indexCount);
    virtual void onGpuDrawNonIndexed(GrPrimitiveType type,
                                     uint32_t vertexCount,
                                     uint32_t numVertices);
    virtual void flushScissor(const GrIRect* rect);
    void clearStencil(uint32_t value, uint32_t mask);
    virtual void clearStencilClip(const GrIRect& rect);
    virtual int getMaxEdges() const;

    // binds texture unit in GL
    void setTextureUnit(int unitIdx);

    // binds appropriate vertex and index buffers, also returns any extra
    // extra verts or indices to offset by.
    void setBuffers(bool indexed,
                    int* extraVertexOffset,
                    int* extraIndexOffset);

    // flushes state that is common to fixed and programmable GL
    // dither
    // line smoothing
    // texture binding
    // sampler state (filtering, tiling)
    // FBO binding
    // line width
    bool flushGLStateCommon(GrPrimitiveType type);

    // subclass should call this to flush the blend state
    void flushBlend(GrPrimitiveType type,
                    GrBlendCoeff srcCoeff,
                    GrBlendCoeff dstCoeff);

    bool hasExtension(const char* ext) {
        return has_gl_extension_from_string(ext, fExtensionString.c_str());
    }

    // adjusts texture matrix to account for orientation, size, and npotness
    static void AdjustTextureMatrix(const GrGLTexture* texture,
                                    GrSamplerState::SampleMode mode,
                                    GrMatrix* matrix);

    // subclass may try to take advantage of identity tex matrices.
    // This helper determines if matrix will be identity after all
    // adjustments are applied.
    static bool TextureMatrixIsIdentity(const GrGLTexture* texture,
                                        const GrSamplerState& sampler);

    static bool BlendCoeffReferencesConstant(GrBlendCoeff coeff);

private:

    // determines valid stencil formats
    void setupStencilFormats();

    // notify callbacks to update state tracking when related
    // objects are bound to GL or deleted outside of the class
    void notifyVertexBufferBind(const GrGLVertexBuffer* buffer);
    void notifyVertexBufferDelete(const GrGLVertexBuffer* buffer);
    void notifyIndexBufferBind(const GrGLIndexBuffer* buffer);
    void notifyIndexBufferDelete(const GrGLIndexBuffer* buffer);
    void notifyTextureDelete(GrGLTexture* texture);
    void notifyRenderTargetDelete(GrRenderTarget* renderTarget);

    void setSpareTextureUnit();

    bool useSmoothLines();

    // bound is region that may be modified and therefore has to be resolved.
    // NULL means whole target. Can be an empty rect.
    void flushRenderTarget(const GrIRect* bound);
    void flushStencil();
    void flushAAState(GrPrimitiveType type);

    void resolveRenderTarget(GrGLRenderTarget* texture);

    bool canBeTexture(GrPixelConfig config,
                      GrGLenum* internalFormat,
                      GrGLenum* format,
                      GrGLenum* type);

    bool fboInternalFormat(GrPixelConfig config, GrGLenum* format);

    friend class GrGLVertexBuffer;
    friend class GrGLIndexBuffer;
    friend class GrGLTexture;
    friend class GrGLRenderTarget;

    static const GrGLuint gUNKNOWN_BITCOUNT = ~0;

    struct StencilFormat {
        GrGLenum  fEnum;
        GrGLuint  fBits;
        bool      fPacked;
    };

    GrTArray<StencilFormat, true> fStencilFormats;

    bool fHWBlendDisabled;

    GrGLuint fAASamples[4];
    enum {
        kNone_MSFBO = 0,  //<! no support for MSAA FBOs
        kDesktopARB_MSFBO,//<! GL3.0-style MSAA FBO (GL_ARB_framebuffer_object)
        kDesktopEXT_MSFBO,//<! earlier GL_EXT_framebuffer* extensions
        kAppleES_MSFBO,   //<! GL_APPLE_framebuffer_multisample ES extension
    } fMSFBOType;

    // Do we have stencil wrap ops.
    bool fHasStencilWrap;

    // The maximum number of fragment uniform vectors (GLES has min. 16).
    int fMaxFragmentUniformVectors;

    // ES requires an extension to support RGBA8 in RenderBufferStorage
    bool fRGBA8Renderbuffer;

    int fActiveTextureUnitIdx;

    // we record what stencil format worked last time to hopefully exit early
    // from our loop that tries stencil formats and calls check fb status.
    int fLastSuccessfulStencilFmtIdx;

    typedef GrGpu INHERITED;
};

#endif

