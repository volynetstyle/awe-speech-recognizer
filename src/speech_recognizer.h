#ifndef SPEECH_RECOGNIZER_H
#define SPEECH_RECOGNIZER_H

#include <memory>
#include <string>
#include <string_view>
#include <pocketsphinx.h>

using DecoderPtr = std::unique_ptr<ps_decoder_t, struct DecoderDeleter>;
using EndpointerPtr = std::unique_ptr<ps_endpointer_t, struct EndpointerDeleter>;
using ConfigPtr = std::unique_ptr<ps_config_t, struct ConfigDeleter>;

struct DecoderDeleter {
    void operator()(ps_decoder_t* ps) const noexcept { ps_free(ps); }
};

struct EndpointerDeleter {
    void operator()(ps_endpointer_t* ep) const noexcept { ps_endpointer_free(ep); }
};

struct ConfigDeleter {
    void operator()(ps_config_t* config) const noexcept { ps_config_free(config); }
};

class SpeechRecognizer {
public:
    SpeechRecognizer(std::string_view hmm, std::string_view lm, std::string_view dict);
    ~SpeechRecognizer() = default;
    SpeechRecognizer(const SpeechRecognizer&) = delete;
    SpeechRecognizer& operator=(const SpeechRecognizer&) = delete;
    std::string recognize();

private:
    DecoderPtr decoder_;
    EndpointerPtr ep_;
    size_t frame_size_;
    bool is_initialized_ = false;
};

extern "C" {
    SpeechRecognizer* create_recognizer(const char* hmm, const char* lm, const char* dict) noexcept;
    void destroy_recognizer(SpeechRecognizer* recognizer) noexcept;
    const char* recognize_speech(SpeechRecognizer* recognizer);
    void free_result(const char* result) noexcept;
}

#endif