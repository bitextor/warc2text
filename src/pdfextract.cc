#include "pdfextract.hh"
#include <boost/log/trivial.hpp>

namespace util {
    std::string PDFextract::config_file;
    std::string PDFextract::log_file;
    long PDFextract::timeout;
    bool PDFextract::verbose;

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

    void PDFextract::setConfig(const std::string& config, const std::string& log, long timeout, bool verbose) {
        PDFextract::config_file = config;
        PDFextract::log_file = log;
        PDFextract::timeout = timeout;
        PDFextract::verbose = verbose;
    }

    void PDFextract::init() {
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

        // complete constructor:
        jmethodID pdfextract_constructor = env->GetMethodID(pdfextract_class, "<init>", "(Ljava/lang/String;ILjava/lang/String;JLjava/lang/String;Ljava/lang/String;)V");

        jint verbose_param = 1;
        jlong timeout_param = 0;
        jstring empty_jstring = env->NewStringUTF("");
        jstring config_file_jstring = env->NewStringUTF(PDFextract::config_file.c_str());
        jstring log_file_jstring = env->NewStringUTF(PDFextract::log_file.c_str());

        jobject temp_extractor = env->NewObject(pdfextract_class, pdfextract_constructor, log_file_jstring, verbose_param, config_file_jstring, timeout_param, empty_jstring, empty_jstring);
        if (exceptionOccurred()) {
            BOOST_LOG_TRIVIAL(error) << "PDFExtract: error constructing PDFExtract object";
            this->extractor = NULL;
            env->DeleteLocalRef(temp_extractor);
            env->ReleaseStringUTFChars(log_file_jstring, NULL);
            env->ReleaseStringUTFChars(config_file_jstring, NULL);
            return;
        }
        env->ReleaseStringUTFChars(log_file_jstring, NULL);
        env->ReleaseStringUTFChars(config_file_jstring, NULL);

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

        if (exceptionOccurred())
            // shouldn't happen
            BOOST_LOG_TRIVIAL(error) << "PDFExtract: something went wrong";
    }

    std::string PDFextract::extract(const std::string& original) {
        std::string html;
        if (env == nullptr or extractor == nullptr) return html;
        //
        env->PushLocalFrame(16);

        // create byte[] with the PDF contents
        const jbyte* data = (jbyte*) original.data();
        jbyteArray bytes_array = env->NewByteArray(original.size());
        env->SetByteArrayRegion(bytes_array, 0, original.size(), data);

        // create ByteArrayInputStreamObject from the byte[] data
        jobject inputstream = env->NewObject(byte_array_input_stream, bais_constructor, bytes_array);
        if (exceptionOccurred()) {
            env->ReleaseByteArrayElements(bytes_array, env->GetByteArrayElements(bytes_array, NULL), 0);
            env->PopLocalFrame(NULL);
            return html;
        }
        env->ReleaseByteArrayElements(bytes_array, env->GetByteArrayElements(bytes_array, NULL), 0);

        // call Extract method with inputStream
        // result is ByteArrayOutputStream object
        jint keepbrtags = 0;
        jint getperm = 0; // getperm = 1 doesn't work (bug on Java side)
        jobject baos_result = env->CallObjectMethod(extractor, extract_method, inputstream, keepbrtags, getperm);
        if (exceptionOccurred()) {
            env->PopLocalFrame(NULL);
            return html;
        }

        // convert ByteArrayOutputStream to java string
        jstring jstring_result = (jstring) env->CallObjectMethod(baos_result, get_string_from_baos);
        if (exceptionOccurred()) {
            env->ReleaseStringUTFChars(jstring_result, NULL);
            env->PopLocalFrame(NULL);
            return html;
        }

        // convert java string to c++ string
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
