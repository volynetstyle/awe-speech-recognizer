#ifndef SPEECH_RECOGNIZER_H
#define SPEECH_RECOGNIZER_H

#include <memory>
#include <string>
#include <string_view>

class SpeechRecognizer {
public:
    SpeechRecognizer(std::string_view hmm, std::string_view lm, std::string_view dict);
    ~SpeechRecognizer() = default;
    SpeechRecognizer(const SpeechRecognizer&) = delete;
    SpeechRecognizer& operator=(const SpeechRecognizer&) = delete;
    std::string recognize();

private:
    struct DecoderDeleter {
        void operator()(ps_decoder_t* ps) const noexcept { ps_free(ps); }
    };
    struct AudioDeleter {
        void operator()(ad_rec_t* ad) const noexcept { ad_close(ad); }
    };

    std::unique_ptr<ps_decoder_t, DecoderDeleter> decoder_;
    std::unique_ptr<ad_rec_t, AudioDeleter> audio_device_;
    bool is_initialized_ = false;
};

extern "C" {
    SpeechRecognizer* create_recognizer(const char* hmm, const char* lm, const char* dict) noexcept;
    void destroy_recognizer(SpeechRecognizer* recognizer) noexcept;
    const char* recognize_speech(SpeechRecognizer* recognizer);
    void free_result(const char* result) noexcept;
}

#endif