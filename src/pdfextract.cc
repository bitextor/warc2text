#include "pdfextract.hh"
#include <boost/log/trivial.hpp>

namespace util {
    void PDFextract::startJavaVM(const std::string& pdfextract_jar) {
        std::string classpath = "-Djava.class.path=" + pdfextract_jar;
        std::string debug = "-Xcheck:jni";

        JavaVMOption options[2];
        options[0].optionString = &classpath[0];
        options[1].optionString = &debug[0];
        JavaVMInitArgs args;
        args.version = JNI_VERSION_1_2;
        args.nOptions = 2;
        args.options = options;
        args.ignoreUnrecognized = 1;

        JavaVM* jvm;
        JNIEnv* env;
        jint res = JNI_CreateJavaVM(&jvm, (void **)&env, &args);
        if (res != JNI_OK)
            BOOST_LOG_TRIVIAL(error) << "PDFextract: error creating Java Virtual Machine";
    }

    bool PDFextract::getJavaVM(JavaVM **vm) {
        jsize created;
        jint res = JNI_GetCreatedJavaVMs(vm, 1, &created);
        if (res != JNI_OK) {
            BOOST_LOG_TRIVIAL(error) << "PDFextract: error getting Java Virtual Machine";
            vm = nullptr;
            return false;
        }
        return created;
    }

    void PDFextract::destroyJavaVM() {
        JavaVM *vm;
        bool created = getJavaVM(&vm);
        if (not created or vm == nullptr) return;
        vm->DestroyJavaVM();
    }

    PDFextract::PDFextract(const std::string& config_file, const std::string& log_file, bool verbose) {
        JavaVM* jvm;
        bool created = getJavaVM(&jvm);
        if (not created or jvm == nullptr) {
            env = nullptr;
            return;
        }

        jint res;
        res = jvm->GetEnv((void **) &env, JNI_VERSION_1_2);
        if (res != JNI_OK) {
            BOOST_LOG_TRIVIAL(error) << "PDFextract: error getting JNI environment";
            return;
        }

        jclass pdfextract_class = env->FindClass("pdfextract/PDFExtract");
        exceptionOccurred();

        // complete constructor:
        jmethodID pdfextract_constructor = env->GetMethodID(pdfextract_class, "<init>", "(Ljava/lang/String;ILjava/lang/String;JLjava/lang/String;Ljava/lang/String;)V");


        jint verbose_param = 0;
        jlong timeout_param = 0;
        jstring empty_jstring = env->NewStringUTF("");

        jobject temp_extractor = env->NewObject(pdfextract_class, pdfextract_constructor, empty_jstring, verbose_param, empty_jstring, timeout_param, empty_jstring, empty_jstring);
        extractor = env->NewGlobalRef(temp_extractor);
        env->DeleteLocalRef(temp_extractor);

        env->ReleaseStringUTFChars(empty_jstring, NULL);

        extract_method = env->GetMethodID(pdfextract_class, "Extract", "(Ljava/io/ByteArrayInputStream;II)Ljava/io/ByteArrayOutputStream;");
        env->DeleteLocalRef(pdfextract_class);

        jclass string_class = env->FindClass("java/lang/String");
        get_bytes_from_string = env->GetMethodID(string_class, "getBytes", "()[B");
        env->DeleteLocalRef(string_class);

        byte_array_input_stream = env->FindClass("java/io/ByteArrayInputStream");
        byte_array_output_stream = env->FindClass("java/io/ByteArrayOutputStream");
        bais_constructor = env->GetMethodID(byte_array_input_stream, "<init>", "([B)V");
        get_string_from_baos = env->GetMethodID(byte_array_output_stream, "toString", "()Ljava/lang/String;");
        exceptionOccurred();
    }

    std::string PDFextract::extract(const std::string& original) {
        std::string html = "";
        if (env == nullptr) return html;
        //
        env->PushLocalFrame(16);
        jstring jstring_original = env->NewStringUTF(original.c_str());
        jobject bytes_array = env->CallObjectMethod(jstring_original, get_bytes_from_string);
        if (exceptionOccurred()) {
            env->ReleaseStringUTFChars(jstring_original, NULL);
            env->PopLocalFrame(NULL);
            return html;
        }
        env->ReleaseStringUTFChars(jstring_original, NULL);


        //
        jobject inputstream = env->NewObject(byte_array_input_stream, bais_constructor, bytes_array);
        if (exceptionOccurred()) {
            env->PopLocalFrame(NULL);
            return html;
        }

        //
        jint keepbrtags = 0;
        jint getperm = 0; // getperm = 1 doesn't work (bug on Java side)
        jobject baos_result = env->CallObjectMethod(extractor, extract_method, inputstream, keepbrtags, getperm);
        if (exceptionOccurred()) {
            env->PopLocalFrame(NULL);
            return html;
        }

        //
        jstring jstring_result = (jstring) env->CallObjectMethod(baos_result, get_string_from_baos);
        if (exceptionOccurred()) {
            env->ReleaseStringUTFChars(jstring_result, NULL);
            env->PopLocalFrame(NULL);
            return html;
        }

        html = env->GetStringUTFChars(jstring_result, NULL);
        env->ReleaseStringUTFChars(jstring_result, NULL);
        env->PopLocalFrame(NULL);

        return html;
    }

    PDFextract::~PDFextract(){
        // env->DeleteGlobalRef(extractor);
        // jvm->DestroyJavaVM();
    }

    bool PDFextract::exceptionOccurred(){
        if (env->ExceptionOccurred()) {
            env->ExceptionDescribe();
            return true;
        }
        return false;
    }

} // namespace util
