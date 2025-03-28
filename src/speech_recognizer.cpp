
#include <stdexcept>
#include <cstring>
#include <vector>

#include "speech_recognizer.h"
#include <portaudio.h>

namespace {
    constexpr int SAMPLE_RATE = 16000;
    using AudioBuffer = std::vector<int16>;
    using ErrorMsg = std::string;

    void check_pa_error(PaError err, const char* msg) {
        if (err != paNoError) {
            throw std::runtime_error(ErrorMsg(msg) + ": " + Pa_GetErrorText(err));
        }
    }
}

typedef ps_vad_mode_e EVoiceActivityDetection;

SpeechRecognizer::SpeechRecognizer(std::string_view hmm, std::string_view lm, std::string_view dict) {
    ConfigPtr::pointer config = ps_config_init(nullptr);
    if (!config) throw std::runtime_error("Failed to initialize config");
    ConfigPtr config_guard(config);

    ps_config_set_str(config, "hmm", hmm.data());
    ps_config_set_str(config, "lm", lm.data());
    ps_config_set_str(config, "dict", dict.data());
    ps_config_set_int(config, "samprate", SAMPLE_RATE);

    decoder_ = DecoderPtr(ps_init(config));
    if (!decoder_) throw std::runtime_error("Failed to initialize decoder");
    config_guard.release();

    ep_ = EndpointerPtr(ps_endpointer_init(0, 0.0, PS_VAD_LOOSE, 0, 0));
    if (!ep_) throw std::runtime_error("Failed to initialize endpointer");

    frame_size_ = ps_endpointer_frame_size(ep_.get());
    is_initialized_ = true;
}

std::string SpeechRecognizer::recognize() {
    if (!is_initialized_) return "Recognizer not initialized";

    PaStream* stream;
    PaError err = Pa_OpenDefaultStream(&stream, 1, 0, paInt16, SAMPLE_RATE, frame_size_, nullptr, nullptr);
    if (err != paNoError) return ErrorMsg("Failed to open stream: ") + Pa_GetErrorText(err);

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        Pa_CloseStream(stream);
        return ErrorMsg("Failed to start stream: ") + Pa_GetErrorText(err);
    }

    AudioBuffer buffer(frame_size_);
    std::string result;
    bool in_speech = false;

    while (true) {
        err = Pa_ReadStream(stream, buffer.data(), frame_size_);
        if (err != paNoError) break;

        const int16* speech = ps_endpointer_process(ep_.get(), buffer.data());
        if (speech != nullptr) {
            if (!in_speech) {
                in_speech = true;
                if (ps_start_utt(decoder_.get()) < 0) {
                    Pa_CloseStream(stream);
                    return "Failed to start utterance";
                }
            }
            ps_process_raw(decoder_.get(), speech, frame_size_, FALSE, FALSE);
            if (!ps_endpointer_in_speech(ep_.get())) {
                ps_end_utt(decoder_.get());
                int32 score;
                const char* hyp = ps_get_hyp(decoder_.get(), &score);
                if (hyp != nullptr) result = hyp;
                break;
            }
        }
    }

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
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
        delete recognizer; // Assume Pa_Terminate() called globally
    }

    const char* recognize_speech(SpeechRecognizer* recognizer) {
        if (!recognizer) return nullptr;
        std::string result = recognizer->recognize();
        char* c_result = new char[result.length() + 1];
        std::strcpy(c_result, result.c_str());
        return c_result;
    }

    void free_result(const char* result) noexcept {
        delete[] result;
    }
}