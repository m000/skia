/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLocalMatrixShader.h"
#include "SkTLazy.h"

#if SK_SUPPORT_GPU
#include "GrFragmentProcessor.h"
#endif

#if SK_SUPPORT_GPU
std::unique_ptr<GrFragmentProcessor> SkLocalMatrixShader::asFragmentProcessor(
        const GrFPArgs& args) const {
    return as_SB(fProxyShader)->asFragmentProcessor(
        GrFPArgs::WithPreLocalMatrix(args, this->getLocalMatrix()));
}
#endif

sk_sp<SkFlattenable> SkLocalMatrixShader::CreateProc(SkReadBuffer& buffer) {
    SkMatrix lm;
    buffer.readMatrix(&lm);
    auto baseShader(buffer.readShader());
    if (!baseShader) {
        return nullptr;
    }
    return baseShader->makeWithLocalMatrix(lm);
}

void SkLocalMatrixShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeMatrix(this->getLocalMatrix());
    buffer.writeFlattenable(fProxyShader.get());
}

SkShaderBase::Context* SkLocalMatrixShader::onMakeContext(
    const ContextRec& rec, SkArenaAlloc* alloc) const
{
    SkTCopyOnFirstWrite<SkMatrix> lm(this->getLocalMatrix());
    if (rec.fLocalMatrix) {
        lm.writable()->preConcat(*rec.fLocalMatrix);
    }

    ContextRec newRec(rec);
    newRec.fLocalMatrix = lm;

    return as_SB(fProxyShader)->makeContext(newRec, alloc);
}

SkImage* SkLocalMatrixShader::onIsAImage(SkMatrix* outMatrix, enum TileMode* mode) const {
    SkMatrix imageMatrix;
    SkImage* image = fProxyShader->isAImage(&imageMatrix, mode);
    if (image && outMatrix) {
        // Local matrix must be applied first so it is on the right side of the concat.
        *outMatrix = SkMatrix::Concat(imageMatrix, this->getLocalMatrix());
    }

    return image;
}

bool SkLocalMatrixShader::onAppendStages(const StageRec& rec) const {
    SkTCopyOnFirstWrite<SkMatrix> lm(this->getLocalMatrix());
    if (rec.fLocalM) {
        lm.writable()->preConcat(*rec.fLocalM);
    }

    StageRec newRec = rec;
    newRec.fLocalM = lm;
    return as_SB(fProxyShader)->appendStages(newRec);
}

void SkLocalMatrixShader::toString(SkString* str) const {
    str->append("SkLocalMatrixShader: (");

    as_SB(fProxyShader)->toString(str);

    this->INHERITED::toString(str);

    str->append(")");
}

sk_sp<SkShader> SkShader::makeWithLocalMatrix(const SkMatrix& localMatrix) const {
    if (localMatrix.isIdentity()) {
        return sk_ref_sp(const_cast<SkShader*>(this));
    }

    const SkMatrix* lm = &localMatrix;

    sk_sp<SkShader> baseShader;
    SkMatrix otherLocalMatrix;
    sk_sp<SkShader> proxy(as_SB(this)->makeAsALocalMatrixShader(&otherLocalMatrix));
    if (proxy) {
        otherLocalMatrix.preConcat(localMatrix);
        lm = &otherLocalMatrix;
        baseShader = proxy;
    } else {
        baseShader = sk_ref_sp(const_cast<SkShader*>(this));
    }

    return sk_make_sp<SkLocalMatrixShader>(std::move(baseShader), *lm);
}
