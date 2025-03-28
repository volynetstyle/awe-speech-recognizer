export interface SpeechRecognizerOptions {
 hmm?: string;
 lm?: string;
 dict?: string;
}

export interface NativeBindings {
 create_recognizer: (hmm: string, lm: string, dict: string) => Buffer;
 destroy_recognizer: (recognizer: Buffer) => void;
 recognize_speech: (recognizer: Buffer) => string;
 free_result: (result: string) => void;
}