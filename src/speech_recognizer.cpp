#include "speech_recognizer.h"
#include <pocketsphinx.h>
#include <stdexcept>
#include <cstring>
#include <vector>

SpeechRecognizer::SpeechRecognizer(std::string_view hmm, std::string_view lm, std::string_view dict) {
    cmd_ln_t* config = cmd_ln_init(nullptr, ps_args(), TRUE,
        "-hmm", hmm.data(),
        "-lm", lm.data(),
        "-dict", dict.data(),
        nullptr);
    
    if (!config) {
        throw std::runtime_error("Failed to initialize config");
    }

    struct ConfigDeleter {
        void operator()(cmd_ln_t* config) const noexcept { cmd_ln_free_r(config); }
    };
    std::unique_ptr<cmd_ln_t, ConfigDeleter> config_guard(config);

    decoder_ = std::unique_ptr<ps_decoder_t, DecoderDeleter>(ps_init(config));
    if (!decoder_) {
        throw std::runtime_error("Failed to initialize decoder");
    }
    config_guard.release(); /

    audio_device_ = std::unique_ptr<ad_rec_t, AudioDeleter>(ad_open_dev(nullptr, 16000));
    if (!audio_device_) {
        throw std::runtime_error("Failed to open audio device");
    }

    is_initialized_ = true;
}

std::string SpeechRecognizer::recognize() {
    if (!is_initialized_) {
        return "Recognizer not initialized";
    }

    if (ad_start_rec(audio_device_.get()) < 0) {
        return "Failed to start recording";
    }
    if (ps_start_utt(decoder_.get()) < 0) {
        return "Failed to start utterance";
    }

    constexpr size_t BUFFER_SIZE = 2048;
    std::vector<int16> buffer(BUFFER_SIZE);
    std::string result;
    bool is_speech = false;

    while (true) {
        int32 k = ad_read(audio_device_.get(), buffer.data(), BUFFER_SIZE);
        if (k < 0) {
            break;
        }

        ps_process_raw(decoder_.get(), buffer.data(), k, FALSE, FALSE);
        
        int32 score;
        if (const char* hyp = ps_get_hyp(decoder_.get(), &score)) {
            result = hyp;
            is_speech = true;
        }

        if (is_speech && k < static_cast<int32>(BUFFER_SIZE)) {
            break;
        }
    }

    ad_stop_rec(audio_device_.get());
    ps_end_utt(decoder_.get());

    return result.empty() ? "No speech detected" : result;
}

extern "C" {
    SpeechRecognizer* create_recognizer(const char* hmm, const char* lm, const char* dict) noexcept {
        try {
            return new SpeechRecognizer(hmm ? hmm : "", lm ? lm : "", dict ? dict : "");
        } catch (const std::exception&) {
            return nullptr;
        }
    }

    void destroy_recognizer(SpeechRecognizer* recognizer) noexcept {
        delete recognizer;
    }

    const char* recognize_speech(SpeechRecognizer* recognizer) {
        if (!recognizer) {
            return nullptr;
        }
        std::string result = recognizer->recognize();
        char* c_result = new char[result.length() + 1];
        std::strcpy(c_result, result.c_str());
        return c_result;
    }

    void free_result(const char* result) noexcept {
        delete[] result;
    }
}