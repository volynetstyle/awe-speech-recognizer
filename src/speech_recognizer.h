#ifndef SPEECH_RECOGNIZER_H
#define SPEECH_RECOGNIZER_H

#include <string>

class SpeechRecognizer {
public:
    SpeechRecognizer(const char* hmm, const char* lm, const char* dict);
    ~SpeechRecognizer();
    std::string recognize();

private:
    void* decoder;  // ps_decoder_t*
    void* audio_device;  // ad_rec_t*
    bool is_initialized;
};

extern "C" {
    SpeechRecognizer* create_recognizer(const char* hmm, const char* lm, const char* dict);
    void destroy_recognizer(SpeechRecognizer* recognizer);
    const char* recognize_speech(SpeechRecognizer* recognizer);
    void free_result(const char* result);
}

#endif