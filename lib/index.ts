import { nativeBindings } from './bindings';
import { SpeechRecognizerOptions } from './types';

const DEFAULT_HMM = '/usr/local/share/pocketsphinx/model/en-us/en-us';
const DEFAULT_LM = '/usr/local/share/pocketsphinx/model/en-us/en-us.lm.bin';
const DEFAULT_DICT = '/usr/local/share/pocketsphinx/model/en-us/cmudict-en-us.dict';

export class SpeechRecognizer {
  private recognizer: Buffer;

  constructor(options: SpeechRecognizerOptions = {}) {
    const { hmm = DEFAULT_HMM, lm = DEFAULT_LM, dict = DEFAULT_DICT } = options;
    
    this.recognizer = nativeBindings.create_recognizer(hmm, lm, dict);
    if (!this.recognizer) {
      throw new Error('Failed to initialize speech recognizer');
    }
  }

  recognize(): string {
    const result = nativeBindings.recognize_speech(this.recognizer);
    const text = result.toString();
    nativeBindings.free_result(result);
    return text;
  }

  async recognizeAsync(): Promise<string> {
    return new Promise((resolve, reject) => {
      try {
        const result = this.recognize();
        resolve(result);
      } catch (error) {
        reject(error);
      }
    });
  }

  destroy(): void {
    nativeBindings.destroy_recognizer(this.recognizer);
  }
}

export default SpeechRecognizer;