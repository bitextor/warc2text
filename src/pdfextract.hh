#ifndef PDFEXTRACT_HH
#define PDFEXTRACT_HH

#include "jni.h"
#include <string>

namespace util {

    struct PDFextractOptions {
        std::string pdfextract_jar;
        std::string config_file;
        std::string log_file;
        bool verbose{};
    };

    class PDFextract {
        private:
            JNIEnv* env;

            jclass byte_array_input_stream;
            jclass byte_array_output_stream;

            jmethodID bais_constructor;

            jmethodID get_bytes_from_string;
            jmethodID get_string_from_baos;

            jmethodID extract_method;
            jobject extractor;

            bool exceptionOccurred();
            static bool getJavaVM(JavaVM **vm);

        public:
            PDFextract();
            std::string extract(const std::string& original);

            ~PDFextract();
            static void destroyJavaVM();
            static void startJavaVM(const std::string& pdfextract_jar);

    };
}

#endif
