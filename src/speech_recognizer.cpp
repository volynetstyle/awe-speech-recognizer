#include "speech_recognizer.h"
#include <pocketsphinx.h>
#include <sphinxbase/ad.h>
#include <stdexcept>

SpeechRecognizer::SpeechRecognizer(const char* hmm, const char* lm, const char* dict) {
    cmd_ln_t* config = cmd_ln_init(NULL, ps_args(), TRUE,
        "-hmm", hmm,
        "-lm", lm,
        "-dict", dict,
        NULL);
    
    if (!config) throw std::runtime_error("Failed to initialize config");

    decoder = ps_init(config);
    if (!decoder) {
        cmd_ln_free_r(config);
        throw std::runtime_error("Failed to initialize decoder");
    }

    audio_device = ad_open_dev(NULL, 16000);
    if (!audio_device) {
        ps_free((ps_decoder_t*)decoder);
        cmd_ln_free_r(config);
        throw std::runtime_error("Failed to open audio device");
    }

    is_initialized = true;
}

SpeechRecognizer::~SpeechRecognizer() {
    if (is_initialized) {
        ad_close((ad_rec_t*)audio_device);
        ps_free((ps_decoder_t*)decoder);
    }
}

std::string SpeechRecognizer::recognize() {
    if (!is_initialized) return "Recognizer not initialized";

    ps_decoder_t* ps = (ps_decoder_t*)decoder;
    ad_rec_t* ad = (ad_rec_t*)audio_device;

    if (ad_start_rec(ad) < 0) return "Failed to start recording";
    if (ps_start_utt(ps) < 0) return "Failed to start utterance";

    int16 buffer[2048];
    std::string result;
    bool is_speech = false;

    while (true) {
        int32 k = ad_read(ad, buffer, 2048);
        if (k < 0) break;

        ps_process_raw(ps, buffer, k, FALSE, FALSE);
        
        int32 score;
        const char* hyp = ps_get_hyp(ps, &score);
        if (hyp) {
            result = std::string(hyp);
            is_speech = true;
        }

        if (is_speech && k < 2048) break;
    }

    ad_stop_rec(ad);
    ps_end_utt(ps);

    return result.empty() ? "No speech detected" : result;
}

extern "C" {
    SpeechRecognizer* create_recognizer(const char* hmm, const char* lm, const char* dict) {
        try {
            return new SpeechRecognizer(hmm, lm, dict);
        } catch (const std::exception& e) {
            return nullptr;
        }
    }

    void destroy_recognizer(SpeechRecognizer* recognizer) {
        delete recognizer;
    }

    const char* recognize_speech(SpeechRecognizer* recognizer) {
        std::string result = recognizer->recognize();
        char* c_result = new char[result.length() + 1];
        strcpy(c_result, result.c_str());
        return c_result;
    }

    void free_result(const char* result) {
        delete[] result;
    }
}