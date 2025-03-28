/**
 * Represents an error specific to the Speech Recognizer.
 * This class extends the built-in `Error` class to include a custom error code.
 */
export enum RecognizerErrorCode {
 AudioInputError = "AUDIO_INPUT_ERROR",
 NetworkError = "NETWORK_ERROR",
 NotInitialized = "NOT_INITIALIZED_ERROR",
 RecognitionFailed = "RECOGNITION_FAILED",
 PermissionDenied = "PERMISSION_DENIED"
}


/**
 * Represents an error specific to the Speech Recognizer.
 * This class extends the built-in `Error` class to include a custom error code.
 */
export class SpeechRecognizerError extends Error {
 /**
  * The error code associated with the speech recognizer error.
  */
 public code: RecognizerErrorCode;

 /**
  * Creates an instance of `SpeechRecognizerError`.
  *
  * @param code - The specific error code of type `RecognizerErrorCode`.
  * @param message - An optional error message. If not provided, the error code will be used as the message.
  */
 constructor(code: RecognizerErrorCode, message?: string) {
  super(message || code);
  this.code = code;
  this.name = "AWESpeechRecognizerError";

  Object.setPrototypeOf(this, SpeechRecognizerError.prototype);
 }
}
