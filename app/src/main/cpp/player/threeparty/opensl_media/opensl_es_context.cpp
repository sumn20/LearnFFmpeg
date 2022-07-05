//
// Created by Sumn on 2021/12/8.
//

#include "opensl_es_context.h"
OpenSLESContext* OpenSLESContext::instance=new OpenSLESContext();

void OpenSLESContext::init() {
    LOGCATI("createEngine");
    SLresult result=createEngine();
    LOGCATI("createEngine result is %s", ResultToString(result));
    if (SL_RESULT_SUCCESS==result){
        LOGCATI("Realize the engine object");
        // Realize the engine object
        result = RealizeObject(engineObject);
        if (SL_RESULT_SUCCESS == result) {
            LOGCATI("Get the engine interface");
            // Get the engine interface
            result = GetEngineInterface();
        }
    }

}

OpenSLESContext::OpenSLESContext() {
    isInited= false;

}

OpenSLESContext *OpenSLESContext::getInstance() {
    if (!instance->isInited) {
        instance->init();
        instance->isInited = true;
    }
    return instance;
}

OpenSLESContext::~OpenSLESContext() {

}
