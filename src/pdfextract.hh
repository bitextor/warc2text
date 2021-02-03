#ifndef PDFEXTRACT_HH
#define PDFEXTRACT_HH

#include "jni.h"
#include <string>

namespace util {

    class PDFextract {
        private:
            JavaVM* jvm;
            JNIEnv* env;

            jmethodID extract_method;
            jobject extractor;

            bool exceptionOccurred();
        public:
            PDFextract();
            std::string extract(const std::string& original);

            ~PDFextract();
    };
}

#endif
