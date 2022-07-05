#include "gpu_texture_frame_uploader.h"

#define LOG_TAG "GPUTextureFrameUploader"

GPUTextureFrameUploader::GPUTextureFrameUploader(){
	decodeTexId = 0;
	LOGCATI("GPUTextureFrameUploader instance created");
}

GPUTextureFrameUploader::~GPUTextureFrameUploader() {
	LOGCATI("GPUTextureFrameUploader instance destroyed");
}

bool GPUTextureFrameUploader::initialize() {
	TextureFrameUploader::initialize();
	//init decodeTexId
	textureFrame = new GPUTextureFrame();
	textureFrame->createTexture();

	// todo: need write lock
	decodeTexId = ((GPUTextureFrame *)textureFrame)->getDecodeTexId();
	LOGCATI("createTexture success!!!!!!!!!! %d", decodeTexId);

	textureFrameCopier = new GPUTextureFrameCopier();
	textureFrameCopier->init();
	return true;
}

void GPUTextureFrameUploader::destroy() {
	if (textureFrameCopier) {
		textureFrameCopier->destroy();
		delete textureFrameCopier;
		textureFrameCopier = NULL;
	}
	if (textureFrame) {
		textureFrame->dealloc();
	}
	TextureFrameUploader::destroy();
}


