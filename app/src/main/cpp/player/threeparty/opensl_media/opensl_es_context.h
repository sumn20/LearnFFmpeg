//
// Created by Sumn on 2021/12/8.
//

#ifndef LEARNFFMPEG_OPENSL_ES_CONTEXT_H
#define LEARNFFMPEG_OPENSL_ES_CONTEXT_H

#include "opensl_es_util.h"
#include "common_tools.h"
#include "log_util.h"

class OpenSLESContext {
private:
    //OpenSL ES 对象仅能通过其关联接口进行访问。其中包括所有对象的初始接口，称为 SLObjectItf
    //对象接口
    SLObjectItf engineObject;
    //引擎接口
    SLEngineItf engineEngine;
    bool isInited;

    /**
     * 创建 OpenSL ES 引擎
     */
    SLresult createEngine() {
        /**
         * 由于 Android 的 OpenSL ES 设计为线程安全的，所以以下设置将被忽略，但它可以使源代码可移植到其他平台。
         */
        SLEngineOption engineOptions[] = {{(SLuint32) SL_ENGINEOPTION_THREADSAFE, (SLuint32) SL_BOOLEAN_TRUE}};

        //创建并返回 OpenSL ES 引擎对象
        return slCreateEngine(&engineObject, ARRAY_LEN(engineOptions), engineOptions, 0, 0, 0);

    }

    /**
     *实现给定的对象。 对象在使用之前需要被实现。
     * @param object object instance.
     */
    SLresult RealizeObject(SLObjectItf object) {
        // Realize the engine object
        return (*object)->Realize(object, SL_BOOLEAN_FALSE);//没有异步,阻塞调用
    }

    /**
     * 从给定的引擎对象获取引擎接口，以便从引擎创建其他对象。
     * @return
     */
    SLresult GetEngineInterface() {
        //获取引擎接口
        return (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    }

    OpenSLESContext();

    void init();

    static OpenSLESContext *instance;
public:
    static OpenSLESContext *getInstance();

    virtual ~OpenSLESContext();

    SLEngineItf getEngine() {
        return engineEngine;
    }


};


#endif //LEARNFFMPEG_OPENSL_ES_CONTEXT_H
