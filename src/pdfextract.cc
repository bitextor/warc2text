#include "pdfextract.hh"
#include <boost/log/trivial.hpp>

namespace util {
    PDFextract::PDFextract() {
        JavaVMOption options[2];
        options[0].optionString = "-Djava.class.path=/home/elsa/pdf-extract/PDFExtract-2.0.jar";
        options[1].optionString = "-Xcheck:jni";
        JavaVMInitArgs args;
        args.version = JNI_VERSION_1_6;
        args.nOptions = 2;
        args.options = options;
        args.ignoreUnrecognized = 1;

        jint res = JNI_CreateJavaVM(&jvm, (void **)&env, &args);
        if (exceptionOccurred()) return;

        jclass pdfextract_class = env->FindClass("pdfextract/PDFExtract");

        // complete constructor:
        jmethodID pdfextract_constructor = env->GetMethodID(pdfextract_class, "<init>", "(Ljava/lang/String;ILjava/lang/String;JLjava/lang/String;Ljava/lang/String;)V");

        jint verbose = 1;
        jlong timeout = 0;
        jstring empty_jstring = env->NewStringUTF("");

        extractor = env->NewGlobalRef(env->NewObject(pdfextract_class, pdfextract_constructor, empty_jstring, verbose, empty_jstring, timeout, empty_jstring, empty_jstring));

        env->ReleaseStringUTFChars(empty_jstring, NULL);

        extract_method = env->GetMethodID(pdfextract_class, "Extract", "(Ljava/io/ByteArrayInputStream;II)Ljava/io/ByteArrayOutputStream;");
        if (exceptionOccurred()) return;
    }

    std::string PDFextract::extract(const std::string& original) {
        //
        jclass string_class = env->FindClass("java/lang/String");
        jmethodID getBytes = env->GetMethodID(string_class, "getBytes", "()[B");

        jstring jstring_original = env->NewStringUTF(original.c_str());
        jobject bytes_array = env->CallObjectMethod(jstring_original, getBytes);
        env->ReleaseStringUTFChars(jstring_original, NULL);

        if (exceptionOccurred()) return "";

        //
        jclass inputstream_class = env->FindClass("java/io/ByteArrayInputStream");
        jmethodID inputstream_constructor = env->GetMethodID(inputstream_class, "<init>", "([B)V");

        jobject inputstream = env->NewObject(inputstream_class, inputstream_constructor, bytes_array);

        if (exceptionOccurred()) return "";

        //
        jint keepbrtags = 0;
        jint getperm = 0; // getperm = 1 doesn't work (bug on Java side)
        jobject baos_result = env->CallObjectMethod(extractor, extract_method, inputstream, keepbrtags, getperm);
        if (exceptionOccurred()) return "";

        //
        jclass outputstream_class = env->FindClass("java/io/ByteArrayOutputStream");
        jmethodID toString = env->GetMethodID(outputstream_class, "toString", "()Ljava/lang/String;");
        jstring jstring_result = (jstring) env->CallObjectMethod(baos_result, toString);
        if (exceptionOccurred()) return "";

        std::string html = env->GetStringUTFChars(jstring_result, NULL);
        env->ReleaseStringUTFChars(jstring_result, NULL);
        if (exceptionOccurred()) return "";

        return html;
    }

    PDFextract::~PDFextract(){
        env->DeleteGlobalRef(extractor);
        jvm->DestroyJavaVM();
    }

    bool PDFextract::exceptionOccurred(){
        if (env->ExceptionOccurred()) {
            env->ExceptionDescribe();
            return true;
        }
        return false;
    }

} // namespace util
